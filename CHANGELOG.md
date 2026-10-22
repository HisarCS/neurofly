# Changelog

All notable changes to NeuroFly are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project uses
[Semantic Versioning](https://semver.org/). The version lives in the `VERSION` file.

## [0.2.0] - 2026-10-22

### Added
- `LICENSE` file (MIT, copyright Hisar IdeaLab FabLab); the README previously claimed MIT without one.
- `VERSION` file and a documented release procedure (README, "Versioning & Releases").
- Protocol v2 (`firmware/common/protocol.h`): XOR checksum on control and telemetry packets, protocol version in both packets, telemetry flags, and a per-classroom ESP-NOW pairing key (PMK/LMK) for encrypted, MAC-paired links.
- Firmware upgrade (see `firmware/`): drone gets MPU6050 PID stabilisation, ToF altitude hold, battery monitoring and an emergency cut; controller gets analog gimbals or buttons, roll and yaw, OLED UI, mode switch, NVS-stored pairing, and serial JSON (`armed, thr, r, p, y, mode, bat, rssi, ver`) with inbound `{"vr","vp"}` corrections for AI_ASSIST. Not flight-verified without hardware.
- `.github/workflows/firmware-ci.yml`: PlatformIO builds for the `drone` and `controller` environments on pushes/PRs touching `firmware/`.
- `scripts/sync_web.sh` keeps `webapp/public/` and the root `index.html` as synced copies of `docs/`; the Pages workflow checks it.
- `docs/ai_autonomous/aruco_tracker.py`: `--port`/`--baud` pyserial output of `{"vr","vp"}` JSON lines at about 20 Hz (zeros when the target is lost and on exit), `--camera` index, pixel deadband, low-pass filter and D term.
- Assembly guide: classroom key change, pairing procedure, pre-flight safety checklist (props off for bench test, failsafe test, emergency cut test, arm only at minimum throttle).
- `CHANGELOG.md`.

### Changed
- `.github/workflows/pages.yml` now uploads only `./docs` (matching the live site, branch `main` + `/docs`) instead of the whole repository root, which published `firmware/` and `hardware/`. Added path filters and the sync check.
- README: corrected repository layout (docs sub-folders, scripts, VERSION, CHANGELOG), quick start now serves `docs/` (the live site) instead of `webapp/public`, live site link, versioning section, honest status table.
- `hardware/schematics/PINOUT.md` reconciled with firmware: controller map is authoritative (D0 throttle, D1 yaw, D2 pitch, D3 roll, D8 arm, D9 mode, D4/D5 OLED), analog-gimbal (3V3 reference) and button (pull-up) wiring documented, 3.3 V gate-drive note, battery divider math.
- `RULES.md`: documents emergency cut, arming rule, pairing/encryption, 200 ms failsafe and protocol version; scoring unchanged.
- Teacher and student guides aligned with firmware behaviour (XOR checksum instead of "CRC", pairing/key troubleshooting, simulator path, honest status).
- BOM and kit-box guide: counts reconciled (4 gate resistors, 4 pull-downs, 2 divider resistors, 10 uF capacitor, optional ToF, gimbal/button variants).
- KiCad READMEs made consistent and now state honestly that the projects are skeletons.
- `.gitignore`: PlatformIO, Python caches, editor and KiCad backup files.

### Fixed
- XIAO ESP32-C6 pin table: D8/D9/D10 corrected to GPIO19/GPIO20/GPIO18 (PINOUT.md and the KiCad XIAO symbol).
- ADC pin conflicts resolved: drone battery sense moved to D2 (GPIO2), rear-right motor to D6; controller gimbal mode reads its four axes through an external ADS1115 (I2C).
- Build fixed: `platformio.ini` now sets `src_dir` and source filters, so both firmware envs link (previously `setup()/loop()` were undefined).
- Pairing redesigned: controller MAC persisted on the drone, unencrypted pair-ack only while unpaired, then encrypted unicast on both sides (compile-verified only, not bench-tested).
- Known: flyback-diode footprint (SOD-123) does not match SS14 (SMA) / 1N5819 (DO-41).

## [0.1.0] - baseline

### Added
- Initial public release: simulator and landing pages, drone and controller firmware (protocol v1), PlatformIO config, KiCad project stubs, laser-cut chassis DXF/SVG, 3D-printable STL models, assembly guide, BOM, competition rules, curriculum, kit-box guide and ArUco vision tracker.
