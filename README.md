# Propwash

An open-source **FPV drone simulator** for Windows and Linux, built to train a school drone
team. The aim is faithful acro flight feel — a full flight-controller PID loop and
Betaflight-style rates — with low, configurable input latency. It's a free, hackable
alternative to commercial sims for clubs and classrooms.

**Status:** 🚧 Early development. The architecture and physics core come first; polish and
fidelity tuning follow.

## Why

Commercial FPV sims cost money per seat, which adds up for a school drone team. This project
gives students a free, open tool to build stick time and learn tuning — and gives anyone the
ability to read, modify, and extend the flight model.

## Planned features (v1)

- 6-DOF quad physics with a real flight-controller PID loop (P/I/D + gyro & D-term filters).
- **Acro, Angle, and Horizon** flight modes (no GPS).
- Betaflight / Actual **rate curves** (RC rate, super rate, expo).
- Two drone classes to start: **5-inch** and **tinywhoop**, defined as editable data.
- Extensible aerodynamics: drag, ground effect, prop wash (more as pluggable effects).
- **RadioMaster Pocket** support via EdgeTX USB Joystick mode (plus gamepad/keyboard fallback).
- **Free-fly** and **time-trial** modes on **procedurally generated** maps and gate courses.
- Full **live tuning UI** (rates, PIDs, filters, drone params) with save/load profiles.
- Configurable physics tick rate, v-sync, and fps cap for a lean, low-latency path.

## Tech stack

- **[Godot 4.x](https://godotengine.org/)** — rendering, input, UI, scenes (MIT-licensed).
- **C++** simulation core via a `godot-cpp` GDExtension (custom integrator, motor model,
  mixer, flight controller, aerodynamics).
- **GDScript** for UI and level glue.

## Requirements

- Godot 4.x
- A C++ toolchain (MSVC on Windows; GCC/Clang on Linux) and SCons (or CMake)
- A RadioMaster Pocket (or any USB gamepad) — keyboard works for testing
- Windows 10+ or a modern Linux distro

## Building

> Commands are finalized in milestone M0; see `CLAUDE.md`.

```bash
git clone <repo-url>
cd propwash
git submodule update --init --recursive

# Linux (debug)
scons platform=linux target=template_debug

# Windows (Developer Command Prompt, debug)
scons platform=windows target=template_debug
```

Then open the Godot project in the editor to run.

## Controls (RadioMaster Pocket)

1. On the Pocket (EdgeTX), enable **USB Joystick / Game Controller** mode and plug in via USB.
2. In the sim, open the input settings, **map channels** (roll, pitch, yaw, throttle, arm,
   mode), and **calibrate** stick endpoints.
3. Arm and fly. Mappings are saved between sessions.

No gamepad? Keyboard controls are available for testing the flight model.

## Roadmap

Built in verifiable milestones (full detail in `SPEC.md`):

- **M0** — Repo + Godot/C++ scaffold building on Windows + Linux
- **M1** — Rigid body, motors, mixer (direct throttle)
- **M2** — Flight controller + Acro mode
- **M3** — RadioMaster Pocket input + mapping/calibration
- **M4** — Angle/Horizon modes + tuning UI + drone presets
- **M5** — Aerodynamics (drag, ground effect, prop wash) + configurable tick rate
- **M6** — Procedural maps, gate courses, time-trial timing + HUD
- **M7** — Latency hygiene + timing overlay
- **M8** — Audio, menus, real assets, packaged builds

## Contributing

Contributions are welcome — especially from drone-team students. Open an issue to discuss a
change before a large PR. See `SPEC.md` for the design and `CLAUDE.md` for build and
architecture rules.

## Assets & credits

3D models and textures are sourced from permissively licensed (CC0 preferred) libraries;
sources and licenses are listed in `ASSETS.md`.

## License

[MIT](LICENSE) © 2026 pengle01
