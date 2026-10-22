# NeuroFly Web Simulator & Ground Station 🎮

The NeuroFly Web Simulator allows students to connect their custom Seeed Studio XIAO ESP32-C6 remote controller to their browser via USB-C using the **Web Serial API**. Students can practice flying, test PID responsiveness, and simulate competition courses virtually before attempting maiden flights.

---

## 🚀 Features

1. **Zero Installation Required:**
   - Runs directly in Google Chrome or Microsoft Edge with zero native software drivers.
2. **Interactive 3D Arena (Three.js):**
   - FabLab competition gates and spatial ground grid.
   - Duct-protected quadcopter physics and dynamics model.
3. **Real-Time Telemetry HUD:**
   - Live displays for altitude, throttle, ARM status, score/lap counters, and battery simulation.

---

## 🏃 Getting Started

Run a lightweight local HTTP server:

```bash
cd webapp/public
python3 -m http.server 8000
```

Open `http://localhost:8000` in Google Chrome or Microsoft Edge.

- **With Hardware Controller:** Connect the XIAO ESP32-C6 transmitter via USB-C, click **"Connect Controller"**, and select the serial COM port.
- **Keyboard Fallback:** If hardware is not connected, use <kbd>Space</kbd> to Arm/Disarm, and <kbd>W</kbd>/<kbd>S</kbd>/<kbd>A</kbd>/<kbd>D</kbd> or arrow keys to fly.
