# Limitations

## 1. Overview

This project is a **simulation-based Proof of Concept (PoC)**.

It is intentionally simplified and **does not represent a production ECU implementation**.

---

## 2. Functional Limitations

- Supports a **single ECU domain (Smart Door) only**
- Limited command set
- Basic fault model (single fault type)
- No multi-ECU communication

---

## 3. Platform Limitations

- Runs only on **Linux**
- Requires **SocketCAN**
- Depends on `vcan0` (virtual CAN)

---

## 4. Real-Time Limitations

- Single-threaded execution
- No real-time scheduling guarantees
- Poll-based timing (not deterministic)

---

## 5. Persistence Limitations

- File-based persistence used instead of NVM
- No wear-leveling or redundancy
- No concurrency protection for file access

---

## 6. Safety and Compliance

- Not compliant with:
  - AUTOSAR
  - ISO 26262
- No safety mechanisms
- No watchdog or fault recovery strategy

---

## 7. Communication Limitations

- No support for:
  - UDS (diagnostic protocol)
  - ISO-TP
  - CAN FD
- Simple single-frame message model

---

## 8. Security Limitations

- No authentication on commands
- No encryption
- System assumes trusted environment

---

## 9. Logging Limitations

- Append-only event log file
- No log rotation
- No timestamping (optional future enhancement)

---

## 10. Scalability Limitations

- Designed for single-node simulation
- Not optimized for large-scale or multi-threaded systems

---

## 11. Purpose Clarification

This project is intended for:

- learning and training
- demonstration of ECU fundamentals
- simulation-based experimentation

It is **not intended for production use**.

---

## Summary

These limitations are intentional, to:

- keep the design simple
- focus on core learning objectives
- enable fast experimentation without complexity