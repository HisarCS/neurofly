# NeuroFly Instructor & Mentor Guide 🧑‍🏫

This guide is designed for FabLab managers, makerspace facilitators, and K-12 STEM educators directing the NeuroFly program at Hisar IdeaLab, MIT Edgerton partner spaces, or Fab Academy classrooms.

---

## 🎯 Pedagogical Alignment

- **MIT Edgerton Center Experiential Learning:** Fosters iterative prototyping, rapid trial-and-error, and mechanical intuition through hands-on making.
- **Fab Academy Competencies:**
  - Computer-Controlled Cutting (Laser cutting airframe)
  - Electronics Production (PCB milling / assembly)
  - Embedded Programming (ESP32-C6 RISC-V, ESP-NOW telecommunication)
  - Interface & Application Programming (Web Serial API & Three.js flight telemetry)

---

## 🛡️ Laboratory Safety Protocols

1. **LiPo Battery Handling:**
   - Always charge 1S LiPo cells inside fireproof LiPo bags using certified balancing chargers.
   - Never charge above 1C (e.g. 350mA for a 350mAh pack).
   - Any punctured, bloated, or damaged battery must be submerged in a saltwater bath and recycled immediately.
2. **Propeller Safety:**
   - **Never install propellers during firmware flashing or bench testing.**
   - Prop guards (ducts) are mandatory during all indoor flight trials.
3. **Safety Glasses:**
   - All participants and observers inside the flight perimeter must wear ANSI Z87.1 certified safety glasses.

---

## 📋 Scrutineering & Evaluation Rubric (100 Points Total)

| Category | Max Pts | Assessment Criteria |
| :--- | :---: | :--- |
| **Engineering Notebook & CAD** | 20 | Complete Git commit history, CAD design choices, weight reduction log |
| **Circuit Craftsmanship** | 20 | Quality of solder joints, no cold joints/bridges, clean routing, safe wiring |
| **Airframe Weight Efficiency** | 20 | Scaled points based on All-Up-Weight ($<45.0\text{g} = 20\text{ pts}$, $-2\text{ pts}$ per gram over 45g) |
| **Simulator & Piloting Agility** | 20 | Laps completed in regulation 3-minute heat, gate navigation precision |
| **Autonomous / AI Mission** | 20 | Stable altitude-hold hovering, obstacle detection, or ArUco target tracking |

---

## 🔧 Common Troubleshooting Matrix

| Symptom | Probable Cause | Remediation Step |
| :--- | :--- | :--- |
| **Quadcopter flips immediately on takeoff** | Wrong motor rotation or inverted prop | Verify FL/RR are CW, FR/RL are CCW. Verify motor pins `D0..D3` match `PINOUT.md`. |
| **XIAO ESP32-C6 resets when throttle is raised** | Battery voltage sag / low-C rating | Replace with high-discharge (≥25C) LiPo; add 100uF low-ESR bypass capacitor across VCC/GND. |
| **No response in Web Simulator** | Web Serial port conflict or unselected baud rate | Ensure baud is set to `115200`. Check that serial monitor is closed in Arduino IDE / VSCode. |
| **One motor spins at maximum when plugged in** | Blown MOSFET or floating Gate | Inspect gate pull-down resistor (10kΩ); replace damaged AO3400 MOSFET. |
