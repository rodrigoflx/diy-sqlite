# DESIGN.md — DIY SQLite Architecture

> **Goal:** A compact, readable SQLite‑inspired DB that teaches real systems design. This design emphasizes **correctness first**, then **concurrency** via **WAL** to support multi‑threaded readers and a serialized writer.

---

## 1) System Overview

DIY SQLite is organized as four layers: **Storage**, **Access Methods**, **Execution**, and **Front‑End**. Each layer offers narrow interfaces and hides lower‑level details.

```
+--------------------------------------------------------------+
|                          Front-End                           |
|  SQL Parser/AST  ->  Binder  ->  Logical Planner             |
+------------------------------|-------------------------------+
                               v
+--------------------------------------------------------------+
|                      Execution: Bytecode VM                  |
|  Opcode interpreter (register model), cursors, value regs    |
+------------------------------|-------------------------------+
                               v
+--------------------------------------------------------------+
|                 Access Methods & Transactions                |
|  Table & Index cursors (B+Tree), Txn Manager (WAL)           |
+------------------------------|-------------------------------+
                               v
+--------------------------------------------------------------+
|                 Storage: B-Tree / Pager / I/O                |
|  B-Tree mgr  |  Pager + Cache  |  File I/O  |  WAL/Checkpoint|
+--------------------------------------------------------------+
```

---

## 2) Design Tenets

1. **Separation of concerns.** SQL never leaks into storage; storage never cares about SQL.
2. **Correctness over speed** at first: strong invariants, defensive checks, fuzz tests.
3. **Predictable on‑disk format:** fixed‑size pages, varints, serial‑types, overflow chains.
4. **Multi‑reader, single‑writer (MRSW)** concurrency with WAL: many readers, one writer at a time; readers are lock‑free w\.r.t. the main DB file.
5. **Observability built‑in:** tracing hooks (page pins, splits, WAL frames, checkpoints).

---

## 3) Data Layout (On‑Disk)

* **Page size:** default 4096 bytes. Header stores page size and metadata.
* **Page kinds:**

  * **B‑tree interior/leaf (table):** key = `rowid` (i64), payload = encoded record.
  * **B‑tree interior/leaf (index):** key = tuple `(index_key..., rowid)`; payload empty or covering columns.
  * **Overflow:** chains for large payloads.
  * **Freelist:** reserved for future space management.
* **Record encoding:** serial‑type scheme (NULL, INTEGER 1–8B, TEXT, BLOB) with varint length prefixes.

---

## 4) Storage Subsystem

### 4.1 Pager & Cache Manager

* **Role:** Uniform interface for fixed‑size pages; owns the page cache, pins/unpins, dirties, and evicts.
* **API sketch:**

```cpp
struct PageHandle { PageId id(); void* bytes(); bool dirty(); /* RAII unpin */ };
PageHandle pin(PageId id);        // increments pin count
void       mark_dirty(PageHandle&);
void       unpin(PageHandle&);    // RAII preferred
```

* **Eviction:** LRU (or clock). Never evict pinned pages. Dirty pages are flushed by the Txn manager in commit order.
* **Rationale:** Centralizing page lifetime avoids leaks/data races and simplifies WAL logic.

### 4.2 B‑Tree Manager

* **Responsibilities:**

  * Navigate interior/leaf nodes; maintain sorted cells.
  * **Split** on overflow (promote median); **merge/borrow** on underflow.
  * Leaf payload stores encoded record; index leaf may store only keys.
* **Cursor state:** `(page_id, cell_idx, parent_stack)`; supports `open(root)`, `seek(key)`, `first/last`, `next/prev`, `insert`, `erase`.
* **Concurrency:** short‑lived **latches** (mutexes) on pages during structural changes; readers use latch coupling (parent→child) only while descending.

### 4.3 Record Codec

* **Purpose:** Byte‑level encode/decode separate from B‑tree logic.
* **Benefits:** Isolation keeps access code simple and makes fuzzing/round‑trip tests easy.

---

## 5) Transactions & Concurrency (WAL)

### 5.1 Concurrency Model

* **Isolation level:** **Snapshot Isolation** for reads; **single writer** at a time.
* **Readers:** do not block writer I/O to the main DB; they read a stable snapshot = **main DB + subset of WAL frames** that existed when the read started.
* **Writer:** appends page images to the **WAL**; commits by emitting a **commit frame/marker**; later, a checkpoint copies frames back to the main DB.

### 5.2 WAL File & Metadata

* **WAL file:** append‑only sequence of **frames**; each frame = `(page_id, page_image, frame_no, checksum)`.
* **Index (in shared memory):** `wal_index` maps `page_id -> latest frame_no` for quick lookup; also stores `salt`, `generation`, and **checkpoint sequence**.
* **Commit rule:** a commit is visible when the commit marker (or frame range + end txn id) is durable; readers use the **snapshot frame boundary** captured at open/read‑txn start.

### 5.3 Read Algorithm

```
read_txn_begin():
  snapshot_seq = wal_index.global_seq

read_page(id):
  if wal_index.has(id) and wal_index.frame_no(id) <= snapshot_seq:
      return wal_frame(page=id, frame_no<=snapshot_seq)
  else:
      return main_db_page(id)
```

* **Guarantee:** Readers see a consistent version for the duration of their transaction, independent of later writer appends.

### 5.4 Write/Commit Algorithm

```
write_txn_begin():
  acquire writer mutex
  begin wal_batch

write_page(id, bytes):
  append frame(id, bytes, next_frame_no)

commit():
  fsync(WAL)
  publish wal_index.global_seq = last_frame_no  // atomic
  release writer mutex
```

* **Rollback:** discard uncommitted frames in memory (writer private state); if already appended, mark the batch aborted before publication.

### 5.5 Checkpointing

* **When:** periodically (size threshold or idle time) or on demand.
* **How:**

  1. Choose a **checkpoint boundary** `chk_seq` ≤ oldest active reader’s snapshot.
  2. For each page with newest frame ≤ `chk_seq`, copy the page image from WAL → main DB.
  3. `fsync(main DB)`; then mark frames ≤ `chk_seq` as checkpointed (can truncate or recycle WAL).
* **Readers:** unaffected; their snapshot\_seq remains valid.

### 5.6 Latches vs. Locks

* **Latches (mutexes):** short‑critical‑section mutual exclusion for in‑memory structures (page frames, B‑tree node edits, wal\_index). Never held across I/O.
* **Locks (txn‑level):** logical coordination; we keep a **single writer lock**; readers acquire no DB‑wide lock.
* **Deadlock policy:** acquire locks/latches in a fixed order (root→leaf), avoid holding them during disk I/O, and time‑bound retries for fairness.

### 5.7 Failure & Recovery

* On startup:

  * If WAL exists with frames beyond main DB, rebuild `wal_index` by scanning the WAL.
  * Last durable `global_seq` defines the latest committed state; discard frames after the last published commit if any.
* **Checksums** detect torn writes; **fsync ordering** ensures commit atomicity.

---

## 6) Execution Layer: Bytecode VM

* **Model:** register‑based with opcodes such as:

  * `OpenRead/OpenWrite`, `SeekEQ/SeekGE`, `First/Next`
  * `Column`, `Const`, `Compare`, conditional `Jump`
  * `ResultRow`, `Insert`, `Delete`, `Halt`
* **Why VM?** Clear separation between planning and storage; excellent for tracing and testing; easy to add new ops.
* **Concurrency interaction:** A VM instance runs inside a read or write transaction. Read VMs capture a `snapshot_seq` on open; write VMs hold the writer mutex only during WAL append windows, not for the entire interpretation.

---

## 7) Front‑End: Parser, Binder, Planner

* **Parser/AST:** hand‑written for transparency; AST nodes for `Select`, `Insert`, `CreateTable`, etc.
* **Binder:** resolves names to catalog IDs; attaches types/affinities.
* **Planner (v0.3):** rule‑based decisions: equality → index seek; range → index scan; else table scan; nested‑loop joins later.
* **Explainability:** `EXPLAIN` prints physical plan and optional VM listing.

---

## 8) Threading Model & Shared State

* **Threads:** multiple worker threads may run read VMs concurrently; at most **one writer VM** in commit path.
* **Shared structures:**

  * **Pager cache:** thread‑safe; protects page frames with fine‑grained mutexes.
  * **wal\_index:** resides in shared memory (or a process‑local shared segment); protected by RW lock: readers do lock‑free reads with epoch fencing or shared locks; writer updates atomically on commit.
* **Memory visibility:** publish‑subscribe via atomic sequence numbers (`global_seq`); readers snapshot the value once at txn start.

---

## 9) Invariants & Safety Checks

* B‑tree: keys sorted; min/max fanout; no dangling overflow; parent pointers consistent.
* Pager: never evict pinned; dirty pages flushed only by Txn manager.
* WAL: frames are append‑only; checksums verified on read; `global_seq` monotonically increases.
* VM: opcodes validated before execution; cursor bounds checked.

---

## 10) Testing & Tooling

* **Golden tests:** open known DB images; verify header/record decoding.
* **Randomized B‑tree tests:** insert/delete/merge; assert invariants.
* **Crash tests:** deterministic kill during commit; verify recovery.
* **Concurrency tests:** many readers + one writer; ensure readers never block and see consistent snapshots.
* **Bench:** page‑cache hit/miss, seek latency, scan throughput; WAL append & checkpoint timings.

---

## 11) Roadmap Hooks (Implementation Order)

1. Record codec + read cursors (v0.1)
2. Pager RAII + B‑tree read path (v0.1)
3. Write path (split/merge) + catalog (v0.2)
4. **WAL foundation**: frame format, append, commit publication, recovery (v0.2→v0.4)
5. Rule‑based planner + EXPLAIN (v0.3)
6. VM opcodes for SELECT/INSERT/DELETE/UPDATE (v0.4)
7. Checkpointing & shared wal\_index (v0.4)

---

## 12) Appendix: Key Data Structures

```cpp
// Shared WAL metadata (conceptual)
struct WalIndex {
  std::atomic<uint64_t> global_seq;     // published commit sequence
  // hash map: page_id -> frame_no (latest <= global_seq)
  ConcurrentMap<PageId, uint64_t> latest_frame;
  // epoch/counter to help readers detect rotation/truncation
  std::atomic<uint64_t> generation;
};

// B-Tree cursor (simplified)
struct Cursor {
  PageId page; int cell; std::vector<PageId> parents;
  bool at_end;
};

// Page cache entry
struct Frame { PageId id; std::mutex m; uint32_t pin; bool dirty; std::array<uint8_t, PAGE_SZ> bytes; };
```

---

## 13) Why These Choices 

* **B+Trees over data structures:** It's the most widely used storage backend for DBs and has been proven to be customisable to keep up with alternatives. It follows the principle of breadth-first and depth when needed.
* **WAL over rollback‑journal for concurrency:** One of the objectives of this project is learning how to design and implement multithreaded data structures.
* **Single writer policy:** Simplifies my life. Multiple write is an interesting technical problem, but very complex to implement.
* **VM execution:** promotes **explainability** and **portability**; Codegen/Volcano is most definetely faster, but are also a lot more complex. Same justification applies as in the "Single Write policy".
* **Rule‑based planner (initially):** In practice, good rules provide extremely good performance for the queries designed for the DB wihout compromising on accuracy, to the dismay of query optimization evangelists.

---

## 14) Future Extensions

* **Cost‑based optimizer** with page‑I/O estimates.
* **Row‑level locking** or predicate locking (if multi‑writer is desired).
* **Compression** for records or pages.
* **Mmap I/O path** with careful write barriers.
* **Foreign keys / triggers / views** once core stability is proven.

---

*This document is the architectural north star for the project. Keep interfaces narrow, invariants explicit, and tests merciless.*
