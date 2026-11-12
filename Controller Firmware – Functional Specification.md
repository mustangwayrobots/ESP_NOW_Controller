# Controller Firmware – Functional Specification

Nov 12, 2026 · @Hog

This document describes what the handheld controller firmware does as of the Nov 12, 2026 code, tested on V1 controller hardware. It is one of four specifications: Car Firmware, Controller Firmware (this one), Car Hardware and Controller Hardware. The controller now matches the car's confirmed pairing handshake (mode 3).

## 1. Scope

- **Covers:** `ESP_NOW_Controller.ino`, `HardwareConfig.h`, `ConfigData.h/.ino`, `RadioChannel.ino`, `Inputs.ino`, `MotorControl.ino`, `SendData.ino`, `setupNormalMode.ino`, `loopNormalMode.ino`, `setupBindingMode.ino`, `loopBindingMode.ino`, `storage.ino`, `Menu.ino`, `MenuActions.ino`, `MenuHelp.ino`, `Screens.ino`, `displayMsg.ino`, `TestRoutine.ino`, `Beep.ino`, `playtune.ino`, `songs2.h`.
- **Does not cover:** the car side of the protocol (Car Firmware FSD), the controller board (Controller Hardware FSD), or the web flasher files in the `flash` and `.github` folders.
- **Status:** Draft 2.0, as-built. Unknowns are marked **\[?\]**.

## 2. What the Controller Does

1. Pairs with one car over ESP-NOW using a three-step handshake and remembers the car's MAC address in flash.
2. Reads a two-axis joystick and five buttons and mixes them into left and right motor commands.
3. Sends drive and weapon commands to the car: every 20 ms while driving, every 50 ms when idle, every 100 ms when not linked.
4. Shows link state, radio channel and the car's battery voltage on a 128×64 OLED.
5. Offers a 19-item settings menu with an edit screen, help pages, a trim test and a startup tune.

## 3. Hardware Dependencies

All pin numbers live in `HardwareConfig.h`. The board is chosen by the line `#define HARDWARE_V1` near the top of `ESP_NOW_Controller.ino`; change it to `HARDWARE_V2` for the V2 board. A build that passes `-DHARDWARE_Vx` (the GitHub flasher build) overrides it, and defining both stops the build with an error. Both boards use an SH1106 128×64 OLED (`OLED_SH1106_128X64`).

| Macro | Role | V1 (GPIO) | V2 (GPIO) |
| --- | --- | --- | --- |
| `JOYSTICK_THROTTLE_PIN` | Forward/back stick axis (analog) | 3 | 3 |
| `JOYSTICK_STEER_PIN` | Left/right stick axis (analog) | 4 | 4 |
| `SPIN_BUTTON_PIN` | Stick press, spin on demand | 7 | 7 |
| `MENU_BUTTON_PIN` | Menu and back, pairing at power-up | 10 | 10 |
| `SELECT_BUTTON_PIN` | Select | 8 | 8 |
| `UP_BUTTON_PIN` | Up, weapon while driving | 1 | 1 |
| `DOWN_BUTTON_PIN` | Down | 0 | 0 |
| `PIEZO_PIN` | Buzzer through a MOSFET | 20 | 20 |
| `OLED_SDA_PIN`, `OLED_SCL_PIN` | OLED I2C at 400 kHz | 5, 6 | 5, 6 |

The V2 values are copies of V1 and are marked "CHECK against V2 schematic" in the file (see C-18). `DEBUG_SERIAL_OUTPUT` (false by default) prints raw stick readings 5 times a second.

## 4. Software Structure

| File | Role |
| --- | --- |
| `ESP_NOW_Controller.ino` | Shared variables, display driver choice, `setup()`, mode dispatch |
| `HardwareConfig.h` | Pin numbers and screen type per board |
| `ConfigData.h` | Named settings list (`SettingId`), tuning numbers, packet struct, shared declarations |
| `ConfigData.ino` | Menu table, factory defaults, help text |
| `RadioChannel.ino` | `getRadioChannel()`: the one place the channel is worked out |
| `Inputs.ino` | Reads stick and buttons; press detection for MENU and SELECT |
| `MotorControl.ino` | Mixing, trim, acceleration limit, pin values, weapon value |
| `SendData.ino` | Sends a mode 1 packet to the cached car MAC |
| `setupNormalMode.ino`, `loopNormalMode.ino` | Radio, link watchdog, adaptive rate, driving screens |
| `setupBindingMode.ino`, `loopBindingMode.ino` | Pairing handshake |
| `storage.ino` | All flash reads and writes (settings and car MAC) |
| `Menu.ino`, `MenuActions.ino`, `MenuHelp.ino` | Menu navigation, editing and saving, help paging |
| `Screens.ino` | Every OLED screen |
| `displayMsg.ino` | Old one-line message call, now a wrapper for `showStatus()` |
| `TestRoutine.ino` | Trim test |
| `Beep.ino`, `playtune.ino`, `songs2.h` | Buzzer, RTTTL player, 94 tunes |

The sketch is a cooperative loop. The Arduino IDE joins all `.ino` tabs into one unit; shared variables are declared once in `ConfigData.h`. The CPU runs at 80 MHz. Built with Arduino-ESP32 core 3.3.12.

## 5. Communication Protocol (Controller View)

The packet is a packed 25-byte struct shared with the car (see Car Firmware FSD): `protocolMode` (1 byte), then `motor_A1`, `motor_A2`, `motor_B1`, `motor_B2`, `weapon` and `battery` (4-byte ints). The driving channel comes from `getRadioChannel()`: the last byte of the controller's own MAC modulo 3 gives 0 → channel 1, 1 → channel 6, 2 → channel 11. Pairing always uses channel 1 (`BINDING_CHANNEL`).

| Mode | Controller does |
| --- | --- |
| 0 Bind request | Broadcasts a zeroed packet on channel 1 every 200 ms while pairing |
| 2 Bind acknowledgement | Receives it from the car; the first one wins and its sender MAC is kept |
| 3 Bind confirmed | Sends it to the car every 100 ms for 1.5 s after the acknowledgement |
| 1 Normal | Sends drive and weapon values to the car; receives the car's battery voltage (mV) |

The link counts as up when a unicast send has been acknowledged by the car's radio within the last 500 ms. Telemetry is accepted only from the paired car's MAC and only if the packet is at least 25 bytes.

## 6. Operating Modes

### 6.1 Boot

1. Set CPU to 80 MHz, start Serial at 115200 and print the board name.
2. Start I2C at 400 kHz and the OLED (flipped, full contrast).
3. Set the buzzer pin as output and the five button pins as inputs with pull-ups.
4. Load the 19 settings from flash. A setting never saved, or outside its menu range, gets its factory default.
5. MENU held: Pairing Mode. Otherwise play the startup tune if set (1 to 93, blocks until done), then Normal setup.

### 6.2 Normal setup

1. Station mode, 19.5 dBm, channel from `getRadioChannel()`.
2. Read the car MAC from flash into RAM (`cachedCarMac`). `carPaired` is false if no MAC is saved.
3. Start ESP-NOW (restart the board if it fails), set the channel, add the car as a unicast peer.
4. Send callback: on success, record the time.
5. Receive callback: copy the packet into `ReceivingData` if it is long enough and from the paired car.

### 6.3 Normal loop

1. Read inputs.
2. If the menu is open, run only the menu, wait 10 ms and return. Nothing is transmitted.
3. Link watchdog: up if a send was acknowledged in the last 500 ms.
4. **Not linked:** a 1000 Hz, 100 ms beep every 10 s. The screen shows `NO SIGNAL` / Waiting / Turn on the car, or `NOT PAIRED` / No car / Hold MENU at power on if no car is saved. If a car is saved, it is re-added as a peer every 2 s and a drive packet is sent every 100 ms. If no car is saved, nothing is sent.
5. **Linked:** the screen shows `CONNECTED`, the channel, and the car battery as x.xx V (`-.-- V` until the first reading). It redraws at most 4 times a second.
6. **Adaptive rate:** input counts as active when any motor value is not zero or UP or SPIN is held. Active sends every 20 ms, idle every 50 ms. On letting go, the stop packet is sent at once.
7. Wait 1 ms.

### 6.4 Pairing

**Setup:** station mode, 19.5 dBm, ESP-NOW on channel 1, broadcast peer. The receive callback ignores short packets, keeps the sender MAC of the first mode 2 packet and raises a flag. Flash writes are done in the loop, not the callback.

**Loop:**

1. While MENU is still held: `PAIRING` / Release / Let go of MENU. The 20 s window starts on release.
2. Searching: shows `Searching` with a countdown and broadcasts mode 0 every 200 ms.
3. No answer in 20 s: shows `Timed out` / Old car is kept, then restarts after 1.5 s. The old pairing stays saved.
4. Car answers: saves its MAC (only if it is a different car), adds it as a peer and shows `Paired!` with the last two MAC bytes.
5. Sends mode 3 to the car every 100 ms for 1.5 s, waits 0.5 s and restarts into Normal Mode.

### 6.5 Settings menu and screens

Every screen has a white header bar (title left, info right), a main area and a one-line hint at the bottom. Status screens redraw only when their text changes, because a full frame takes about 25 ms over I2C.

| Button | Settings list | Edit screen | Help pages |
| --- | --- | --- | --- |
| MENU | Close the menu | Cancel, back to list (not saved) | Back to list |
| SELECT | Edit the item (Help opens help) | Save | Back to list |
| UP | Previous item (higher on screen) | Value +1 | Previous page |
| DOWN | Next item (lower on screen) | Value −1 | Next page |

- **List:** header `SETTINGS` and n/19, four rows with the current item highlighted and its value on the right. Moving past either end wraps around.
- **Edit:** header shows the setting name and EDIT, the value in large text, a range bar (not for on/off items, tune or reset) and a hint.
- **Help:** header shows the setting name and n/19, with its help text wrapped over up to four lines.
- UP and DOWN repeat every 150 ms while held. MENU and SELECT act once per press, with a 200 ms lockout.
- Saving writes all 19 settings to flash and shows `Saved` for 0.5 s.
- **Throttle zero / Steer zero:** editing starts from the live stick reading, so centering the stick and pressing SELECT twice calibrates it.
- **Factory reset:** SELECT restores all defaults (pairing is kept) and shows `Reset done` for 1.2 s.
- **Trim test:** SELECT saves, then drives forward for the set time, stops 1 s, reverses for the set time, then sends 1 s of stop packets. Full stick at `Max speed`, through the acceleration limit. MENU stops it early.
- No packets are sent while the menu is open, so the car stops on its own 250 ms timeout.

## 7. Settings

Code uses the names in the `SettingId` list (for example `currentSettings[SET_MAX_SPEED]`). The number is the save position in flash.

| # | Menu label | Range | Default | Effect |
| --- | --- | --- | --- | --- |
| 0 | Max speed | 0–100 % | 100 | Scales top PWM |
| 1 | Fwd trim | −50 to 50 | 0 | Drift fix going forward: + slows left wheel, − slows right |
| 2 | Rev trim | −50 to 50 | 0 | Same, in reverse |
| 3 | Accel rate | 0–100 % | 50 | Soft start: 0 takes 1 s from stop to full, 100 is instant |
| 4 | Steer rate | 0–100 % | 40 | Steering share of max speed |
| 5 | Steer expo | 0–100 % | 60 | Blend of linear and cubic steering |
| 6 | Spin speed | 0–100 % | 100 | Spin-on-demand speed |
| 7 | Motor A dir | Normal / Rev | Normal | Swap A pins |
| 8 | Motor B dir | Normal / Rev | Normal | Swap B pins |
| 9 | Swap motors | No / Yes | No | Swap A and B channels |
| 10 | Weapon on | 0–180° | 180 | Servo angle while UP is held |
| 11 | Weapon off | 0–180° | 0 | Servo angle when UP is released |
| 12 | Trim test | 1–10 s | 3 | Length of each test leg; saving runs the test |
| 13 | Deadzone | 20–150 | 20 | Center deadzone in raw ADC counts |
| 14 | Throttle zero | 0–4095 | 2048 | Raw ADC center, forward/back axis |
| 15 | Steer zero | 0–4095 | 2048 | Raw ADC center, left/right axis |
| 16 | Startup tune | Off, 1–93 | Off | Tune at power-on; edit screen shows its name |
| 17 | Factory reset | n/a | n/a | Restore defaults, pairing kept |
| 18 | Help | n/a | n/a | Help pages |

## 8. Drive Mixing and Weapon Value

`CalculateMotorDriverValues()` fills in the packet; the caller then sends it with `SendData()`. Left drives motor A, right drives motor B. Steps in order:

1. **Center.** Throttle = −(raw − `Throttle zero`), because pushing forward lowers the reading. Steering = raw − `Steer zero`. Units are raw ADC counts.
2. **Deadzone.** An axis below `Deadzone` (at least 20) becomes 0.
3. **Spin held:** both wheels at `Spin speed` in opposite directions toward the steering side, or stopped if steering is centered.
4. **Otherwise arcade drive.** Each axis is divided by 1800 counts (`STICK_FULL_RANGE`) and clamped to ±1. Throttle is scaled to ±(`Max speed` × 2.55). Steering blends linear and cubic by `Steer expo`, then is scaled by max speed × `Steer rate`. Left = throttle + steering, right = throttle − steering.
5. **Trim** applies only while the stick is off center: `Fwd trim` when throttle is 0 or more, `Rev trim` when negative.
6. **Clamp** to ±255.
7. **Acceleration limit.** Speeding up is limited to full scale in (100 − `Accel rate`)% of 1000 ms, measured in real time (time step capped at 50 ms). Slowing down is instant. Reversing passes through zero first. At 100% there is no limit.
8. **Jitter floor.** A wheel value below 15 becomes 0.
9. **Pin values.** Positive speed goes to input 1 of the motor, negative to input 2. Then `Motor A dir`, `Motor B dir` and `Swap motors` are applied.
10. **Weapon.** UP held: `Weapon on`. UP released: `Weapon off`.

`SendData()` sets mode 1 and sends the 25 bytes to the car MAC held in RAM. It does not touch flash.

## 9. Functional Requirements

| ID | Requirement |
| --- | --- |
| CR-01 | The controller shall enter Pairing Mode if MENU is held at power-up, and Normal Mode otherwise. |
| CR-02 | At boot it shall load the 19 settings from flash, using defaults for any never saved or out of range, and load the car MAC into RAM. |
| CR-03 | It shall drive on the channel from `getRadioChannel()` (1, 6 or 11) and pair on channel 1. |
| CR-04 | It shall send drive packets every 20 ms while input is active, every 50 ms when idle, every 100 ms when not linked (only if a car is saved), and send a stop packet at once when input goes idle. |
| CR-05 | It shall treat the link as up when a unicast send was acknowledged within the last 500 ms. |
| CR-06 | It shall show `CONNECTED` with channel and car battery (x.xx V) when linked, `NO SIGNAL` when paired but not linked, and `NOT PAIRED` when no car is saved. |
| CR-07 | It shall accept telemetry only from the paired car, in packets of at least 25 bytes. |
| CR-08 | It shall compute motor and weapon values as in Section 8. |
| CR-09 | It shall provide the 19-item menu with the button actions in Section 6.5 and save all settings to flash when an edit is confirmed. |
| CR-10 | It shall transmit nothing while the menu is open. |
| CR-11 | In Pairing Mode it shall broadcast mode 0 every 200 ms, accept the first mode 2, save the MAC only if it changed, send mode 3 for 1.5 s and restart. |
| CR-12 | Pairing shall time out 20 s after MENU is released and keep the old pairing. |
| CR-13 | It shall play the selected tune at power-up when the setting is 1 to 93. |
| CR-14 | It shall beep for 100 ms every 10 s while not linked. |
| CR-15 | The trim test shall drive forward, stop 1 s, reverse, then send 1 s of stop packets, and stop early when MENU is pressed. |

## 10. Key Parameters

All are named constants in `ConfigData.h`, except the pairing timings (top of `loopBindingMode.ino`) and the menu timings.

| Parameter | Value |
| --- | --- |
| Packet interval | 20 ms active, 50 ms idle, 100 ms not linked |
| Link-lost threshold | 500 ms without an acknowledged send |
| Peer re-add while not linked | every 2 s |
| Stick full range | 1800 ADC counts from center |
| Deadzone minimum | 20 ADC counts |
| Jitter floor | 15 of 255 |
| Accel ramp at 0% | 1000 ms stop to full; time step capped at 50 ms |
| Bind request / confirm interval | 200 ms / 100 ms |
| Confirm time, pairing timeout | 1.5 s, 20 s |
| Menu repeat, button lockout | 150 ms, 200 ms |
| Driving screen redraw | at most every 250 ms |
| Waiting beep | 1000 Hz, 100 ms every 10 s |
| TX power, CPU, I2C | 19.5 dBm, 80 MHz, 400 kHz |

## 11. Storage

All flash access is in `storage.ino`, namespace `rc-config`.

- `set_0` to `set_18`: the settings, saved by position in the `SettingId` list. New settings must be added at the end of the list.
- All 19 settings are written on every confirmed edit. On load, a value outside its menu range is replaced by its default.
- `mac`: the car MAC as the text `xx:xx:xx:xx:xx:xx`. Read once at boot; a value that is not 17 characters means not paired.
- The MAC is written only when pairing finds a different car.

## 12. Observations and Optimization Candidates

Severity is my estimate.

**Resolved since Draft 1:** C-01, C-02, C-04, C-05, C-06, C-07, C-09, C-10, C-11, C-13, C-22, most of C-21. Resolved Nov 12: menu UP/DOWN direction, C-08 range checks, C-14 `displayMsg()` cleanup, C-15 tune octave range, C-23 deadzone minimum 20, C-24 no sending when not paired.

| ID | Severity | Status | Observation |
| --- | --- | --- | --- |
| C-18 | High | Waiting for V2 board | **V2 pins are not verified.** The V2 block in `HardwareConfig.h` copies V1 and is marked CHECK. |
| C-19 | Medium | Open, choose a fix | **Channel load can be uneven.** The channel comes from the controller MAC, so 15 pairs could land 8, 3 and 4 on channels 1, 6 and 11. Each pair at 50 Hz uses an estimated 5% of a channel (not measured). |
| C-03 | Low | Open | **Blocking sounds.** The waiting beep blocks 100 ms (only while not linked). The startup tune blocks boot for its length. |
| C-12 | Low | Open | **Modem sleep.** Whether the core's default power save slows telemetry is not tested **\[?\]**. |
| C-21 | Low | Accepted | **First car to answer wins** if two cars are pairing at once. |
| C-16 | Low | Accepted | **Trim test runs at full `Max speed`.** MENU stops it. |
| C-17 | Low | Accepted | **GPIO8 is SELECT and may be the module's LED pin.** The module has no other free pin. |
| C-25 | Medium | Open, needs both sides | **`CONNECTED` can show when the car is not listening.** The link counts as up when the car's radio acknowledges a send. That acknowledgement happens below ESP-NOW, even if the car ignores the packet because it is paired to another controller. The screen then shows `CONNECTED` with `-.-- V` while the car does not move. Counting the link as up only after a reply from the car (for example telemetry every 1 s) would fix it. |
| C-26 | Medium | Open, needs both sides | **No protocol version, and received packets are not checked for mode.** Telemetry and drive both use mode 1, and the receive callback accepts any mode from the car. A version byte and a separate telemetry mode would let each side reject mismatched firmware. Matches car items N-07 and O-06. |

## 13. Open Questions

1. **C-19 fix.** Choose between a channel set in the menu and an automatic quietest-channel pick at pairing. Both need a car firmware change.
2. **V2 pins.** Confirm the V2 pin list once the board arrives (C-18).
3. **Weapon.** Details deferred to next year.
