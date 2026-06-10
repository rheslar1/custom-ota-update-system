# Validation Plan

## Current Host Checks

- CMake configure completes.
- C++17 OTA planner builds.
- Executable emits accepted plans for Embedded Linux A/B rootfs and MCU dual-slot firmware.
- CTest verifies Linux rootfs policy, MCU policy, staged rollout, HTTPS enforcement, and rollback behavior.
- GitHub Actions runs configure, build, executable smoke run, and CTest.

## Hardware Evidence To Add

- Linux systemd service log for manifest fetch and inactive rootfs install.
- RAUC/Mender/SWUpdate install transcript.
- MCUboot or ESP-IDF OTA slot-state transcript.
- Failed trial boot rollback evidence.
- Signed manifest and SHA-256 verification evidence.
- CI screenshot after the public repository is pushed.

## Project-Specific Evidence Target

Production deployment pipelines, secure update handling, flash layout discipline, and field recovery behavior.
