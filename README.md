# Atmospheric Water Generator (AWG) Control System

An embedded control system based on the **ESP32-C3 (RISC-V)** architecture designed to automate the condensation of water from ambient air. The system dynamically manages high-power thermoelectric coolers (TEC) and thermal dissipation mechanisms based on real-time thermodynamic calculations.

---

## 🚀 Project Overview

The core objective of this project is to optimize the water generation process by dynamically calculating the **Dew Point** of the ambient air and regulating the cooling surface accordingly. 

### Key Features:
* **Dynamic Dew Point Tracking:** Uses ambient temperature and relative humidity to calculate the exact condensation threshold.
* **Thermal Management:** Adaptive PWM control of a high-power Delta fan based on the Peltier module's hot-side temperature to prevent thermal runaway.
* **Hardware Interrupts:** Utilizes hardware timers and interrupts (FG signal) to accurately measure the cooling fan's RPM without blocking the main control loop.
* **High-Current Switching:** Integrates logic-level MOSFETs and dedicated gate drivers to safely control the 300W load.

---

## 🛠️ Hardware Architecture

The system is built to safely handle high currents required by thermoelectric cooling, separating the 3.3V logic circuit from the 12V power circuit.

* **MCU:** ESP32-C3 (RISC-V) SoC with Wi-Fi/BLE capabilities.
* **Sensors:** Dual DHT22 sensors (Ambient conditions & Radiator temperature).
* **Thermoelectric Cooler (TEC):** TEC1-12706 Peltier modules.
* **Power Electronics:** * **IRLZ44N Logic-Level MOSFET:** Used for low $R_{ds(on)}$ switching of the Peltier elements.
  * **TC4420EOA Gate Driver:** Ensures rapid gate capacitance charging to minimize switching losses.
* **HMI:** 0.91" I2C OLED display for real-time monitoring of Dew Point, temperatures, and fan RPM.

---

## 🧠 Thermodynamics & Control Logic

The condensation efficiency relies on keeping the cold side of the Peltier module strictly below the dew point $T_{dp}$, while simultaneously preventing the hot side from exceeding operational limits. 

The embedded logic:
1. Samples ambient $T$ and $RH\%$.
2. Approximates the dew point: $T_{dp} \approx T - \frac{100 - RH}{5}$
3. Adjusts the duty cycle of the heat-sink fan to maximize thermal dissipation on the hot side.
4. Toggles the TEC module to maintain the condensation state.

---

## 📂 Repository Structure

* `/src/main.cpp` - The main C++ firmware running on the ESP32-C3.
* `/hardware` - Contains the electrical schematics and custom PCB layout.
* `/docs` - Contains photos of the working prototype and performance readouts.

## ⚙️ How to Build
The code is written using the Arduino framework for ESP32. It can be compiled using the Arduino IDE or PlatformIO. Required libraries: `Adafruit GFX`, `Adafruit SSD1306`, `DHT sensor library`.

### 🎥 Live Prototype Testing


https://github.com/user-attachments/assets/6294b0c6-e3f9-4f8c-8508-4db3b5c4cbf2


*Watch the ESP32-based control system in action. The firmware actively monitors ambient conditions to compute the dew point, regulates the Peltier cooling module, and adjusts the heat-sink fan speed to ensure efficient water condensation without thermal runaway.*
