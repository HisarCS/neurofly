# NeuroFly Challenge & Competition Guide 🏆

The NeuroFly Competition hosted at **Hisar IdeaLab / FabLab** evaluates students' interdisciplinary engineering prowess across digital fabrication, electrical assembly, piloting agility, autonomous algorithms, and AI perception.

---

## 🏆 Grand Prix Overall Scoring Formula

The overall champion team is determined by an aggregate score spanning three core pillars:

$$\text{Final Score} = P_{\text{Weight}} + P_{\text{Laps}} + P_{\text{Autonomous/AI}}$$

---

### Pillar 1: Fabrication & Weight Efficiency ($P_{\text{Weight}}$)
*"Build the lightest yet most robust airframe"*

In aeronautical engineering and FabLab digital fabrication, weight reduction directly translates into agility, battery endurance, and safety.
- Every drone is weighed at technical inspection (All-Up-Weight: AUW, including battery).
- **Baseline Weight Target:** **45.0 grams**.
- **Scoring:** Bonus points are awarded for every gram below 45g without compromising structural integrity and safety ducts. Clean KiCad carrier PCB design and minimal wiring earn design bonuses.

---

### Pillar 2: Agility, Speed & Endurance ($P_{\text{Laps}}$)
*"Most laps completed in regulation time"*

- Drones take off simultaneously or in heats over a **3-minute round**.
- Pilots navigate their quadcopters through illuminated FabLab race gates and obstacles using their custom handheld controllers.
- **Scoring:** **+10 Points** for every cleanly navigated gate and full lap completion.
- Crashes incur no structural penalty due to mandatory prop guards; pilots may retrieve/re-arm and continue within the 3-minute window.

---

### Pillar 3: Autonomous Navigation & AI Missions ($P_{\text{Autonomous/AI}}$)
*"Algorithmic precision without pilot intervention"*

Students program embedded state machines and computer vision routines:
1. **Autonomous Precision Takeoff & Altitude Hold:**
   - Drone launches automatically, locks onto a 50cm altitude above the ground using ToF ranging, and hovers stably for 15 seconds: **+50 Points**.
2. **Dynamic Obstacle Avoidance:**
   - Drone cruises forward down a hallway; upon detecting an obstacle via ToF, it decelerates, hovers, and routes around or halts smoothly: **+75 Points**.
3. **Computer Vision & ArUco Target Tracking:**
   - Drone integrates with an overhead vision station or onboard camera stream to localize and execute a pinpoint landing on designated visual markers: **+100 Points**.

---

## 💻 Web App & Simulation Environment

Before students test their physical drones in the arena, they validate their firmware, PID responsiveness, and controller ergonomics in the **NeuroFly 3D Browser Simulator**:
- **Hardware-in-the-Loop:** Students plug their physical XIAO ESP32-C6 transmitter into any PC/Chromebook via USB-C.
- **Zero-Install Web Serial API:** Direct telemetry and stick input streaming directly in Google Chrome / Edge.
- **Virtual Gate Course:** Teams benchmark lap times and tune control rates risk-free before maiden flight.

---

## 🛡️ Mandatory Safety Regulations

1. **Enclosed Propeller Ducts:** No drone may power its motors without 3D-printed prop guards / ducts enclosing all four propellers.
2. **Failsafe Timeout:** If communication between the transmitter and drone drops for longer than **200 ms**, the flight controller must automatically cut all motor PWM outputs (`DISARM`). Judges may test this at technical inspection by switching the transmitter off while the motors spin (props off).
3. **Emergency Cut:** Holding the controller's **ARM button for 1 second or longer** triggers the emergency cut (`aux_buttons` bit 2 in the protocol, sent for 500 ms). It immediately stops all motors, locks the drone out until a disarmed state is seen, and requires a fresh arm at minimum throttle. The pilot, and the judge of each heat, must be able to trigger it at any time; teams must demonstrate it at technical inspection.
4. **Arming:** Drones may only be armed at minimum throttle. No arming inside the pit area with propellers installed.
5. **Pairing & Radio Discipline:** Each controller is paired to exactly one drone and the link is encrypted. Each classroom/team uses its own key pair (`NEUROFLY_PMK` / `NEUROFLY_LMK` in `firmware/common/protocol.h`) so that neighbouring teams cannot command each other's drones. Pairing is stored on both devices (the drone keeps its controller's MAC; hold BOOT 3 s shortly after power-up to clear it, hold MODE at controller power-up to re-pair). An unpaired drone never arms. Pair your drone before the heat, and only power up in your assigned slot. (Pairing is compile-verified only; not yet bench-tested on hardware.)
6. **Protocol Version:** Controller and drone must run the same protocol version (v2). Mismatched or corrupted packets (bad checksum) are ignored by the drone.
7. **Safety Eyewear:** All pilots, pit crew, and judges must wear safety glasses during flight tests and live heats.

> The scoring formulas above are unchanged in v0.2.0. Note that the firmware safety behaviours (failsafe, emergency cut, arming rule) are implemented but have not been verified on physical hardware by the authors; inspectors should verify them on each drone.
