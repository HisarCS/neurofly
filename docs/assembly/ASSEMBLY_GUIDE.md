# NeuroFly Step-by-Step Assembly Manual 🛠️
*A Visual Hardware Assembly Guide for Students & Makers*

---

## 🧰 Required Tools & Consumables
- M2 hex screwdriver / tweezers
- Temperature-controlled soldering iron ($330^\circ\text{C}$) & lead-free solder
- Multimeter with continuity beep mode
- 4× M2 × 8mm nylon screws, nuts, and 4mm standoffs
- High-adhesion foam tape (for IMU vibration isolation)
- 2× Small zip-ties (100mm × 2.5mm)

---

## 🧩 Step 1: Airframe & Prop Guard Integration

```text
       [Prop Duct FL]            [Prop Duct FR]
              \                        /
               \                      /
                ┌────────────────────┐
                │  Laser-Cut Chassis │
                │  (85mm Wheelbase)  │
                └────────────────────┘
               /                      \
              /                        \
       [Prop Duct RL]            [Prop Duct RR]
```

1. Take the laser-cut Baltic birch or acrylic spine (`neurofly_chassis_frame.dxf`).
2. Slide the four 3D-printed TPU prop ducts (`neurofly_prop_duct_40mm.stl`) over the chassis arm ends.
3. Press the 8520 coreless motors gently into the motor collars:
   - **Front-Left & Rear-Right:** Install **CW motors** (Red [+] / Blue [-] wires).
   - **Front-Right & Rear-Left:** Install **CCW motors** (Black [-] / White [+] wires).
4. Route motor wires through the chassis arm wire guide slots towards the center.

---

## ⚡ Step 2: PCB Mounting & Wiring

1. Mount the NeuroFly Flight Controller Carrier PCB onto the central 30.5mm × 30.5mm holes using M2 nylon standoffs.
2. Trim motor wires to length and solder them to the corresponding corner solder pads:
   - `M1_FL` (Front-Left)
   - `M2_FR` (Front-Right)
   - `M3_RR` (Rear-Right)
   - `M4_RL` (Rear-Left)
3. Solder the JST-PH 2.0 or BT2.0 battery lead to the center battery pads. **Double-check polarity! Red is VCC (+3.7V), Black is GND.**

---

## 🛡️ Step 3: Sensor Decoupling & Battery Tray

1. Apply a 10mm × 10mm square of vibration-dampening foam tape beneath the MPU6050 IMU breakout.
2. Press the IMU gently onto the carrier board header. Ensure the IMU X-axis arrow points toward the front of the drone.
3. Attach the 3D-printed battery cradle (`neurofly_battery_tray.stl`) to the bottom of the chassis using two M2 nylon screws.
4. Slide your 1S LiPo (300mAh - 450mAh) into the cradle and secure it with a small zip-tie.

---

## 🔌 Step 4: Flash Firmware & Set the Classroom Key

Firmware version 0.2.0, protocol v2 (see `firmware/common/protocol.h`).

1. **Install PlatformIO** (VS Code extension or `pip install platformio`).
2. **Change the per-classroom key BEFORE flashing.** Open `firmware/common/protocol.h` and edit the two 16-character strings:
   ```c
   #define NEUROFLY_PMK "NeuroFlyPMK-0001"   // exactly 16 chars
   #define NEUROFLY_LMK "NeuroFlyLMK-0001"   // exactly 16 chars
   ```
   Pick a different pair for every classroom or team (e.g. `NeuroFlyPMK-B7K2`). Drone and controller **must be flashed with the identical pair**, otherwise encrypted ESP-NOW frames are not accepted and the drone will never receive commands. Do not commit your private keys to a public repository.
3. Flash from the `firmware/` folder, with the board plugged into USB-C:
   ```bash
   cd firmware
   pio run -e drone      -t upload     # drone board
   pio run -e controller -t upload     # controller board
   ```
   (Hold BOOT while plugging in if the board is not detected.)

### Pairing procedure (controller <-> drone)

Each drone remembers exactly one controller (its MAC is stored in the drone's non-volatile memory) and each controller remembers exactly one drone. After pairing, all traffic is encrypted with the key above and frames from any other MAC are ignored. Armed/flight frames are never accepted unencrypted or from an unpaired MAC.

1. Props **off**. A drone with nothing stored is in the **pairing state**: the LED double-flashes and the motors cannot run.
2. Power the drone, then on the **controller** hold the **MODE** button while powering it on (USB-C or battery). The controller broadcasts unencrypted, DISARMED pairing frames. The drone accepts only such a frame, answers with a pair-ack (telemetry with the pairing flag), stores the controller MAC and switches to an encrypted link. The controller stores the drone MAC from that reply and also switches to encrypted unicast.
3. Wait until the controller OLED shows the link up (RSSI and drone battery voltage appear) and the telemetry `ver` matches. On later power-ups both sides reconnect automatically (the drone registers its stored controller at boot).
4. **Clear pairing / re-pair:** on the drone, wait until it has started (LED blinking), then hold the **BOOT** button (GPIO9) for 3 s within the first 5 s; it clears its memory and restarts in pairing state. (Do not hold BOOT while powering on: that enters the flashing bootloader.) On the controller, hold MODE at power-up again. If only one side was cleared the link will not come up; clear/re-pair both. Pair one team at a time, because an unpaired drone accepts the first valid pairing request it hears (use a different key per team where possible).

> Status: **compile-verified only.** The pairing flow has not been bench-tested on hardware (none was available). Confirm each step on the bench and update this section if your build behaves differently.

---

## 🛡️ Step 5: Pre-Flight Safety Checklist (every session)

**Bench test (propellers OFF):**
- [ ] Multimeter between LiPo (+) and (-) shows **NO short circuit** (before first power-up and after any rewiring).
- [ ] **Propellers are NOT installed.** Motors are tested on the bench with props off only.
- [ ] Connect USB-C to the XIAO ESP32-C6 and verify the power LED illuminates.
- [ ] Disconnect USB-C, insert the 1S battery (fully charged, no puffing) and verify the status LED shows ready.
- [ ] Controller: throttle at minimum, drone **disarmed**. Pair/link is up (OLED shows RSSI and battery).
- [ ] **Arm only at minimum throttle.** Arming with throttle up must be refused by the firmware; verify this once.
- [ ] Raise throttle slightly: all four motors spin, with correct directions (FL CW, FR CCW, RR CW, RL CCW).
- [ ] Tilt the drone by hand: the motors on the low side speed up (IMU axes and mixer correct).
- [ ] **Failsafe test:** with motors spinning at low throttle, switch the controller off (or unplug it). Within about 200 ms all motors must stop. Repeat after any firmware change.
- [ ] **Emergency cut test:** hold the **ARM** button for 1 second or longer on the controller; motors must stop immediately and the drone stays locked out until a DISARMED state is seen, then re-arm at minimum throttle.
- [ ] Low-battery: confirm the OLED shows the drone battery voltage and the low-battery flag appears when you power the drone from a deliberately low (about 3.4 V) supply or a benchtop supply.

**Only after the bench test passes:**
- [ ] Push the propellers onto the shafts: verify CW props on CW motors, CCW props on CCW motors (Orange / White props on the front, Black on the rear).
- [ ] Prop guards/ducts installed on all four motors, safety glasses on everyone in the flight area.
- [ ] Clear the area, arm at minimum throttle, lift off gently. Keep a thumb on the ARM button (hold 1 s = emergency cut).

> Status: this checklist describes intended behaviour of firmware 0.2.0. The firmware has not been flight-verified without hardware; treat the first flights as test flights in a netted area with a hand on the cut control.
