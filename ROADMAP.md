# ROADMAP.md — DIY SQLite

This roadmap captures the phased development plan for DIY SQLite, aligning with the architecture described in DESIGN.md. It incorporates WAL to support concurrent readers and a single writer, following a correctness-first, incremental implementation strategy.

---

## v0.1 — Readable DB (Read‑Only)

**Focus:** Basic storage, decoding, and traversal.

* Implement record codec (varint + serial types).
* Pager with RAII handles, pin/unpin, dirty marking, LRU eviction.
* B‑tree read path for table trees: open, seek, iterate.
* CLI for \d, SELECT \* LIMIT N.
* Tests: golden DB files, b‑tree scan invariants.

## v0.2 — Writes, Catalog & WAL Foundation

**Focus:** Mutation support, schema persistence, WAL basics.

* Insert/delete/merge in b‑trees.
* Create/drop table; system catalog persistence.
* WAL append path: frame format, writer mutex, commit publication.
* WAL recovery on startup.
* Tests: crash injection during commit, schema reload, write/read consistency.

## v0.3 — Indexes & Planner

**Focus:** Secondary indexes and basic planning.

* Create/drop indexes; unique enforcement.
* Planner: rule‑based selection between table scan, index seek, and range scan.
* EXPLAIN output for plans.
* Tests: index utilization, uniqueness violations, plan selection changes with/without index.

## v0.4 — VM, Concurrency & Checkpointing

**Focus:** Full execution layer and WAL concurrency model.

* Bytecode VM: SELECT/INSERT/DELETE/UPDATE opcodes.
* Multi‑reader, single‑writer WAL coordination; snapshot isolation.
* Checkpointing: size/idle triggers; truncate/recycle WAL.
* Tests: concurrent readers/writer consistency, checkpoint correctness, performance benchmarks.

---

## Cross‑Cutting Deliverables

* **Testing:** fuzz b‑trees, concurrency stress, recovery correctness.
* **Benchmarks:** page cache hit/miss, b‑tree seeks, WAL append, checkpoint time.
* **Docs:** update DESIGN.md alongside code; developer guides.

---

## Order of Implementation

1. Record codec + read cursors (v0.1)
2. Pager RAII + b‑tree read path (v0.1)
3. Write path + catalog (v0.2)
4. WAL append, commit, and recovery (v0.2)
5. Secondary indexes + planner (v0.3)
6. VM opcodes for DML/SELECT (v0.4)
7. WAL concurrency + checkpointing (v0.4)

---

**Milestones:**

* **v0.1:** Single-threaded, read-only.
* **v0.2:** Single-threaded writes, WAL foundation.
* **v0.3:** Indexes and basic query optimization.
* **v0.4:** Concurrent readers, single writer, full WAL.

---

*This roadmap should be updated as implementation progresses, with each milestone tested and documented before moving to the next.*
