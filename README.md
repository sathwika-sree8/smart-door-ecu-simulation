# Smart Door ECU Simulation (C++ | SocketCAN | Automotive PoC)

![Language](https://img.shields.io/badge/language-C++-blue)
![Platform](https://img.shields.io/badge/platform-Linux-green)
![Domain](https://img.shields.io/badge/Domain-Automotive-orange)
![Type](https://img.shields.io/badge/Type-ECU%20PoC-yellow)


## Overview

This project is a **scratch-built automotive ECU simulation** developed in **C++** on **Linux**, using **SocketCAN** and a **virtual CAN interface (`vcan0`)**. The purpose is to help learners understand how an ECU can be designed, implemented, tested, and improved **without physical hardware**.

The project begins from a minimal CMake-based executable and grows milestone by milestone into a small but realistic **Smart Door ECU** with:

- command-driven state management
- CAN receive/transmit support
- lock / door / lamp behavior
- automotive-style validation rules
- fault handling and fault reset
- persistence of lock state across restart
- lamp timeout behavior
- in-memory event history and persistent event logging
- unit tests and integration testing

This README is intentionally written as a **learning guide**, not just a usage note. It is meant to help a reader:

1. understand what the project does
2. understand how the architecture is organized
3. build and run the project
4. test it manually and automatically
5. learn milestone by milestone what to implement and why

---

## Keywords

C++, Embedded Systems, Automotive Software, ECU Simulation, SocketCAN, CAN Bus, Linux CAN, State Machine, Automotive PoC, ECU Design, Embedded Learning, Body Control Module, Smart Door ECU

---

## PoC Objective

This project demonstrates a simulation-based Smart Door ECU implemented in C++ using Linux SocketCAN.

The PoC focuses on:
- Command-driven ECU behavior
- State-machine-based design
- Rule enforcement (e.g., lock restriction when door open)
- Fault reporting and reset
- Time-based behavior (lamp timeout)
- Persistent state across restart
- Testability using virtual CAN (vcan0)

This PoC intentionally avoids hardware dependency and complex frameworks to focus on core ECU concepts.

---

## Scope

### In Scope
- CAN Rx/Tx via SocketCAN
- Smart Door ECU state machine
- Command decoding
- Fault handling and reset
- Lamp timeout behavior
- Persistent lock state
- Event logging
- Unit + integration testing

### Out of Scope
- Hardware integration
- AUTOSAR compliance
- Real-time scheduling guarantees
- UDS diagnostics
- Functional safety (ISO 26262)

---

## Learning Objectives

By studying and implementing this project, a learner should understand how to:

- start a clean C++ project from scratch
- use **SocketCAN** for CAN communication on Linux
- use **`vcan0`** to simulate CAN traffic without hardware
- design a small ECU-style **state machine**
- separate communication, state logic, persistence, and runtime orchestration
- implement business rules, faults, and time-based behavior
- persist important state across restart
- build **unit tests** and **integration tests**
- structure code so it remains maintainable as complexity grows

---

## What the Project Simulates

This project simulates a **Smart Door ECU**.

The ECU listens for commands on **CAN ID `0x101`** and publishes status on **CAN ID `0x201`**.

### Supported Commands

| Command Byte | Meaning |
|---|---|
| `0x01` | `LOCK` |
| `0x02` | `UNLOCK` |
| `0x03` | `DOOR_OPEN` |
| `0x04` | `DOOR_CLOSE` |
| `0x05` | `RESET_FAULT` |
| `0x06` | `READ_STATUS` |

### Status Frame Format

The ECU responds on **CAN ID `0x201`** with 4 bytes:

- **Byte 0** → lock state
- **Byte 1** → door state
- **Byte 2** → lamp state
- **Byte 3** → fault state

### Modeled States

- **LockState**: `LOCKED`, `UNLOCKED`
- **DoorState**: `OPEN`, `CLOSED`
- **LampState**: `ON`, `OFF`
- **FaultState**:
  - `NONE`
  - `LOCK_DENIED_DOOR_OPEN`

---

## Architecture

## Architecture

The ECU is organized into modular components:

- `EcuApp` → application orchestration
- `DoorController` → state machine and rules
- `CanInterface` → SocketCAN communication
- `StateStorage` → persistent lock state
- `EventLog` → in-memory history
- `EventFileLogger` → persistent logging

📄 Detailed architecture:
docs/architecture.md

---

## Project Structure

A typical final structure looks like this:

```text
smartdoor-ecu-simulation/
├── CMakeLists.txt
├── smartdoor_state.txt            # generated at runtime
├── smartdoor_events.log           # generated at runtime
├── include/
│   ├── can_interface.hpp
│   ├── door_controller.hpp
│   ├── ecu_app.hpp
│   ├── event_file_logger.hpp
│   ├── event_log.hpp
│   ├── smartdoor_types.hpp
│   ├── smartdoor_utils.hpp
│   ├── state_storage.hpp
│   └── status_frame_builder.hpp
├── src/
│   ├── can_interface.cpp
│   ├── door_controller.cpp
│   ├── ecu_app.cpp
│   ├── event_file_logger.cpp
│   ├── event_log.cpp
│   ├── main.cpp
│   ├── smartdoor_utils.cpp
│   ├── state_storage.cpp
│   └── status_frame_builder.cpp
├── scripts/
│   ├── test_integration.sh
│   └── test_artifacts/
├── tests/
│   └── unit_tests.cpp
└── build/
```

---

## Prerequisites

### System Requirements

- Linux (recommended: Ubuntu)
- CMake
- GCC / G++
- `can-utils`
- virtual CAN support (`vcan`)

### Install Tools

```bash
sudo apt update
sudo apt install build-essential cmake can-utils
```

### Set Up Virtual CAN

```bash
sudo modprobe vcan
sudo ip link add dev vcan0 type vcan
sudo ip link set up vcan0
```

Verify interface:

```bash
ip link show vcan0
```

---

## Build Instructions

From project root:

```bash
cmake -S . -B build
cmake --build build
```

Run the ECU:

```bash
./build/smart_door_ecu
```

---

## How to Manually Test the ECU

### Terminal 1 — Run ECU

```bash
./build/smart_door_ecu
```

### Terminal 2 — Monitor CAN traffic

```bash
candump vcan0
```

### Terminal 3 — Send commands

#### Lock

```bash
cansend vcan0 101#0100000000000000
```

#### Unlock

```bash
cansend vcan0 101#0200000000000000
```

#### Door open

```bash
cansend vcan0 101#0300000000000000
```

#### Door close

```bash
cansend vcan0 101#0400000000000000
```

#### Reset fault

```bash
cansend vcan0 101#0500000000000000
```

#### Read status

```bash
cansend vcan0 101#0600000000000000
```

---

## Behavior Rules Implemented

### 1. Lock Rule
- `LOCK` is applied only if the door is closed.
- If the door is open, lock is rejected and `LOCK_DENIED_DOOR_OPEN` fault is set.

### 2. Duplicate Command Handling
- `LOCK` when already locked → ignored
- `UNLOCK` when already unlocked → ignored
- `DOOR_OPEN` when already open → ignored
- `DOOR_CLOSE` when already closed → ignored

### 3. Lamp Behavior
- `DOOR_OPEN` → lamp turns ON immediately
- `DOOR_CLOSE` → lamp stays ON for a timeout period, then turns OFF automatically

### 4. Fault Reset
- `RESET_FAULT` clears only the fault state
- it does not change lock / door / lamp state

### 5. Read Status
- `READ_STATUS` does not change ECU state
- it returns the current status and prints recent event history

### 6. Persistence
- lock state is saved whenever it changes
- on restart, ECU restores the last persisted lock state

---

## Runtime Files Generated

### `smartdoor_state.txt`
Stores persisted lock state.

Typical content:

```text
LOCKED
```

or

```text
UNLOCKED
```

### `smartdoor_events.log`
Stores persistent event history.

Example:

```text
============================================================
ECU SESSION START
============================================================
[1] No persisted state found. Starting with default UNLOCKED state.
[2] ECU initialized successfully
[3] Command received: LOCK
[4] Persisted lock state: LOCKED
[5] State after LOCK => lock=LOCKED, door=CLOSED, lamp=OFF, fault=NONE
[6] Status sent (LOCK): 1 0 0 0
...
ECU SESSION END
```

---

## Testing

### Unit Tests

Build first, then run:

```bash
ctest --test-dir build --output-on-failure
```

or:

```bash
./build/smart_door_ecu_tests
```

### Integration Test

If `scripts/test_integration.sh` is present and executable:

```bash
./scripts/test_integration.sh
```

This validates end-to-end ECU behavior using live `vcan0`, `candump`, and `cansend`.

---

# Milestone-by-Milestone Learning and Implementation Guide

This is the most important section for a learner.

Each milestone explains:

1. **what to learn**
2. **what to implement**
3. **what success looks like**
4. **the key lesson**

---

## Milestone 0 — Project Bootstrapping

### What to learn
- how to start a clean C++ project from scratch
- how to write a minimal `CMakeLists.txt`
- how to build and run a simple executable

### What to implement
- create project directory structure
- create `main.cpp`
- create minimal CMake build

### Success criteria
- project builds with CMake
- executable runs and prints startup message

### Key lesson
Before working on ECU logic, first make the project reproducible and buildable.

---

## Milestone 1 — CAN Interface Bring-Up

### What to learn
- how Linux SocketCAN works
- how to open a raw CAN socket
- how to bind to `vcan0`

### What to implement
- `CanInterface` class
- `open()` and `close()` logic

### Success criteria
- program successfully opens and binds to `vcan0`
- no hardware is required

### Key lesson
Communication infrastructure should be isolated from business logic.

---

## Milestone 2 — Receive One CAN Frame

### What to learn
- how CAN reception works in SocketCAN
- structure of `can_frame`
- how to interpret ID, DLC, and data bytes

### What to implement
- `receive()` in `CanInterface`
- receive one frame and print it

### Success criteria
- program receives a frame sent by `cansend`
- ECU prints the received frame correctly

### Key lesson
Validate the RX path independently before adding application logic.

---

## Milestone 3 — Transmit One CAN Frame

### What to learn
- how CAN transmission works in SocketCAN
- how to validate transmission with `candump`

### What to implement
- `send()` in `CanInterface`
- send a hard-coded frame

### Success criteria
- `candump vcan0` sees the transmitted frame

### Key lesson
Always validate TX and RX independently before combining them.

---

## Milestone 4A — Command Decoding + Internal State

### What to learn
- how to represent ECU state cleanly
- how to define command enums and state enums
- how to separate mode/state handling from communication

### What to implement
- `smartdoor_types.hpp`
- `DoorController`
- command decoding logic

### Success criteria
- one received command updates internal state correctly
- state can be printed readably

### Key lesson
Never mix state-machine logic directly into low-level communication code.

---

## Milestone 4B — Send Status Frame After Command

### What to learn
- how to convert internal state into wire-format CAN payload
- request → process → response pattern

### What to implement
- build status frame on CAN ID `0x201`
- transmit status after processing a command

### Success criteria
- after each valid command, `candump` shows the correct status response

### Key lesson
An ECU should not only consume commands; it must also provide observable output.

---

## Milestone 5 — Continuous ECU Loop

### What to learn
- how a long-running ECU process behaves
- how state evolves across multiple commands in one session

### What to implement
- continuous `while(true)` loop
- repeated receive → process → respond cycle

### Success criteria
- ECU keeps running until `Ctrl+C`
- state evolves across multiple commands

### Key lessona
A real ECU is a persistent reactive system, not a one-shot script.

---

## Milestone 6A — Realistic Rule: Lock Rejection if Door Open

### What to learn
- how to embed business / physical rules into ECU logic
- difference between valid commands and invalid requests

### What to implement
- reject `LOCK` if door is open
- introduce `FaultState`
- send fault byte in status frame

### Success criteria
- invalid `LOCK` does not change lock state
- fault byte indicates lock denied

### Key lesson
Automotive software is not just state update logic; it must enforce system rules.

---

## Milestone 6B — Debug Visibility for Applied / Ignored / Rejected Commands

### What to learn
- why observability matters in embedded systems
- why logs are essential during development and validation

### What to implement
- console messages inside `DoorController`
- indicate whether command was:
  - applied
  - ignored
  - rejected

### Success criteria
- logs clearly explain ECU decisions

### Key lesson
A correct ECU that cannot explain itself is hard to debug.

---

## Milestone 7 — Persistent Lock State

### What to learn
- what state should survive restart
- how to use file-based persistence as an NVM substitute in simulation

### What to implement
- `StateStorage`
- save lock state changes
- restore lock state on startup

### Success criteria
- lock state survives process restart

### Key lesson
Persistent state is essential for realistic ECU behavior.

---

## Milestone 8 — Lamp Timeout After Door Close

### What to learn
- how to implement time-based ECU behavior
- polling with timeout
- distinction between command-driven events and time-driven events

### What to implement
- `tick()` in `DoorController`
- delayed lamp OFF after door close
- `receiveWithTimeout()` using `poll()` in `CanInterface`

### Success criteria
- `DOOR_CLOSE` keeps lamp ON temporarily
- lamp turns OFF automatically after timeout

### Key lesson
An ECU must handle both **events** and **time**.

---

## Milestone 9 — Architectural Refactor

### What to learn
- when and why to refactor
- how helper modules improve readability and maintainability

### What to implement
- `smartdoor_utils`
- `status_frame_builder`
- simplify `main.cpp`

### Success criteria
- functionality remains unchanged
- code structure becomes cleaner and easier to extend

### Key lesson
Refactoring is not optional if you want long-term maintainability.

---

## Milestone 10A — Unit Testing

### What to learn
- difference between unit tests and integration tests
- how to validate logic without live CAN activity

### What to implement
- `tests/unit_tests.cpp`
- tests for:
  - `decodeCommand`
  - `DoorController`
  - `StateStorage`
  - status frame builder

### Success criteria
- tests pass via direct execution or CTest

### Key lesson
Unit tests protect logic from regressions during future changes.

---

## Milestone 10B — RESET_FAULT Command

### What to learn
- how to add a new command cleanly without breaking architecture
- side-effect-limited control commands

### What to implement
- `0x05 = RESET_FAULT`
- clear fault only, preserve all other states

### Success criteria
- fault can be set, then cleared independently

### Key lesson
Not every command changes the full system state; some commands affect only one subsystem.

---

## Milestone 10C — Automated Integration Script

### What to learn
- how to validate a running ECU end-to-end automatically
- how to script build, launch, inject, verify, and cleanup flow

### What to implement
- `scripts/test_integration.sh`
- build + run + candump + cansend + assertions

### Success criteria
- one script validates complete ECU behavior end-to-end

### Key lesson
Integration testing proves the whole system works together, not just isolated modules.

---

## Milestone 11 — Event History Buffer

### What to learn
- implementing lightweight diagnostics / trace history
- ring-buffer concept

### What to implement
- `EventLog`
- fixed-capacity in-memory history buffer

### Success criteria
- recent events can be dumped to terminal
- older entries roll out as new entries arrive

### Key lesson
A bounded event history is often more useful than uncontrolled logging.

---

## Milestone 12 — READ_STATUS Command

### What to learn
- distinction between control commands and diagnostic query commands
- no-side-effect runtime inspection

### What to implement
- `0x06 = READ_STATUS`
- respond with current status without changing ECU state
- dump recent event history to terminal

### Success criteria
- READ_STATUS returns the current state without disturbing operation

### Key lesson
A mature ECU should support observability without disturbing operation.

---

## Milestone 13 — `EcuApp` Class Refactor

### What to learn
- role of application orchestrator classes
- how to move runtime flow out of `main.cpp`

### What to implement
- `EcuApp`
- startup, loop, timeout handling, frame handling, and event logging orchestration

### Success criteria
- `main.cpp` becomes very small
- runtime loop is encapsulated cleanly

### Key lesson
As systems grow, top-level orchestration should be encapsulated.

---

## Milestone 15 — Persistent Event Logging to File

### What to learn
- difference between recent in-memory history and persistent log archive
- file-append logging strategy

### What to implement
- `EventFileLogger`
- persistent `smartdoor_events.log`
- session start/end markers
- append every important runtime event to file

### Success criteria
- event history survives ECU restart
- log file can be inspected after the process exits

### Key lesson
Persistent logs make the simulation much more useful for learning, debugging, and demonstration.

---

# How a Reader Should Study This Project

A good learner should **not** jump to the final code first.

Recommended order:

1. **Read the project overview and architecture**
2. **Understand CAN message format**
3. **Study `DoorController` carefully**
4. **Read `CanInterface` to understand SocketCAN usage**
5. **Read `EcuApp` to understand application flow**
6. **Run manual tests with `cansend` / `candump`**
7. **Run unit tests**
8. **Run integration script**
9. **Inspect `smartdoor_state.txt` and `smartdoor_events.log`**
10. **Walk milestone by milestone and ask: why was this added?**

The best learning comes from following the same progression in which the project was built.

---

# Suggested Exercises for Learners

A reader who wants to learn actively can try these exercises:

1. Add a new fault type.
2. Add a new command.
3. Change status frame format and update all affected modules.
4. Change lamp timeout duration.
5. Extend persistence beyond lock state.
6. Add timestamps to the event file.
7. Add file-rotation logic for large logs.
8. Make the integration script verify event file content too.

---

## Documentation

- [Architecture](docs/architecture.md)
- [Test Scenarios](docs/test_scenarios.md)
- [Limitations](docs/limitations.md)

---

# Key Takeaways

This project demonstrates an important engineering lesson:

> You do not need hardware to learn real embedded-system thinking.

With a Linux machine, SocketCAN, and disciplined milestone-based development, you can learn:

- communication
- state machines
- timing
- persistence
- validation
- architecture
- testability

in a very practical and reusable way.

---

## Usage Note

This project is intended as a **learning and teaching example**. If you use it in a lab, FDP, student project, or self-study module, it is strongly recommended that you preserve the milestone structure, because the milestone progression is one of the most valuable parts of the learning journey.