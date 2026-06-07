# FPV Drone Simulator — Project Spec & Claude Code Kickoff

> **How to use this with Claude Code**
> 1. Create an empty git repo and drop this file in as `SPEC.md`.
> 2. From the repo folder, run `claude`.
> 3. Paste the "Kickoff prompt" block below as your first message.
> Claude Code will read this spec and build the project milestone by milestone.

---

## Kickoff prompt (paste this into Claude Code)

You are helping me build an open-source FPV drone simulator from scratch, solo. I am
confident in C++ and familiar with Rust. The full spec is in `SPEC.md` in this repo —
read it fully before doing anything.

Working agreement:
- Work **one milestone at a time** (M0 → M8 in the spec). Do not scaffold everything at once.
- At the start of each milestone, give me a short plan and the acceptance criteria you'll
  hit, then implement it. At the end, tell me how to build and verify it on both Windows
  and Linux before moving on.
- Keep a `CLAUDE.md` at the repo root with build/run commands, architecture notes, code
  style, and constraints. Update it as the project grows. Keep it under ~200 lines.
- Write unit tests for the simulation core (integrator, mixer, PID). The flight model is
  the heart of the project — it must be testable without the renderer.
- Ask me before adding any heavy third-party dependency. Prefer the engine's built-ins and
  small, well-maintained libraries.
- Commit at the end of each milestone with a clear message.

Start with **M0** now: confirm the plan, then scaffold the repo.

---

## 1. Project overview

A cross-platform (Windows + Linux desktop) FPV quadcopter simulator used to **train a
school drone team**. It is open source. The benchmark for "good" is VelociDrone-level
physics fidelity and low input latency — reached iteratively, not in v1. V1's job is to
get the **architecture** right so fidelity becomes a tuning problem, not a rewrite.

**Primary goals**
- Faithful acro flight feel via a full flight-controller PID loop and Betaflight-style rates.
- Low, configurable input-to-photon latency.
- A clean, extensible force/torque model for aerodynamics.
- Free-fly + time-trial gate courses on procedurally generated maps.
- Full tuning exposed to the user.

## 2. Tech stack & key decisions

- **Engine:** Godot 4.x (latest stable), MIT-licensed, Windows + Linux.
- **Simulation core:** C++ as a Godot **GDExtension** via `godot-cpp`. The integrator,
  motor model, mixer, flight-controller/PID loop, and aerodynamic forces all live in C++,
  fully decoupled from rendering and engine physics. **Do not use Godot's built-in rigid
  body / Jolt for the drone** — we integrate our own state.
- **UI / glue / scene:** GDScript (menus, HUD, tuning panels, level assembly).
- **Build:** CMake or SCons for the GDExtension (use the `godot-cpp` standard), reproducible
  on Windows (MSVC) and Linux (GCC/Clang). Provide a one-command build doc per platform.
- **Alternative (note only):** If I later choose Rust+Bevy instead, the same architecture
  (custom fixed-step integrator, decoupled render, data-driven drone params) applies — keep
  the sim core engine-agnostic where reasonable so this stays possible.

## 3. Non-goals for v1 (explicitly out of scope)

- No GPS / position-hold / RTH modes.
- No multiplayer or networking.
- No web/browser build.
- No mobile.
- No real radio-link simulation (we read the TX as a USB controller, see §7).

## 4. Core architecture: the loop

- **Decouple physics from rendering.** Run a **fixed-step** physics/FC loop with an
  accumulator; render frames interpolate between the last two physics states for smooth
  visuals independent of frame rate.
- **Parameterizable tick rate.** Expose the physics/FC step as a setting (e.g. start at
  500 Hz; allow 250–2000 Hz). The flight controller loop rate may be the physics rate or an
  integer multiple — make this explicit and configurable.
- **Latency-conscious present path** (see §8): late input sampling, minimal buffering,
  configurable v-sync and fps cap.
- Sim core exposes a clean state struct (position, orientation quaternion, linear & angular
  velocity) and steps deterministically given inputs + dt.

## 5. Flight model (the accuracy core)

Implement a 6-DOF rigid-body quad, built from composable pieces so each can be improved later.

**Rigid body**
- State: position, orientation (quaternion), linear velocity, body angular velocity.
- Configurable mass and inertia tensor. Stable integrator (RK4 or semi-implicit Euler —
  pick one, justify, make it swappable).

**Motors & mixer**
- Quad-X layout. Four motors, each with a position and spin direction.
- Each motor: commanded value → RPM with a first-order time constant (motor + ESC lag).
  Thrust ≈ k·RPM²; reaction yaw torque ∝ thrust/RPM. Design a hook for battery voltage sag
  reducing max RPM under load (stub for v1, wired for later).
- Standard quad-X mixer: throttle + roll/pitch/yaw demands → per-motor commands, with
  motor saturation/clipping and air-mode-style handling.

**Flight controller (full PID loop simulation)**
- Gyro reads body angular rates from the rigid body each FC step.
- **Setpoint generation from sticks** via configurable rate curves — implement
  **Betaflight / Actual rates** (RC rate, super rate, expo) so feel matches real quads.
- **Modes:** Acro (rate), Angle (self-leveling outer P loop → inner rate loop), Horizon
  (blend of the two). No GPS modes.
- **Inner loop:** per-axis PID on (setpoint rate − gyro rate) → mixer demands. Include
  P, I (with anti-windup / iterm-relax hook), D (with D-term lowpass filter).
- **Filters:** gyro lowpass and D-term lowpass — model them, since they shape feel and
  prop-wash handling.
- Include the essentials and leave clean extension points for TPA, throttle curve, etc.

**Aerodynamics (start simple, extensible)**
- A pluggable list of force/torque contributors so new effects drop in cleanly:
  - **Drag:** linear + quadratic body drag (v1).
  - **Ground effect:** thrust efficiency rises near ground as a function of height / rotor
    radius (v1).
  - **Prop wash:** disturbance torque/force when descending through the craft's own
    downwash (v1, simple model; flag for refinement).
  - (Later: turbulence, wind, blade-flapping, etc. — just new contributors.)

## 6. Drone classes / presets

Drone parameters are **data**, not code (e.g. a resource/JSON the tuning UI edits). Ship two presets:
- **5-inch freestyle/race** (~600–700 g, typical 5" prop/motor characteristics).
- **Tinywhoop** (~25–35 g, 65–75 mm, much lower inertia and authority).
Make adding a new class a matter of new data, not new code.

## 7. Input (RadioMaster Pocket)

- The Pocket connects over USB running EdgeTX in **USB Joystick (HID)** mode; the sim reads
  it as a standard game controller — **no radio-link simulation**.
- Read raw axes/buttons; provide a **channel-mapping UI** (assign roll/pitch/yaw/throttle +
  arm + mode switches) and **endpoint calibration** (min/center/max per axis) with a
  deadband setting. Persist the mapping.
- Sample input at high rate and as late as possible before each FC/physics step (see §8).
- Also support a generic USB gamepad fallback and keyboard for testing before hardware.

## 8. Latency requirements

- Make **v-sync mode, fps cap, and physics tick rate** user-configurable settings.
- Sample controller input immediately before the physics step (minimize stale-input delay).
- Avoid deep render queues / extra buffering; keep present path lean so higher-refresh
  monitors automatically reduce motion-to-photon latency with no code changes.
- Target 60 fps on a mid laptop, but the architecture must not assume 60 Hz — it should fly
  correctly and feel better at 120/144 Hz.
- Add an on-screen debug overlay showing physics rate, render fps, and an input→render
  timing estimate so latency is measurable, not guessed.

## 9. Game modes & maps (v1)

- **Free fly:** open procedurally generated environment.
- **Time trials:** a procedurally generated **gate course** (seeded for reproducibility) —
  ordered gates/checkpoints, start/finish, lap & split timing, best-time tracking, restart.
- Simple OSD/HUD: timer, speed, armed state, current mode, battery (cosmetic for now).

## 10. Tuning UI (full exposure)

A GDScript panel (toggle in-flight) to live-edit and save: rates (RC/super/expo per axis),
PID gains per axis, gyro & D-term filter cutoffs, physics tick rate, drone preset selection
and core drone params (mass, inertia, motor constants). Changes apply live; profiles
save/load to disk.

## 11. Assets

- Source **free, permissively licensed (CC0 preferred)** 3D assets and textures from the
  web for the drone model, gates, and environment props (e.g. Kenney, Poly Haven, CC0
  sources). Use simple placeholders first; record asset sources and licenses in an
  `ASSETS.md` / credits file. Don't block flight-model work on art.

## 12. Repo, build & quality

- Cross-platform build documented per OS; verify on both Windows and Linux each milestone.
- Add CI later (build the GDExtension on Win + Linux) — not required for M0.
- Sim core unit-tested independently of the engine.
- `CLAUDE.md` kept current: build/run commands, architecture, code style, constraints.
- License the repo (MIT or GPL — ask me which) and add a README explaining the project's
  training purpose.

## 13. Milestone roadmap

**M0 — Scaffold.** Repo, license, README, `CLAUDE.md`. Godot 4 project + `godot-cpp`
GDExtension that loads and prints from C++. Builds on Windows + Linux. Acceptance: blank
Godot window with the C++ extension confirmed loaded.

**M1 — Rigid body + motors + mixer.** 6-DOF integrator, motor model (RPM lag, thrust,
reaction torque), quad-X mixer. Drive throttle directly (no FC yet). Render a placeholder
drone with **interpolation** between physics states. Unit tests for integrator & mixer.
Acceptance: drone responds to direct motor input; physics decoupled from fps.

**M2 — Flight controller (Acro).** Gyro, per-axis rate PID with filters, Betaflight rates
for setpoints, arm/disarm. Fly with keyboard/gamepad. Tests for PID & rate curves.
Acceptance: controllable acro flight that feels rate-based.

**M3 — Input (RadioMaster Pocket).** Read TX as USB joystick, channel-mapping UI, endpoint
calibration + deadband, persisted. Acceptance: fly with the Pocket; mapping survives restart.

**M4 — Angle & Horizon + tuning UI.** Self-leveling modes; live tuning panel for rates, PID,
filters; drone presets (5" + tinywhoop) as data. Acceptance: switch modes in flight; edit
and save tunes live.

**M5 — Aerodynamics + configurable tick.** Pluggable drag, ground effect, prop wash;
expose physics tick rate setting. Acceptance: visible ground effect & prop-wash behavior;
tick rate adjustable.

**M6 — Maps & modes.** Procedural free-fly environment; seeded procedural gate course;
time-trial timing (laps/splits/best), restart; OSD/HUD. Acceptance: complete a timed run.

**M7 — Latency hygiene.** Configurable v-sync/fps cap, late input sampling, lean present
path; timing/latency debug overlay; settings persistence. Acceptance: settings change
measurably affect the overlay's latency estimate.

**M8 — Polish.** Motor/wind audio, menus, swap placeholders for sourced CC0 assets, credits
file, packaged Windows + Linux builds. Acceptance: a student can download, plug in a Pocket,
and fly a timed course on either OS.
