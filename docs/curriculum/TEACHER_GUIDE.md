# NeuroFly Instructor & Mentor Handbook 🧑‍🏫
**A Comprehensive Guide for Makerspace Facilitators, Educators & Judges**  
*Hisar IdeaLab FabLab (MIT Edgerton Center Hub)*

---

## 🏛️ Pedagogical Foundation & Learning Outcomes

The NeuroFly curriculum is built upon **Experiential Constructionism** (pioneered by MIT's Seymour Papert and refined at the MIT Edgerton Center). Students do not consume abstract robotics theories—they construct physical, functional artifacts that reveal physical laws through immediate tactile and telemetry feedback.

### Key Student Learning Outcomes (SLOs):
1. **Design Thinking & Iterative Prototyping:** Formulate hypotheses regarding weight versus rigidity, evaluate mass budgets, and iterate CAD files.
2. **Electronics Literacy:** Master circuit schematics, interpret semiconductor datasheets, recognize passive pull-up/pull-down topologies, and execute defect-free SMD soldering.
3. **Embedded Systems Architecture:** Understand real-time operating constraints, hardware timers, 20kHz PWM generation, and peer-to-peer wireless packet exchange.
4. **Systems Engineering & Piloting Ergonomics:** Evaluate how clear and usable the remote controller and its telemetry display are, and test and document the system as a whole.

---

## 🛡️ Laboratory Infrastructure & Safety Management

### 1. Workstation Preparation (Per Team of 2-3 Students):
- 1× Temperature-controlled soldering station with fine chisel tip (set to $320^\circ\text{C} - 350^\circ\text{C}$ for lead-free solder).
- 1× Solder fume extractor with activated carbon / HEPA filtration.
- 1× Digital multimeter with fine needle probes.
- 1× Benchtop DC power supply with current limiting ($0.0 - 5.0\text{ V}$, $0.0 - 2.0\text{ A}$).
- 1× Anti-static ESD silicone workbench mat.
- 1× Safety glasses (mandatory ANSI Z87.1 approved for every student in the flight perimeter).

### 1b. Per-Classroom Setup (teacher)
- Choose a unique 16-character `NEUROFLY_PMK` / `NEUROFLY_LMK` pair for your class in `firmware/common/protocol.h` and give teams the **same** file; teams flash with `pio run -e drone -t upload` / `pio run -e controller -t upload` from `firmware/`.
- Run the pre-flight checklist in `docs/assembly/ASSEMBLY_GUIDE.md` (props off for bench test, failsafe test, arm only at minimum throttle) before every flight session.
- Honest status: firmware 0.3.0 has been written and reviewed but **not flight-verified without hardware**. Treat the first flights as supervised test flights and verify failsafe and emergency cut on each drone.

### 2. LiPo Battery Fire Safety Protocols:
- **Charging Station:** All 1S LiPo packs must be charged exclusively inside certified fireproof LiPo bags positioned on a non-flammable concrete or ceramic tile surface.
- **Maximum Charge Rate:** Never exceed $1\text{C}$ charge rate ($350\text{ mA}$ for a $350\text{ mAh}$ cell).
- **Damage Protocol:** Any battery showing physical swelling (puffing), denting, or punctured casing must be isolated immediately into a sandbox or metal bucket containing dry sand or sodium carbonate.
- **Class D / ABC Fire Extinguisher:** Positioned within 5 meters of the battery charging bench.

---

## 📋 Comprehensive 100-Point Grading Rubric

| Category | Novice (1-2 pts) | Developing (3 pts) | Proficient (4 pts) | Exemplary (5 pts) | Max Pts |
| :--- | :--- | :--- | :--- | :--- | :---: |
| **1. Digital Fabrication & Airframe** | Significant kerf errors; loose motor mounts; rough cuts. | Accurate frame but heavy; un-optimized prop ducts. | Clean laser cutting; precise motor press-fit; durable TPU ducts. | Outstanding mass optimization; parametric weight cutouts; AUW $< 42\text{ g}$. | **20** |
| **2. Circuit Soldering & Craftsmanship** | Cold solder joints; excess solder; loose wiring; polarity hazards. | Functional connections but messy wire routing; minor solder bridges. | Clean, shiny solder joints; proper pull-down/diode polarity; clean harness. | IPC-A-610 quality soldering; strain relief on all battery/motor leads. | **20** |
| **3. Firmware & Telemetry Integration** | Unable to flash firmware; failsafe non-functional. | Firmware loads but packet drop rate $> 15\%$; incorrect pin mapping. | Reliable ESP-NOW link; $< 5\text{ ms}$ latency; failsafe shuts down within 200ms. | Advanced telemetry calibration; custom PID rate profiles; Web Serial integration. | **20** |
| **4. Flight Dynamics & Piloting Agility** | Unable to maintain steady hover; frequent uncontrolled crashes. | Can hover but struggles with gate alignment; slow response. | Completes clean gate loops in regulation heat; controlled landings. | Rapid lap times; high-speed gate slalom; smooth banked turns. | **20** |
| **5. Autonomous / AI Mission Execution** | Did not attempt autonomous flight routines. | Attempted hover but drifted out of designated zone ($> 1\text{ m}$). | Autonomous 15s altitude-hold hover or ToF obstacle detection success. | Flawless autonomous mission execution with ArUco visual touchdown. | **20** |

---

## 🔍 Instructor Live Triage & Diagnostics Flowchart

When students encounter issues during laboratory sessions, guide them through this systematic debugging tree:

```
PROBLEM: Drone will not take off or spins out on the ground
├── Step 1: Multimeter Battery Check
│   └── Is LiPo voltage >= 3.80V?
│       ├── NO  -> Charge battery (voltage sag causes brownouts).
│       └── YES -> Proceed to Step 2.
│
├── Step 2: Motor Rotation & Propeller Direction Verification
│   └── Are diagonally opposed motors rotating in matching directions?
│       ├── FL (Front-Left):  Must spin CW with CW propeller.
│       ├── FR (Front-Right): Must spin CCW with CCW propeller.
│       ├── RR (Rear-Right):  Must spin CW with CW propeller.
│       └── RL (Rear-Left):   Must spin CCW with CCW propeller.
│       (If inverted, swap motor wire polarity or motor position).
│
├── Step 3: Failsafe & Wireless Signal
│   └── Is transmitter communicating with drone?
│       └── Open Serial Monitor at 115200 baud:
│           Verify packet reception callback triggers every 20-50ms.
│           If failing, check in this order:
│             1. Both boards flashed with the SAME NEUROFLY_PMK / NEUROFLY_LMK
│                (protocol.h). A mismatch = silent, no packets accepted.
│             2. Controller and drone are PAIRED (controller OLED shows RSSI).
│             3. Both run protocol v2 (telemetry `ver` field matches).
│             4. Failsafe latched (200 ms timeout) or emergency cut pressed:
│                disarm, throttle to minimum, re-arm.
│
└── Step 4: IMU Orientation & Sensor Bus
    └── Check I2C bus with I2C scanner sketch:
        MPU6050 must respond at address 0x68 (or 0x69).
        Verify IMU axes match frame: X = Forward, Y = Left, Z = Up.
```

---

## 🎓 Audience, Pacing & Age-Appropriate Safety (ages ~14-19)

**Audience.** NeuroFly targets high-school students (grades 9-12) and first-year university students. A competition for both groups is planned for the future; until it is announced, run the Grand Prix as an internal class or club event.

**Suggested session plans** (adapt to your timetable):
- *Weekly 90-minute club or class:* 8 sessions. Sessions 1-2 airframe (CAD, laser, print), 3-4 soldering and electrical checks, 5 firmware, pairing and simulator, 6 bench test and PID, 7 supervised test flights, 8 Grand Prix day.
- *Two intensive days or a camp week:* Day 1 fabrication and soldering, Day 2 firmware, simulator and flights; or spread across 5 mornings.
- *Grades 9-10:* start from the button controller variant and a pre-flashed firmware; give soldering demos in pairs. *Grades 11-12 and university:* teams edit and flash the firmware themselves.

**Safety for this age group.**
- Soldering, laser cutting and 3D printers: an adult supervises at all times; first-time users get a demonstration and a short check before working alone. Fume extraction on, hair tied back, no food or drink at the bench.
- LiPo packs are charged and stored only by or under direct supervision of an adult, never overnight or unattended. Students under 16 do not handle damaged packs.
- Props off for every bench test. Flying happens only inside the netted arena with safety glasses for everyone present, and a staff member with a hand on the emergency cut.
- Keep to the technical-inspection checklist in `docs/competition/RULES.md`.

**Optional university extension track.** For first-year university students or advanced high-school teams:
- *State estimation:* implement and compare a complementary filter and a Kalman filter on the IMU data; log and plot attitude error.
- *PID tuning math:* derive the rate loop from the motor model, tune with step-response metrics (rise time, overshoot, settling time), try Ziegler-Nichols and compare with hand tuning.
- *Computer vision:* extend `docs/ai_autonomous/aruco_tracker.py` with camera calibration and pose estimation (`solvePnP`) and close the loop over the serial link.
Assess the extension with a short written report plus logged data; it is optional and sits on top of the 100-point rubric.
