# Custom OTA Update System

Secure dual-partition firmware update flow for ESP32 or STM32 with signed image verification, staged rollouts, and automatic fallback.

## Portfolio Purpose

This repository is an Embedded Systems project scaffold for the Rheslar portfolio. It is designed to become a hardware-backed project with build output, validation logs, and reviewable implementation evidence.

## Stack

- ESP32/STM32
- Dual partition
- MCUboot
- ECDSA
- TLS
- Rollback logic

## Quick Start

```bash
cmake -S . -B build
cmake --build build
./build/custom_ota_update_system
python -m unittest discover -s tests
```

## Implementation Slices

- Native starter executable that exposes the project identity, stack, and validation target.
- Architecture document with control boundaries, data flow, safety assumptions, and evidence plan.
- Unit smoke test that keeps source, docs, and CI files present as the repo grows.
- GitHub Actions workflow for configure, build, executable smoke run, and repository validation.

## Evidence Target

Production deployment pipelines, secure update handling, flash layout discipline, and field recovery behavior.

## Remote

Intended public repository: https://github.com/rheslar1/custom-ota-update-system
