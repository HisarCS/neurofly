# NeuroFly Student Workshop & Engineering Handbook 🎓
**A 4-Week Comprehensive Aerial Robotics & Digital Fabrication Journey**  
*Hisar IdeaLab FabLab (MIT Edgerton Center Hub)*

---

## 🧭 Course Overview & Philosophy

Welcome to the **NeuroFly Engineering Program**! In this course, you will not simply assemble a pre-made drone kit—you will engineer an indoor micro-quadcopter and bi-directional smart transmitter from raw digital design files, through CNC PCB fabrication, embedded programming on a 32-bit RISC-V microcontroller (Seeed Studio XIAO ESP32-C6), and flight stabilization in both simulated and real-world environments.

### The 4 Pillars of Mastery:
1. **Digital Fabrication:** Parametric CAD, laser cutting tolerance budgeting, and flexible 3D printing.
2. **Electronics & Circuit Craftsmanship:** KiCad schematics, SMD soldering, MOSFET switching dynamics, and LiPo electrochemical safety.
3. **Embedded Systems & Telemetry:** Low-latency wireless protocols (ESP-NOW), 20kHz motor PWM mixing, and IMU sensor fusion.
4. **Control Theory & Piloting:** Closed-loop PID stabilization, attitude rate dynamics, and autonomous mission execution.

---

## 🗓️ 4-Week Detailed Syllabus

```
WEEK 1: Airframe Dynamics & Digital Fabrication (Laser & 3D Printing)
WEEK 2: PCB Production, Soldering & Electrical Verification
WEEK 3: Embedded Firmware, Telemetry & Web-in-the-Loop Simulation
WEEK 4: Flight Tuning, PID Stabilization & Grand Prix Arena Challenge
```

---

## 🛠️ Week 1: Airframe Dynamics & Digital Fabrication

### 1.1 Quadcopter Physics Fundamentals
A quadcopter achieves 6 degrees of freedom (6-DOF) through differential thrust from four fixed-pitch propellers:

$$\text{Total Thrust } T_{\text{total}} = T_1 + T_2 + T_3 + T_4$$

$$\text{Hover Condition: } T_{\text{total}} = m \cdot g$$

- **Mass Budget Target:** Total All-Up-Weight ($m$) must be $\le 45.0 \text{ grams}$.
- **Gravitational Force ($F_g$):** $0.045\text{ kg} \times 9.81\text{ m/s}^2 \approx 0.44\text{ N}$.
- **Thrust-to-Weight Ratio ($TWR$):** Target $TWR \ge 2.0:1$. Each of the 4 coreless motors must generate at least $0.22\text{ N}$ (~22.5 grams of thrust) at 100% throttle.

### 1.2 Laser Cutting the Chassis Frame
- **File:** `hardware/laser/neurofly_chassis_frame.dxf` / `neurofly_chassis_frame.svg`
- **Material Choices:**
  - *Option A (Recommended):* 2.5mm or 3.0mm 3-ply Baltic Birch Plywood (lightweight, high dampening against motor micro-vibrations).
  - *Option B:* 2.0mm Cast Acrylic (PMMA) (rigid and visually striking, but brittle on hard impacts).
- **Beam Kerf Compensation:**
  - Standard CO2 laser focal kerf is $\approx 0.15\text{ mm}$.
  - Motor holes are drawn at $\varnothing 8.5\text{ mm}$. With kerf, the inner hole will cut at $\approx 8.35 - 8.45\text{ mm}$, providing an ideal press-fit for 8520 coreless motor cans without requiring adhesive.

### 1.3 3D Printing the Propeller Ducts
- **File:** `hardware/cad/neurofly_prop_duct_40mm.stl`
- **Material:** TPU 95A (Thermoplastic Polyurethane) or lightweight PLA.
- **Why Ducts?** 
  - Prevents propeller contact with spectators, walls, and delicate indoor equipment.
  - Generates duct lip thrust in ground effect hover, increasing lift efficiency by 5-8%.
- **Slicing Parameters:**
  - Layer height: $0.18\text{ mm}$, Wall loops: 2 perimeters, Infill: 20% Gyroid, Print speed: $30\text{ mm/s}$ for TPU.

---

## ⚡ Week 2: Electronics Production & Circuit Assembly

### 2.1 Motor Driver Topology: Why MOSFETs?
Microcontroller GPIO pins can only deliver up to $20\text{ mA}$ at $3.3\text{ V}$. An 8520 coreless DC motor draws up to $1.8\text{ A}$ under full load! Therefore, we use an **N-Channel Logic-Level MOSFET (AO3400A)** as an electronic switch.

```text
                  +3.7V (1S LiPo VCC)
                       │
                 ┌─────┴────────┐
                 │              │
             [Coreless]     [SS14 Diode] (Cathode towards +3.7V)
             [ Motor  ]         │
                 │              │
                 └─────┬────────┘
                       │ Drain (D)
GPIO PWM ──[100Ω]── Gate (G) [AO3400A N-FET]
                       │ Source (S)
                      GND
                       │
       (10kΩ Pull-down resistor Gate-to-GND)
```

- **Schottky Diode (SS14 / 1N5819):** Absorbs high-voltage inductive back-EMF spikes generated when motor coils switch off. Without this, the MOSFET will be destroyed within milliseconds!
- **10kΩ Pull-Down Resistor:** Guarantees the gate voltage is pulled strictly to $0\text{ V}$ during microcontroller bootup, preventing motors from spinning uncontrollably before firmware initialization.

### 2.2 Soldering Verification Checklist
Before applying battery power, every student must execute this continuity checklist using a digital multimeter:

| Multimeter Test Point | Expected Measurement | Pass Criteria |
| :--- | :---: | :--- |
| **LiPo (+) to LiPo (-)** | Resistance $> 100\text{ k}\Omega$ (or open) | **NO SHORT CIRCUIT** (Beep = Immediate Fail!) |
| **XIAO 3V3 Pin to GND** | Resistance $> 10\text{ k}\Omega$ | No bridged SMD pins on regulator |
| **MOSFET Gate to GND** | Resistance $\approx 10\text{ k}\Omega$ | Pull-down resistor properly soldered |
| **Motor (+) to LiPo (+)** | Continuous ($0.0\text{ }\Omega$) | Power rail connection verified |
| **Motor (-) to MOSFET Drain**| Continuous ($0.0\text{ }\Omega$) | Drain output pad connected |

---

## 💻 Week 3: Firmware, Telemetry & Web Simulation

### 3.1 Wireless Protocol: ESP-NOW Architecture
Unlike bulky Wi-Fi routers that introduce 50–200ms latency, **ESP-NOW** communicates directly peer-to-peer at the MAC layer within $< 4\text{ ms}$:
- **Transmitter (Controller):** Broadcasts `NeuroFlyControlPacket` at $40\text{ Hz}$ containing throttle, pitch, roll, yaw, and mode flags.
- **Flight Controller (Drone):** Receives packets, checks CRC and monotonic packet sequence ID.
- **Safety Watchdog (Failsafe):** If no packet is received for $> 200\text{ ms}$, the drone enters `FAILSAFE` mode and automatically shuts down all motor PWM channels ($0\text{ V}$).

### 3.2 Hardware-in-the-Loop 3D Simulation
Students connect their physical XIAO ESP32-C6 remote controller to their computer via USB-C and practice inside the **NeuroFly 3D Arena**:
- **Launch Simulator:** Open `https://hisarcs.github.io/neurofly/` or run `cd webapp/public && python3 -m http.server 8000`.
- **Connect:** Click **"Connect Web Serial"** and select the XIAO USB Serial COM port.
- **Training Drills:**
  1. *Drill 1 (Gentle Lift):* Arm the drone, increase throttle to $60\%$, stabilize hover at $1.0\text{ m}$ altitude, and perform a controlled landing on the pad.
  2. *Drill 2 (Box Maneuver):* Fly forward $3\text{ m}$, bank right $2\text{ m}$, fly backward $3\text{ m}$, bank left $2\text{ m}$, return to origin.
  3. *Drill 3 (Gate Slalom):* Navigate through Gates 1, 2, 3, and 4 in consecutive order without contacting obstacle pylons.

---

## 🏆 Week 4: Flight Tuning, PID Stabilization & Grand Prix

### 4.1 Understanding Closed-Loop PID Control
An unassisted quadcopter is naturally unstable and cannot be flown manually without high-frequency stabilization. The flight controller runs a PID loop at $200\text{ Hz}$:

$$\text{Error } e(t) = \text{Target Angle} - \text{Measured IMU Angle}$$

$$u(t) = K_p \cdot e(t) + K_i \int e(t) \, dt + K_d \cdot \frac{de(t)}{dt}$$

- **Proportional ($K_p$):** Corrects current tilt. (Too low = sluggish response; too high = rapid, aggressive wobble).
- **Integral ($K_i$):** Corrects steady-state aerodynamic imbalances and battery pack shifts over time.
- **Derivative ($K_d$):** Predicts future trajectory and dampens overshoot, delivering butter-smooth stops.

### 4.2 Quadcopter Motor Mixer Matrix
The output of the PID controller is translated into motor speeds through an **X-configuration mixer**:

```text
    (M1: Front-Left - CW)      (M2: Front-Right - CCW)
                \              /
                 \            /
                  \   ▲     /
                      │ Forward
                  /   │     \
                 /            \
                /              \
    (M4: Rear-Left - CCW)      (M3: Rear-Right - CW)
```

$$\text{PWM}_{M1} = \text{Throttle} + \text{Pitch} + \text{Roll} - \text{Yaw}$$

$$\text{PWM}_{M2} = \text{Throttle} + \text{Pitch} - \text{Roll} + \text{Yaw}$$

$$\text{PWM}_{M3} = \text{Throttle} - \text{Pitch} - \text{Roll} - \text{Yaw}$$

$$\text{PWM}_{M4} = \text{Throttle} - \text{Pitch} + \text{Roll} + \text{Yaw}$$

### 4.3 Grand Prix Challenge Day Scoring Matrix
Teams enter the official arena heat and receive an aggregate Grand Prix Score:

$$\text{Total Score} = P_{\text{Weight}} + P_{\text{Laps}} + P_{\text{Autonomous/AI}}$$

1. **Weight Efficiency ($P_{\text{Weight}}$):** Baseline target is $45.0\text{ g}$. Teams receive $+10\text{ points}$ per gram below 45g!
2. **Lap Racing ($P_{\text{Laps}}$):** Complete as many laps through the 4 illuminated gates as possible in 3 minutes ($+10\text{ points}$ per clean gate pass).
3. **Autonomous Navigation ($P_{\text{Autonomous/AI}}$):**
   - 15-second unassisted hovering ($+50\text{ points}$).
   - ToF dynamic obstacle avoidance ($+75\text{ points}$).
   - ArUco target vision landing ($+100\text{ points}$).
