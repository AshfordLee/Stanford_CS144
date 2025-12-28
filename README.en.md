Stanford CS144 — Project Overview
=================================

This repository contains code and exercises from the Stanford CS144 networking labs (the "minnow" implementation and related checkpoints). This README provides a short overview, instructions for building and running tests, a navigational file-tree, and brief descriptions of key modules to help you get started quickly.

Quick start (build & run tests)

- Configure and build the project (CMake):

```bash
cmake -S . -B build
cmake --build build
```

- Run all tests (CMake `test` target):

```bash
cmake --build build --target test
```

(Optional) Use ctest for verbose output:

```bash
ctest --test-dir build -V
```

Common CMake targets (see `minnow/README.md` for more):
 - `cmake --build build --target speed` — run speed benchmarks
 - `cmake --build build --target tidy` — run clang-tidy suggestions
 - `cmake --build build --target format` — auto-format source

Project layout (high-level)

The repository contains several top-level exercises and the `minnow/` networking implementation used in CS144:

- `Checkpoint0/`, `Check4_Analyzing_Data/` — course checkpoints / exercises
- `minnow/` — main networking stack implementation and tests
  - `minnow/CMakeLists.txt` — CMake build entry
  - `minnow/src/` — source code (protocol stack modules)
    - `wrapping_integers.hh` — Wrap32: 32-bit sequence number wrap/unwrap utilities
    - `network_interface.hh` — NetworkInterface: ethernet frame handling, ARP cache, send/recv logic
    - other `tcp_*`, `byte_stream*`, `reassembler*`, `router*` files implement TCP, byte streams, reassembly, routing, etc.
  - `minnow/tests/` — unit and integration tests
  - `minnow/apps/` — example/demo applications
  - `minnow/scripts/`, `minnow/util/`, `minnow/etc/` — helper scripts, utilities and configuration
  - `minnow/writeups/` — lab write-ups and notes

Key modules (brief)

- `wrapping_integers.*` — handles 32-bit sequence number arithmetic with `wrap`/`unwrap`.
- `network_interface.*` — links IP layer with Ethernet, maintains ARP cache, sends/receives frames, and handles ARP requests/replies.
- `tcp_*`, `byte_stream*`, `reassembler*` — simplified TCP sender/receiver logic, byte-stream interfaces, and segment reassembly. See `minnow/tests/` for many focused tests.

Running specific tests / debugging

- Run tests that match a regex using `ctest`:

```bash
ctest --test-dir build -V -R wrapping_integers
```

- You can also run individual test executables produced in `build/` (path depends on CMake configuration).

Standard file-tree (navigation)

Below is a canonical view of the repository structure (some generated/temporary folders omitted):

```text
/
├── Checkpoint0/
├── Check4_Analyzing_Data/
├── minnow/
│   ├── CMakeLists.txt
│   ├── README.md
│   ├── build/                 # local build output (ignore)
│   ├── src/
│   │   ├── wrapping_integers.hh
│   │   ├── network_interface.hh
│   │   ├── tcp_sender.hh
│   │   ├── byte_stream.hh
│   │   ├── reassembler.hh
│   │   └── ...                # other protocol stack sources
│   ├── tests/                 # unit & integration tests
│   ├── apps/                  # example/demo programs
│   ├── scripts/               # helper scripts (build/test/run)
│   ├── util/                  # utilities / shared code
│   └── writeups/              # lab reports / notes
├── .gitignore
└── README.md
```

