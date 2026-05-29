# Test Scenarios

## 1. Overview

This document defines **functional test scenarios** used to validate the Smart Door ECU simulation.

These scenarios are used for:
- manual validation
- integration testing
- demo execution

---

## 2. Test Environment

- Linux system
- `vcan0` interface enabled
- `candump` for monitoring
- `cansend` for command injection

---

## 3. Scenario 1 — Lock and Persistence

### Objective
Validate correct lock behavior and persistence across restart.

### Steps
1. Start ECU
2. Send LOCK command
3. Observe status frame
4. Stop ECU
5. Restart ECU

### Expected Result
- LockState = LOCKED before shutdown
- After restart → restored lock state = LOCKED

---

## 4. Scenario 2 — Unlock Flow

### Objective
Validate UNLOCK command behavior.

### Steps
1. Ensure ECU is locked
2. Send UNLOCK command

### Expected Result
- LockState changes to UNLOCKED
- FaultState = NONE

---

## 5. Scenario 3 — Invalid Lock (Door Open)

### Objective
Validate rule enforcement.

### Steps
1. Send DOOR_OPEN
2. Send LOCK

### Expected Result
- LockState remains UNLOCKED
- DoorState = OPEN
- FaultState = LOCK_DENIED_DOOR_OPEN

---

## 6. Scenario 4 — Reset Fault

### Objective
Validate fault clearing without altering state.

### Steps
1. Trigger fault (Scenario 3)
2. Send RESET_FAULT

### Expected Result
- FaultState = NONE
- Other states unchanged

---

## 7. Scenario 5 — Lamp Timeout

### Objective
Validate time-based behavior.

### Steps
1. Send DOOR_OPEN (lamp ON)
2. Send DOOR_CLOSE
3. Wait for timeout

### Expected Result
- Lamp remains ON immediately after close
- Lamp turns OFF after timeout

---

## 8. Scenario 6 — Duplicate Command Handling

### Objective
Validate idempotency.

### Steps
1. Send LOCK twice
2. Send UNLOCK twice

### Expected Result
- Second command ignored
- No state change
- No fault generated

---

## 9. Scenario 7 — READ_STATUS Command

### Objective
Validate diagnostic command.

### Steps
1. Send READ_STATUS

### Expected Result
- Status frame is sent
- ECU state remains unchanged
- Event history printed

---

## 10. Scenario 8 — CAN Validation

### Objective
Verify CAN communication.

### Steps
1. Use `candump vcan0`
2. Send commands via `cansend`

### Expected Result
- Correct response frames observed on `0x201`

---

## 11. Automation

All scenarios are covered in:

```
scripts/test\_integration.sh
```

---

## Summary

These scenarios validate:

- functional correctness
- rule enforcement
- timeout behavior
- persistence
- system robustness

They collectively demonstrate full PoC coverage.