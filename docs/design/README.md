# Custom OTA Update System Design Package

## Purpose

Secure dual-partition firmware update flow for ESP32 or STM32 with signed image verification, staged rollouts, and automatic fallback.

This package defines the project as an implementation-ready embedded system. It covers system architecture, requirements, interface boundaries, runtime design, validation evidence, and phased delivery.

## Project Profile

| Field | Value |
| --- | --- |
| Repository | `rheslar1/custom-ota-update-system` |
| Primary stack | C++17, C++ Design Patterns, SOLID, ESP32/STM32, Dual partition, MCUboot, ECDSA, TLS, Rollback logic |
| Review proof point | Production deployment pipelines, secure update handling, flash layout discipline, and field recovery behavior. |

## Artifacts

- [System Design](system-design.md)
- [Requirements](requirements.md)
- [Interface Control](interface-control.md)
- [Runtime Design](runtime-design.md)
- [Validation Plan](validation-plan.md)
- [Implementation Roadmap](implementation-roadmap.md)
- [Draw.io UML](diagrams/system-design.drawio)
- [PNG UML](diagrams/system-design.png)
