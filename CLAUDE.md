# CLAUDE.md

Context for Claude Code working in this repo. The full design lives in `SPEC.md` — read it
before making changes. Keep this file current and under ~200 lines.

## What this is

Open-source FPV quadcopter simulator for **Windows + Linux desktop**, built to train a
school drone team. The fidelity benchmark is VelociDrone-level physics and low
input-to-photon latency, reached iteratively. V1's job is to get the architecture right so
fidelity becomes a tuning problem, not a rewrite.

Project name: **Propwash**.

## Tech stack

- **Engine:** Godot 4.x (MIT). Handles rendering, windowing, joystick/HID input, UI, scenes.
- **Simulation core:** C++ via a `godot-cpp` **GDExtension**. Contains the 6-DOF rigid-body
  integrator, motor model, quad-X mixer, flight-controller PID loop, and aerodynamic forces.
  **Do NOT use Godot's built-in rigid body / Jolt physics for the drone** — we integrate our
  own state.
- **UI / glue / level assembly:** GDScript.
- **Build:** SCons (the `godot-cpp` standard); CMake is acceptable. Finalize in M0.

## Build & run (placeholders — finalize during M0)

```bash
# One-time: fetch godot-cpp (as a submodule) and build the C++ bindings + extension
git submodule update --init --recursive

# Linux (debug)
scons platform=linux target=template_debug

# Windows (from a Developer Command Prompt, debug)
scons platform=windows target=template_debug

# Run: open the Godot project in the editor, or
godot --path godot/
```

Record the final, verified commands here after M0, for both Windows and Linux.

## Architecture rules (do not violate without asking)

- **Decouple physics from rendering.** Fixed-step physics/FC loop with an accumulator;
  render frames interpolate between the two most recent physics states.
- **Parameterizable tick rate.** Physics/FC step is a setting (default 500 Hz; range
  ~250–2000 Hz). FC loop rate may be an integer multiple of the physics rate — keep explicit.
- **Drone parameters are DATA** (resource/JSON the tuning UI edits), never hardcoded.
- **Aerodynamics = pluggable force/torque contributors** (drag, ground effect, prop wash...),
  so new effects drop in without touching the integrator.
- **Sim core must be unit-testable without the renderer.** Integrator, mixer, and PID have
  tests.
- **Latency:** sample controller input as late as possible before each step; keep the
  present path lean; v-sync / fps cap / tick rate are user-configurable settings.

## Flight model summary (see SPEC.md §5 for detail)

- 6-DOF rigid body (configurable mass + inertia tensor).
- Motors: command → RPM with first-order lag; thrust ∝ RPM²; reaction yaw torque. Battery
  sag is a stubbed hook for later.
- Quad-X mixer with saturation handling.
- Flight controller: gyro → per-axis rate PID (P, I with anti-windup hook, D with lowpass);
  gyro + D-term filters; Betaflight/Actual rate curves (RC rate, super rate, expo).
- Modes: Acro (rate), Angle (self-leveling), Horizon (blend). No GPS modes.
- Aerodynamics v1: linear + quadratic drag, ground effect, simple prop wash.

## Code style

- **C++:** modern C++17/20; small focused translation units; no exceptions across the
  GDExtension boundary; prefer plain data structs for sim state; keep engine types out of
  the core math where reasonable so the sim stays portable.
- **GDScript:** for UI/HUD/level glue only; no flight-model logic in GDScript.
- Use clear units and document them (SI throughout: metres, kg, seconds, radians).

## Constraints / non-goals (v1)

No GPS / position-hold / RTH. No multiplayer or networking. No web/browser build. No mobile.
No radio-link simulation — the transmitter is read as a USB joystick (EdgeTX USB Joystick /
HID mode).

## Input

RadioMaster Pocket over USB in EdgeTX **USB Joystick (HID)** mode, read as a generic
controller. Provide channel mapping + endpoint calibration + deadband, persisted. Support a
generic gamepad fallback and keyboard for testing before hardware is connected.

## Workflow for Claude Code

- Work **one milestone at a time** (M0–M8 in `SPEC.md`). For each: short plan + acceptance
  criteria → implement → tell me how to build/verify on **both** Windows and Linux → commit
  with a clear message.
- **Ask before** adding any heavy third-party dependency.
- Write/extend tests for the sim core as you go.
- Update this file when commands, architecture, or status change.

## Status

- Current milestone: **M0 — not started.**
- License: MIT.
