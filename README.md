# diy-sqlite

This is the diy-sqlite project written in C++17 (Clang). The goal of this project is to make a simple educational DB with transactional properties ensured via WAL that can 
parse simple SQL queries. It's a disk DB much like SQLite with indexes based on a B-tree. The commands are executed on a VM and there are plans for multithreadded support.

It takes heavy inspiration from [Edward Sciore's Database Design and Implementation](https://link.springer.com/book/10.1007/978-3-030-33836-7) as well as the [SQLite Docs](https://sqlite.org/docs.html).

# Goals
- Parse a real .db file
- Traverse a B-tree
- Execute simple SQL Queries via a bytecode VM
- Thorough testing 
- Concurrent (Single Writer, Multiple Readers) should be supported

# Non-Goals
- Cost based query optimizer
- Foreign Keys
- Advanced Types
- Triggers
- Views
- VACUUM
- Page-level encryption

# Design 

See [DESIGN](DESIGN.md) for a design overview

# Roadmap

See [ROADMAP.md](ROADMAP.md) for a roadmap of this project

# Building and installing

See the [BUILDING](BUILDING.md) document.
