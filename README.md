# Custom OTA Update System

Secure dual-partition firmware update flow for ESP32 or STM32 with signed image verification, staged rollouts, and automatic fallback.

## Portfolio Purpose

This repository implements a host-testable OTA update planner for both MCU dual-partition firmware and Embedded Linux A/B rootfs systems. It validates signed manifests, HTTPS artifact policy, staged rollout buckets, rollback counters, slot health, Linux rootfs bundle requirements, and automatic fallback behavior.

## Stack

- C++17
- C++ Design Patterns
- SOLID
- ESP32/STM32
- Dual partition
- MCUboot
- ECDSA
- TLS
- Rollback logic
- Embedded Linux
- A/B rootfs

## Quick Start

```bash
cmake -S . -B build
cmake --build build
./build/custom_ota_update_system
ctest --test-dir build --output-on-failure
```

## Implementation Slices

- MCU firmware update planning for inactive slot installation.
- Embedded Linux A/B rootfs update planning for RAUC/Mender-style bundles.
- HTTPS, signature, SHA-256, rollback counter, target compatibility, and staged rollout gates.
- Failed trial boot rollback to the previous confirmed slot.
- Text evidence reports for deployment logs.
- CTest coverage for Linux rootfs updates, MCU updates, rollout rejection, insecure URI rejection, and rollback.

## Evidence Target

Production deployment pipelines, secure update handling, flash layout discipline, and field recovery behavior.

## Remote

Intended public repository: https://github.com/rheslar1/custom-ota-update-system

<!-- cpp17-solid-implementation:start -->
## C++17, Design Patterns, and SOLID Implementation

This repository includes a host-buildable C++17 implementation, not only documentation. The implementation applies:

- Strategy pattern for validation rules.
- Adapter interfaces for input samples and telemetry/reporting.
- Composite validation for combining safety and readiness checks.
- Facade orchestration through the project runtime class.
- SOLID boundaries between profile data, input acquisition, validation, telemetry encoding, and tests.
<!-- cpp17-solid-implementation:end -->

<!-- deep-architecture-links:start -->
## Deep Architecture and UML

- [Deep architecture](docs/deep-architecture.md)
- [Full UML Draw.io source](docs/diagrams/full-system-uml.drawio)
- [Full UML PNG export](docs/diagrams/full-system-uml.png)
<!-- deep-architecture-links:end -->
