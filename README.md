# HaptiGrip 🦾
### EMG-Driven Prosthetic Hand with Smart Haptic Feedback

> Presented at **ITAI 2026** – 2nd International Conference on Information Technology and Artificial Intelligence, Lasell University, Newton, Massachusetts, USA.

[![Conference](https://img.shields.io/badge/ITAI_2026-Presented-blue)](https://scrs.in/conference/icitai2026)
[![Arduino](https://img.shields.io/badge/Arduino-C%2B%2B-teal)](arduino/)
[![Python](https://img.shields.io/badge/Python-3.x-yellow)](python/)

---

## 📌 Overview

HaptiGrip is a command-driven prosthetic hand system built on Arduino with:
- **5 servo-driven fingers** (Thumb, Index, Middle, Ring, Pinky)
- **Bluetooth (HC-05)** for wireless command input
- **Vibration motor** for haptic feedback
- **FSR pressure sensor** (optional, configurable)
- **Real-time Python visualizer** showing live finger angles via USB serial

---

## 🗂️ Repository Structure

\```
haptigrip/
├── arduino/
│   └── haptigrip.ino        # Arduino firmware (servo control + BT + EMG output)
├── python/
│   └── visualizer.py        # Live finger angle graph via matplotlib
├── .gitignore
├── LICENSE
└── README.md
\```

---

## 🔧 Hardware Requirements

| Component | Details |
|---|---|
| Arduino (Uno/Mega) | Main microcontroller |
| 5× Servo Motors | One per finger |
| HC-05 Bluetooth Module | Wireless command input |
| Vibration Motor | Haptic feedback |
| FSR Sensor (optional) | Force-sensitive resistor on A0 |
| USB Cable | Serial data to Python visualizer |

### Pin Mapping

| Finger | Pin |
|---|---|
| Thumb | D3 |
| Index | D5 |
| Middle | D6 |
| Ring | D9 |
| Pinky | D10 |
| FSR | A0 |
| Vibration Motor | D2 |
| HC-05 RX/TX | D0/D1 |

---

## ⚙️ Arduino Setup

1. Open `arduino/haptigrip.ino` in the Arduino IDE.
2. Connect your Arduino via USB.
3. Upload the sketch.
4. Open Serial Monitor at **9600 baud** to verify output.

### Configuration (top of `haptigrip.ino`)

\```cpp
#define FSR_ENABLED  false   // Set true to enable FSR auto-grip
#define THUMB_FLIP   true    // Flip thumb servo direction if needed
#define MOTOR_MODE   0       // 0 = source, 1 = sink (motor wiring)
\```

---

## 📡 Bluetooth Commands

Send these commands from any Bluetooth terminal app (e.g., Serial Bluetooth Terminal on Android):

| Command | Action |
|---|---|
| `open` | Open all fingers |
| `close` | Close all fingers (fist) |
| `grip` | Power grip pose |
| `peace` | Peace sign ✌️ |
| `ok` | OK sign 👌 |
| `point` | Point finger ☝️ |
| `wave` | Wave hand 👋 |
| `thumbsup` | Thumbs up 👍 |
| `reset` | Return to rest position |
| `open thumb` | Open a single finger |
| `close index` | Close a single finger |
| `move ring 90` | Set finger to exact angle |

---

## 📊 Python Visualizer

The visualizer reads live EMG angle data from the Arduino over USB and plots all 5 finger angles + FSR pressure in real time.

### Requirements

\```bash
pip install pyserial matplotlib
\```

### Configuration

Edit the top of `python/visualizer.py`:

\```python
USB_PORT = 'COM5'   # Arduino USB port (Windows: COMx, Linux/Mac: /dev/ttyUSBx)
BT_PORT  = 'COM7'   # Bluetooth port for sending commands
BAUD     = 9600
\```

### Run

\```bash
python python/visualizer.py
\```

### Serial Data Format

Arduino sends:
\```
EMG,<timestamp>,<thumb>,<index>,<middle>,<ring>,<pinky>,<fsr>,<label>
\```
Example:
\```
EMG,12345,30,10,10,10,10,0,idle
\```

---

## 🤖 Gesture Haptic Feedback

| Gesture | Buzz Pattern |
|---|---|
| open, peace, ok, point, thumbsup | Single buzz |
| grip, wave | Double buzz |
| reset | Single buzz (on manual call) |
| startup | Triple buzz |

> *Paper presented at ITAI 2026 — Certificate No. SCRS/ITAI2026/PC/170*
