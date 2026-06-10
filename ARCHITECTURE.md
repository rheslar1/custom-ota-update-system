# Custom OTA Update System Architecture

## Goal

Production deployment pipelines, secure update handling, flash layout discipline, and field recovery behavior.

## Runtime Shape

1. The device reports target type, hardware revision, active version, rollback counter, and rollout bucket.
2. The OTA service fetches a signed manifest over HTTPS.
3. The planner validates slot layout, target compatibility, staged rollout, artifact policy, signature, digest, and rollback counter.
4. MCU targets install firmware to the inactive slot.
5. Embedded Linux targets install a signed rootfs or bundle artifact to the inactive A/B rootfs slot.
6. Failed trial boots roll back to the previous confirmed slot.

## C++17 Design Shape

- `OtaUpdatePlanner` owns install, stay-current, rollback, and reject decisions.
- `IManifestVerifier` isolates cryptographic verification from update policy.
- `SimulatedManifestVerifier` provides deterministic CI behavior.
- `TextPlanReporter` serializes evidence for agent or systemd logs.

## SOLID Notes

- Single Responsibility: manifest verification, policy planning, slot state, and reporting are separated.
- Open/Closed: RAUC, Mender, SWUpdate, MCUboot, or ESP-IDF installers can attach behind the same plan output.
- Liskov Substitution: any verifier can replace the simulator.
- Interface Segregation: manifest verification is a focused interface.
- Dependency Inversion: planning depends on verifier abstraction, not a concrete crypto library.

## Boundaries

- `include/ota/`: OTA data model, verifier interface, and planner API.
- `src/`: OTA planner, simulated verifier, and CLI demo.
- `docs/`: validation plans, timing notes, hardware captures, and acceptance evidence.
- `tests/`: host-side tests for update, rollback, rollout, and Linux policy decisions.
- `.github/workflows/`: CI entry point for build and validation evidence.

## Validation Plan

- Build the host OTA planner with CMake.
- Run the executable and confirm both Embedded Linux and MCU plans are accepted.
- Run CTest to validate Linux rootfs, MCU, rollout, URI, signature, and rollback gates.
- Add device logs after wiring to a real installer.
- Capture CI, terminal, and hardware evidence for the portfolio detail page.

## Expansion Notes

- Connect Linux plans to RAUC, Mender, or SWUpdate.
- Connect MCU plans to MCUboot, ESP-IDF OTA, or STM32 flash writers.
- Persist slot state and rollback counters across reboot.

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
