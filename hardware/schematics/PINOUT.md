# NeuroFly Hardware & Pinout Specifications

This document defines the pinout mapping, peripheral assignments, and circuit design guidelines for the Seeed Studio XIAO ESP32-C6 on both the **NeuroFly Drone** and the **NeuroFly Remote Controller**. Firmware version 0.2.0 (protocol v2).

> Source of truth: the **controller map in section 3 is authoritative**; firmware `#define`s must match it. If firmware and this file disagree, fix the firmware or open an issue, do not silently change one side.

---

## 1. Seeed Studio XIAO ESP32-C6 Pin Mapping

| Pin Label | ESP32-C6 GPIO | Notes |
| :--- | :--- | :--- |
| **D0** | GPIO0 | ADC1_CH0 (analog-capable) |
| **D1** | GPIO1 | ADC1_CH1 (analog-capable) |
| **D2** | GPIO2 | ADC1_CH2 (analog-capable) |
| **D3** | GPIO21 | Digital only (no ADC) |
| **D4** | GPIO22 | Default I2C SDA |
| **D5** | GPIO23 | Default I2C SCL |
| **D6** | GPIO16 | Default UART TX, digital only (no ADC) |
| **D7** | GPIO17 | Default UART RX, digital only (no ADC) |
| **D8** | GPIO19 | Default SPI SCK |
| **D9** | GPIO20 | Default SPI MISO |
| **D10** | GPIO18 | Default SPI MOSI |

**Correction in v0.2.0:** earlier revisions of this file (and the KiCad symbol) listed D8=GPIO18, D9=GPIO19, D10=GPIO20. To the best of our knowledge of the Seeed XIAO ESP32-C6 wiki the correct mapping is D8=GPIO19, D9=GPIO20, D10=GPIO18 (as above). Firmware uses the `Dx` names, so it is unaffected, but **please verify against the current Seeed pinout diagram before ordering PCBs**.

**Things we are confident about / not confident about**
- Confident: D0-D2 are the only exposed ADC pins; the ESP32-C6 ADC only exists on GPIO0-GPIO6, so D3 (GPIO21), D6 (GPIO16) and D7 (GPIO17) **cannot do `analogRead`**.
- Confident: ESP32-C6 ADC input range is roughly 0 to ~3.1 V with 11 dB attenuation and is not very linear near the ends; never exceed 3.3 V on any pin (GPIOs are not 5 V tolerant).
- Not verified: whether the extra back-side pads of the XIAO ESP32-C6 expose further ADC-capable GPIOs; we do not rely on them.
- Not verified here: the exact on-board battery-charger / regulator behaviour of the XIAO ESP32-C6 for powering from a 1S LiPo through the 5V/BAT pads. Check the Seeed wiki before finalising the drone power path.

**BOOT button** = GPIO9 (used by the drone: hold 3 s within 5 s after power-up to clear pairing; never hold it while powering on, that enters the ROM bootloader).

### Pin-map conflicts: resolved in firmware 0.2.0
- Drone battery sense moved to **D2 (GPIO2, ADC1_CH2)** and the Rear-Right motor PWM moved to **D6 (GPIO16)** (digital only is fine for LEDC PWM).
- Controller analog-gimbal variant reads its four axes through an external **ADS1115** on the shared I2C bus (see 3a); the native pins D0-D3 are only used by the button variant.

---

## 2. NeuroFly Drone (Flight Controller Carrier Board)

The XIAO ESP32-C6 mounts onto a custom KiCad carrier PCB via female headers or castellated pads. Pins below match the `#define`s in `firmware/drone/main.cpp` (motors D0, D1, D6, D3; battery sense D2; SDA/SCL D4/D5; LED D7). The firmware does not use an XSHUT pin; D8 is reserved for it.

| Function | XIAO Pin | ESP32-C6 GPIO | Description |
| :--- | :--- | :--- | :--- |
| **Motor 1 (Front Left - CW)** | D0 | GPIO0 | PWM (20 kHz, 10-bit) -> N-FET gate (AO3400) |
| **Motor 2 (Front Right - CCW)** | D1 | GPIO1 | PWM -> N-FET gate |
| **Motor 3 (Rear Right - CW)** | D6 | GPIO16 | PWM -> N-FET gate (moved from D2 so D2 can be the ADC input) |
| **Motor 4 (Rear Left - CCW)** | D3 | GPIO21 | PWM -> N-FET gate |
| **I2C SDA** | D4 | GPIO22 | Shared bus: MPU6050 (0x68/0x69) and VL53L0X (0x29) |
| **I2C SCL** | D5 | GPIO23 | Shared bus |
| **Battery Voltage Monitor** | D2 | GPIO2 | 2x 10k divider -> ADC1_CH2 (ADC exists only on GPIO0-6) |
| **Status LED / Buzzer** | D7 | GPIO17 | Status indicator / piezo buzzer |
| **ToF XSHUT (Optional)** | D8 | GPIO19 | VL53L0X shutdown (optional) |

### Motor Drive Topology (Per Motor Channel)
```text
                  +3.7V (1S LiPo)
                       |
                     +-+---------+
                     |           |
                 [Motor]    [Schottky 1N5819 / SS14]
                     |      (cathode to +VBAT)
                     |           |
                     +-+---------+
                       |
                    Drain (D)
GPIO PWM --[100R]---Gate (G) [AO3400A N-MOSFET]
                       |   \__ 10k pull-down Gate->GND
                  Source (S)
                       |
                      GND
```

Design notes
- **3.3 V gate drive:** AO3400A has Vgs(th) around 0.65-1.45 V and a specified Rds(on) at Vgs = 2.5 V, so a 3.3 V GPIO drives it fully on; at 8520 currents (about 1-2 A peak) dissipation is only tens of milliwatts. The 100 ohm series resistor limits gate-charge current and ringing; the 10 kohm pull-down keeps the motor off while the MCU boots.
- **Flyback diode orientation:** cathode to +VBAT, anode to the drain/motor-minus node. The KiCad symbol footprint is `D_SOD-123`; SS14 is an SMA part and 1N5819 is DO-41 (the SOD-123 variant is 1N5819HW). Choose one and set the footprint accordingly before ordering.
- **Battery divider math:** 2x 10 kohm gives Vadc = Vbat / 2. A full 4.2 V cell reads 2.1 V and a 3.0 V cell reads 1.5 V, both inside the ~0-3.1 V ADC range. The divider draws 4.2 V / 20 kohm = 210 uA continuously (about 0.6 mAh/day, negligible while flying but it will slowly drain a stored pack: unplug the battery when not in use). Use `battery_mv = adc_mv * 2` and calibrate against a multimeter; ESP32 ADCs are only accurate to several percent uncalibrated. A 100 nF capacitor from the ADC node to GND steadies the reading.
- **Decoupling:** 10 uF low-ESR ceramic close to the battery pads (the soldering sequence calls for it).
- **Brownout:** a 1S pack sags under motor load; the XIAO's 3.3 V regulator needs headroom. Flying below about 3.4 V pack voltage risks brown-outs, which is why the firmware low-battery flag exists.

---

## 3. NeuroFly Remote Controller (Transmitter Board) - AUTHORITATIVE

The controller is an ESP-NOW transmitter and also a USB-C serial device for the browser simulator (Web Serial) and for the AI tracker.

| Peripheral | XIAO Pin | ESP32-C6 GPIO | Description |
| :--- | :--- | :--- | :--- |
| **Throttle** | D0 | GPIO0 | Button variant: throttle up/down. Analog variant: ADS1115 A0 |
| **Yaw** | D1 | GPIO1 | Button variant only. Analog variant: ADS1115 A1 |
| **Pitch** | D2 | GPIO2 | Button variant only. Analog variant: ADS1115 A2 |
| **Roll** | D3 | GPIO21 | Button variant only (roll/pitch- button). Analog roll is ADS1115 A3 |
| **Arm / Disarm** | D8 | GPIO19 | Pushbutton or toggle to GND (internal pull-up) |
| **Flight Mode** | D9 | GPIO20 | Pushbutton to GND (internal pull-up), cycles modes |
| **OLED SDA** | D4 | GPIO22 | SSD1306 128x64, I2C address 0x3C |
| **OLED SCL** | D5 | GPIO23 | SSD1306 I2C clock |
| **ADS1115 (analog variant only)** | D4/D5 | GPIO22/23 | Same I2C bus as the OLED, address 0x48 (ADDR to GND) |

### 3a. Analog-gimbal wiring
```text
 3V3 ----+---------------+            each gimbal axis = 10k pot
         |               |
       [pot end 1]    [pot end 2]
              \          /
               [ wiper ]------> ADS1115 A0 / A1 / A2 / A3
                         \---> GND (pot end 2)

 ADS1115 module: VDD -> 3V3, GND -> GND, SDA -> D4, SCL -> D5, ADDR -> GND (0x48)
 A0 = throttle (left Y), A1 = yaw (left X), A2 = pitch (right Y), A3 = roll (right X)
```
- Power the potentiometers from **3V3 (not 5V/VBUS)**. The ESP32-C6 ADC only reads up to about 3.1-3.3 V; a pot on 5V would exceed the pin limit and damage the GPIO.
- Use 10 kohm thumbstick pots; add a 100 nF capacitor from each wiper to GND for noise.
- Firmware calibrates centre at boot (leave sticks centred and throttle low when powering on).
- Gimbal push-switches are not used. The ADS1115 is **optional hardware, only for the gimbal variant** (`-DINPUT_MODE=1`); if it is not found at boot, sticks read as zero throttle/centred (safe) and an error line is printed on serial.

### 3b. Button wiring (firmware default, `INPUT_MODE=0`)
- Each button connects its pin to **GND**; the firmware enables the internal pull-up (`INPUT_PULLUP`), so no external resistors are needed (optional 10 kohm to 3V3 for long wires). Pressed = LOW.
- Only four stick buttons exist, so the **MODE button is a shift key** (per firmware): D0 = throttle up / yaw right (with MODE held), D1 = throttle down / yaw left, D2 = pitch + / roll right, D3 = pitch - / roll left. A short press-release of MODE (without using shift) cycles the flight mode.
- Arm: short press of the D8 button toggles arm/disarm (refused unless throttle is at minimum). **Holding ARM for 1 s or more = emergency cut.**
- Analog gimbals are selected at build time with `-DINPUT_MODE=1` in `firmware/platformio.ini` (they use the ADS1115, not D0-D3); build the board for one variant (the KiCad files implement neither yet).
- D8/D9 are GPIO19/GPIO20; the `Dx` names are what the firmware uses.

### 3c. Serial JSON (USB-C CDC, 115200 baud, one object per line)
Outbound (controller to PC, used by the simulator): `{"armed","thr","r","p","y","mode","bat","rssi","ver"}`.
Inbound (PC to controller, used by `docs/ai_autonomous/aruco_tracker.py`): `{"vr":roll,"vp":pitch}` corrections, only applied in `AI_ASSIST` mode.

---

## 4. KiCad Fabrication Guidelines (FabLab & Fab Academy Standards)

1. **Dual Fabrication Compatibility:**
   - **Milled PCBs (FabLab Roland SRM-20 / Modela):**
     - Single-sided or double-sided routing with top ground plane.
     - Minimum trace width: 0.40 mm (16 mil).
     - Minimum clearance: 0.40 mm (16 mil).
   - **Commercial Fabrication (JLCPCB / PCBWay):**
     - Standard 1.2mm or 0.8mm FR4 thickness to save weight.
     - 0805 or 1206 passive footprint sizing for easy hand-soldering by students.
2. **Weight Optimization:**
   - Structural carrier board includes internal cutouts (honeycomb/cross lattice) to reduce mass while preserving stiffness.
