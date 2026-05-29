# Architecture

## 1. Overview

The Smart Door ECU simulation is designed as a **modular, single-process application** that mimics the behavior of a Body Control ECU.

The system is built to ensure:
- clear separation of responsibilities
- testability
- maintainability
- extensibility for future enhancements

---

## 2. High-Level Architecture

```text
+------------------------------------------------------+
|                      EcuApp                          |
|------------------------------------------------------|
| Application loop                                     |
| Command dispatch                                     |
| Timeout handling                                     |
| Persistence trigger                                  |
| Event logging                                        |
+--------------------------+---------------------------+
                           |
        +------------------+------------------+
        |                                     |
        v                                     v
+----------------------+          +----------------------+
|   DoorController     |          |   CanInterface       |
|----------------------|          |----------------------|
| State machine        |          | SocketCAN wrapper    |
| Business rules       |          | RX/TX                |
| Lamp timeout         |          | Poll-based receive   |
| Fault logic          |          +----------------------+
+----------------------+
        |
        +------------------------------+
        |                              |
        v                              v
+----------------------+      +--------------------------+
|   StateStorage       |      | EventLog / FileLogger    |
|----------------------|      |--------------------------|
| Lock-state           |      | In-memory history        |
| persistence          |      | Persistent file logging  |
+----------------------+      +--------------------------+
````

***

## 3. Module Responsibilities

### 3.1 EcuApp

* Acts as the **application orchestrator**
* Runs the main loop
* Handles:
  * CAN receive polling
  * command routing
  * timeout handling
  * state persistence trigger
  * event logging

***

### 3.2 CanInterface

* Encapsulates Linux **SocketCAN API**
* Responsibilities:
  * Open CAN socket
  * Bind to interface (`vcan0`)
  * Send CAN frames
  * Receive CAN frames (blocking and timeout-based)

***

### 3.3 DoorController

* Implements the **core ECU logic**
* Maintains internal state:
  * Lock
  * Door
  * Lamp
  * Fault
* Enforces rules:
  * lock allowed only when door closed
  * duplicate commands ignored
* Handles time-based logic:
  * lamp timeout after door close

***

### 3.4 StateStorage

* Provides simple persistence using file system
* Responsibilities:
  * Save lock state
  * Restore lock state during startup

***

### 3.5 EventLog (In-Memory)

* Maintains a **bounded event history buffer**
* FIFO / ring buffer behavior
* Useful for quick inspection/debugging

***

### 3.6 EventFileLogger

* Writes events to persistent file (`smartdoor_events.log`)
* Append-only logging
* Maintains history across ECU restarts

***

## 4. Data Flow

### Command Processing Flow

```text
CAN RX → decodeCommand → DoorController → update state →
(optional persistence) → build response → CAN TX
```

***

### Timeout Flow

```text
poll timeout → DoorController.tick() →
if state changed → send status frame
```

***

## 5. Design Principles

This project follows key engineering principles:

* **Separation of Concerns**
  * Communication, business logic, persistence are isolated

* **Single Responsibility**
  * Each module has a clearly defined role

* **Deterministic Behavior**
  * State transitions are rule-based and predictable

* **Testability**
  * Logic modules are testable independently (unit tests)

* **Minimal Dependencies**
  * Standard C++ + Linux APIs only

***

## 6. Execution Model

* Single-threaded
* Poll-based event loop
* Command-driven with periodic timeout checks

***

## 7. Configuration

Project uses centralized constants:

* CAN IDs
* timeouts
* file paths

Defined in:

```
include/project_config.hpp
```

***

## 8. Future Extension Points

Although not implemented (by design), the architecture allows:

* Additional commands
* More fault types
* Hardware abstraction layer
* Multi-node simulation
* Diagnostic interfaces (UDS)

***

## Summary

The architecture demonstrates how a small ECU can be designed with:

* clear layering
* controlled dependencies
* scalable structure

This makes it suitable both for learning and for PoC demonstration.