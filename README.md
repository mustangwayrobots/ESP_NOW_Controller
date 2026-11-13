# ESP NOW Controller

The handheld controller for the [ESP NOW CAR](https://github.com/mustangwayrobots/ESP_NOW_CAR). It talks to the car over the low-latency **ESP-NOW** wireless protocol.

Developed for high school STEM and robotics education. It runs on an ESP32-C3 with a 1.3" OLED screen, a two-axis joystick, five buttons and a buzzer, and is powered by a 1S 500 mAh LiPo.

---

## ✨ Features

* Pairs with one car and remembers it. Pairing survives power-off and web flashing.
* Shows the link state, radio channel and the car's battery voltage.
* 19-item settings menu: speed limit, trims, acceleration, steering rate and expo, motor direction and swap, weapon angles, stick calibration, startup tune and more, each with a help page.
* Built-in trim test that drives the car forward and back.

---

## ⚡ Firmware Installation & Web Flashing

1. Connect the controller to your computer via USB.
2. Open the browser-based [Firmware Flasher Portal](https://mustangwayrobots.github.io/ESP_NOW_Controller/flash/).
3. Pick your board version, click **Connect**, select the serial port, and install. Your pairing and settings are kept.
4. The car and the controller must run matching firmware (same protocol version). After updating one, update the other too.

### Pairing

1. Hold **MENU** on the controller while switching it on, then let go. You have 20 seconds.
2. Hold the **bind button** on the car while switching it on.
3. The controller shows **Paired!** and both restart.

### Screens when not connected

| Screen | Meaning |
| :--- | :--- |
| **NOT PAIRED** | No car saved. Pair one. |
| **NO SIGNAL** | The car is off or out of range. |
| **NO REPLY** | The car is on but not answering this controller. Re-pair it, or update its firmware. |
| **FIRMWARE / Mismatch** | The car runs a different protocol version. Update both. |

### Building in Arduino IDE

* **Board:** MakerGO ESP32 C3 SuperMini · **Core:** esp32 3.3.12 (Boards Manager)
* **Library:** U8g2 2.36.19 (Library Manager). A pinned copy is kept in `libraries/U8g2` for the GitHub build; Arduino IDE uses the one in your own libraries folder.
* Open `ESP_NOW_Controller.ino`. Pick your board revision with the `#define HARDWARE_V1` line near the top (`V1` or `V2`).

---

## 📄 Specifications

| Document | Covers |
| :--- | :--- |
| [Controller Firmware FSD](docs/Controller_Firmware_FSD.md) | What the controller firmware does, protocol, menu, open items |
| [Controller Hardware FSD](docs/Controller_Hardware_FSD.md) | Controller board: power, pins, parts, open items |
| [Car Firmware FSD](https://github.com/mustangwayrobots/ESP_NOW_CAR/blob/main/docs/Car_Firmware_FSD.md) | Car firmware (separate repo) |
| [Car Hardware FSD](https://github.com/mustangwayrobots/ESP_NOW_CAR/blob/main/docs/Car_Hardware_FSD.md) | Car board (separate repo) |

---

## 📁 Repository Layout

| Path | Contents |
| :--- | :--- |
| `*.ino`, `*.h` | The firmware (Arduino sketch). `Protocol.h` must match the car repo's copy exactly. |
| `docs/` | Functional specifications |
| `libraries/` | Pinned copies of the libraries the build needs (U8g2) |
| `flash/` | Web flasher page (GitHub Pages) |
| `.github/workflows/build.yml` | Builds every board version and publishes the flasher on each push |
| `CHANGELOG.md` | What changed, by date |
| `LICENSE` | MIT License |

---

## 📜 License

Released under the [MIT License](LICENSE). The U8g2 library in `libraries/U8g2` keeps its own license (see its `LICENSE` file).
