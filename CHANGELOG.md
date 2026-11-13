# Changelog

All notable changes to the controller firmware. Newest first.
The car and the controller must run the same **protocol version**.

## 2026-11-12

### Protocol version 2 (update the car at the same time)
- Every packet now starts with a protocol version number. Packets from mismatched firmware are ignored.
- CONNECTED now means the car is really replying, not just that its radio is on.
- New screens: **NO REPLY** (car is on but not answering: re-pair or update it) and **FIRMWARE / Mismatch** (car runs a different protocol version).
- The drive channel is sent to the car during pairing.
- Battery voltage on screen updates 5 times a second.

### Web flasher
- Web flashing no longer erases the saved pairing or settings.

### Repository
- Specifications moved to `docs/`. Pinned copy of U8g2 2.36.19 in `libraries/`. Added `README.md`, `LICENSE` (MIT) and this changelog.

### Earlier on Nov 12
- Board select line, matching pairing handshake, menu and screen fixes.

Earlier history: see the git log.
