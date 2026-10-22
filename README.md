# NeuroFly 🚀

**NeuroFly** is an open-source, modular, indoor micro-drone and smart remote controller platform developed by **Hisar IdeaLab FabLab (MIT Edgerton Center Hub)**.

The platform bridges digital fabrication (laser cutting & 3D printing), embedded systems engineering (Seeed Studio XIAO ESP32-C6), ultra-low-latency wireless telemetry (ESP-NOW), browser-based 3D simulation (Web Serial API), and multi-tier student drone challenges (manual, autonomous, and computer vision / AI).

---

## 🎯 Core Pillars & Vision

1. **Digital Fabrication First:**
   - **Chassis / Airframe:** Parametric laser-cut designs (Acrylic, Baltic Birch Plywood, or Delrin).
   - **Protection & Mounting:** 3D-printed lightweight components (PLA / flexible TPU propeller ducts).
2. **Safe Indoor Architecture:**
   - Powered by 1S LiPo batteries and lightweight coreless DC motors (safer and more accessible than high-KV brushless motors for indoor classrooms).
   - Full prop-guard ducting ensures safe flight in laboratory and classroom environments.
3. **Smart Bi-Directional Transmitter:**
   - Powered by Seeed Studio XIAO ESP32-C6.
   - Onboard I2C OLED display (SSD1306) for real-time telemetry (RSSI, battery, flight modes).
   - Low-latency, paired and encrypted ESP-NOW RF link (protocol v2: XOR checksum, versioned packets) for direct drone control.
   - Native USB-C Web Serial integration for instant browser simulator training without installing native drivers.
4. **Interactive 3D Web Simulator:**
   - Built with WebGL / Three.js.
   - Allows students to connect their physical controller and practice flying, test PID tunings, and simulate race gates virtually before maiden flight.
5. **Multi-Disciplinary Competition Matrix:**
   - **Weight Efficiency:** Rewarding the lightest structural designs.
   - **Agility & Endurance:** Lap racing through FabLab gate courses.
   - **Autonomous & AI:** Altitude-hold, ToF laser obstacle avoidance, and visual target tracking.

---

## 📁 Repository Layout

```text
neurofly/
├── VERSION                 # single source of truth for the release version (semver)
├── CHANGELOG.md            # Keep-a-Changelog release notes
├── LICENSE                 # MIT, Hisar IdeaLab FabLab
├── firmware/               # PlatformIO project (envs: drone, controller)
│   ├── drone/              # Flight controller firmware (IMU PID, ToF alt-hold, battery, failsafe, emergency cut)
│   ├── controller/         # Transmitter firmware (gimbals or buttons, OLED, pairing, serial JSON)
│   └── common/             # Shared protocol header (protocol.h: packets, checksum, keys, versions)
├── hardware/
│   ├── kicad/              # KiCad project skeletons (drone_board, controller_board), NOT finished/routed, see their READMEs
│   ├── cad/                # 3D printable models (STL: prop ducts, case, battery tray)
│   ├── laser/              # Laser-cutting drawings (DXF/SVG chassis frame)
│   └── schematics/         # PINOUT.md: pin maps and wiring (the hardware reference)
├── docs/                   # GitHub Pages root: this folder IS the live website
│   ├── index.html          # Landing page / interactive explorer
│   ├── sim/                # Browser flight simulator (Web Serial)
│   ├── assembly/           # Assembly guide, pairing, pre-flight checklist
│   ├── bom/                # Bill of Materials
│   ├── competition/        # Rules, safety regulations, scoring
│   ├── curriculum/         # Teacher guide, student workshop guide, arena spec
│   ├── kit_box/            # Kit unboxing / box prep guide
│   └── ai_autonomous/      # ArUco vision tracker (aruco_tracker.py) and AI track notes
├── webapp/public/          # Synced copy of the site (kept identical by scripts/sync_web.sh)
├── index.html              # Synced copy of the root page (same script)
├── scripts/                # sync_web.sh: keeps webapp/public and root index.html in sync with docs/
└── .github/workflows/      # pages.yml (deploy docs/), firmware-ci.yml (PlatformIO builds)
```

`docs/` is the source of truth for the website. `webapp/public/` and the root `index.html` are generated copies: edit `docs/`, then run `scripts/sync_web.sh` (and `scripts/sync_web.sh --check` to verify; CI runs the check).

---

## 🛠️ Hardware Specifications

- **Main MCU:** [Seeed Studio XIAO ESP32-C6](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/) (High-performance 32-bit RISC-V, Wi-Fi 6, BLE 5.3, Zigbee/Thread 802.15.4, ESP-NOW)
- **Drone Inertial Unit (IMU):** 6-Axis Gyroscope & Accelerometer (MPU6050 or LSM6DS3 via I2C)
- **Ranging / Obstacle Sensor:** VL53L0X / VL53L1X Time-of-Flight (ToF) distance sensor
- **Motors:** 4x Coreless Brushed DC Motors (7x16mm 716 or 8.5x20mm 8520)
- **Motor Drivers:** 4x Logic-Level N-Channel MOSFETs (AO3400 / SI2302) with Flyback Schottky Diodes (1N5819 / SS14)
- **Propellers:** 31mm - 55mm Micro Propellers (2x CW, 2x CCW)
- **Controller Interface:** 0.96" I2C OLED (SSD1306, 128x64) + Directional Push Buttons / Mini Analog Thumbsticks
- **Power:** 1S 3.7V 300mAh - 450mAh LiPo Battery (25C-30C discharge)

---

## 🚀 Quick Start

**Live site:** <https://hisarcs.github.io/neurofly/> (simulator at `/sim/`).

Run it locally (the live site is the `docs/` folder):

```bash
cd docs
python3 -m http.server 8000
```
Open `http://localhost:8000/sim/` in Google Chrome or Microsoft Edge (Web Serial is not available in Firefox/Safari). Plug in the XIAO ESP32-C6 controller via USB-C and click **Connect Web Serial**.

Build and flash the firmware (PlatformIO):

```bash
cd firmware
pio run -e drone      -t upload
pio run -e controller -t upload
```
Before flashing, change the per-classroom keys in `firmware/common/protocol.h` and follow `docs/assembly/ASSEMBLY_GUIDE.md` (pairing + pre-flight checklist).

---

## ✅ Status (honest)

| Area | State |
| :--- | :--- |
| Web simulator, docs, curriculum, rules | Working; this is what the live site serves |
| Firmware (protocol v2) | CI builds both PlatformIO envs (`drone`, `controller`) on each push (first run pending). **Not flight-verified**: it cannot be tested without hardware. IMU PID tuning, ToF altitude hold, battery monitor, failsafe/emergency cut, pairing and encryption all need bench testing (props off) and then supervised flight testing |
| Pin map | Documented in `hardware/schematics/PINOUT.md`. Drone battery sense is on D2 (ADC) and the Rear-Right motor on D6; the analog-gimbal controller variant uses an external ADS1115 (the ESP32-C6 ADC exists only on GPIO0-6). Pairing (drone stores controller MAC, BOOT 3 s clears) and pins are compile-verified only, not bench-tested |
| KiCad boards | Skeleton projects only: outlines and symbol libraries, no placed parts, no routing, no DRC. Do not send to a fab yet |
| CAD / laser files | Provided; print/cut a test piece before a batch |
| `aruco_tracker.py` | Implemented, with optional serial output to the controller; untested against real hardware |

---

## 🏷️ Versioning & Releases

- **Single source of truth:** the `VERSION` file at the repo root (semantic versioning, `MAJOR.MINOR.PATCH`). Git tags are `vX.Y.Z` (e.g. `v0.2.0`).
- **Protocol version vs firmware version:** `NEUROFLY_PROTOCOL_VERSION` (an integer in `firmware/common/protocol.h`, currently 2) changes only when the over-the-air packet format or semantics change, and controller and drone must match. `NEUROFLY_FIRMWARE_VERSION` (a string in the same header) follows `VERSION`. The simulator compares the firmware version reported by the controller (`ver`) with its own `NEUROFLY_VERSION` and warns on mismatch.
- **Cutting a release:**
  1. Bump `VERSION`.
  2. Update the `NEUROFLY_VERSION` JS constant in `docs/sim/index.html` (and any other site page that has one), then run `scripts/sync_web.sh` so `webapp/public/` and the root `index.html` follow.
  3. Update `NEUROFLY_FIRMWARE_VERSION` in `firmware/common/protocol.h` (and bump `NEUROFLY_PROTOCOL_VERSION` only if the wire format changed).
  4. Add the release to `CHANGELOG.md`.
  5. Commit, then `git tag vX.Y.Z` and push the tag.

---

## 🌐 Deployment

GitHub Pages serves branch `main`, folder `/docs`. The workflow `.github/workflows/pages.yml` uploads only `./docs` (never `firmware/` or `hardware/`) and fails if `scripts/sync_web.sh --check` reports out-of-sync copies. Note: using this workflow requires the repository's Pages source to be set to "GitHub Actions"; with the "branch + /docs" source the workflow is redundant and can be disabled. `.github/workflows/firmware-ci.yml` builds both PlatformIO environments on every push/PR touching `firmware/`.

---

## 📜 License
Open source under the [MIT License](LICENSE), copyright Hisar IdeaLab FabLab.
