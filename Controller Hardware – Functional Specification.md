# Controller Hardware – Functional Specification

Nov 11, 2026 · @Hog

This document describes the handheld controller board as drawn in your EasyEDA project (three schematic revisions) and lists hardware items worth checking. It is one of four specifications: Car Firmware, Controller Firmware, Car Hardware and Controller Hardware (this one). The most important finding is that the newest schematic puts the joystick's horizontal axis on a pin that cannot read analog signals (see HC-1).

## 1. Scope

- **Covers:** the controller board, its power path, inputs, buzzer and OLED module.
- **Does not cover:** firmware behavior (Controller Firmware FSD) or the car (Car Hardware FSD).
- **Status:** Draft 1.0. Values not in the project file are marked **\[?\]**.

## 2. Revisions at a Glance

| Item | Schematic 1 | Schematic 2 | Schematic 3 (newest) |
| --- | --- | --- | --- |
| Power to the module | Boost converter to the 5V pin | HT7833 LDO to the 3V3 pin | HT7833 LDO to the 3V3 pin |
| Extra part near the LDO | none | none | U6 EC-8X12 (likely an electrolytic capacitor) |
| Joystick horizontal (Horz) | GPIO3 | GPIO3 | GPIO7 |
| Joystick press (Sel) | GPIO7 | GPIO7 | GPIO3 |
| Everything else | same | same | same |

The controller firmware matches schematics 1 and 2.

## 3. Hardware Functions

| ID | Requirement |
| --- | --- |
| HC-F1 | The board shall run from one 1S Li-ion/LiPo cell on a 2-pin JST-PH2.0 connector (CN2). |
| HC-F2 | The cell shall be charged through a TP4056 USB module (U4). |
| HC-F3 | A slide switch (SW1) shall switch the load rail `VBAT_SW`. |
| HC-F4 | An HT7833-labelled LDO (U2) shall make the 3.3 V rail for the module. |
| HC-F5 | An ESP32-C3 development board with a 1.3 inch OLED (U1, 16 pins) shall be the MCU and display. |
| HC-F6 | A two-axis analog joystick with a press button (U3) shall provide stick input and the spin button. |
| HC-F7 | Four tactile buttons shall provide Menu/Bind, Select, Down and Up. |
| HC-F8 | A buzzer (SG2) shall be driven by a 2N7000 MOSFET (Q1) from one GPIO. |
| HC-F9 | Four 3 mm holes (J1–J4) shall allow mounting. |

## 4. Parts (Schematic 3)

| Ref | Part | Function | Notes |
| --- | --- | --- | --- |
| U1 | ESP32-C3 Development Board, 1.3 Inch OLED | MCU, USB, OLED | OLED on I2C: SDA GPIO5, SCL GPIO6 |
| U3 | Joystick, part COM-09032 | Two analog axes and a press button |  |
| BT1 | 12 × 12 mm tactile | Menu/Bind |  |
| BT2 | 12 × 12 mm tactile | Select |  |
| BT3 | 12 × 12 mm tactile | Down |  |
| BT4 | 12 × 12 mm tactile | Up |  |
| SG2 | Buzzer | Sound | Driven through Q1 |
| Q1 | 2N7000 | Buzzer driver | Gate from GPIO20 |
| U4 | TP4056 module | Charger | Charge current set on the module |
| SW1 | SS12F23G5 | Power switch | Rating not in the library entry **\[?\]** |
| U2 | HT7833, TO-92 | 3.3 V LDO | See HC-4 |
| U6 | EC-8X12 | Likely an electrolytic capacitor | Value not in the file **\[?\]** |
| CN2 | SH-PH2.0-2PZZ | Battery connector |  |
| J1–J4 | 3 mm holes | Mounting |  |

## 5. Power Path

1. Cell (CN2) → TP4056 module (B+ and B−). The module's OUT+ goes to the slide switch.
2. Slide switch (SW1) → `VBAT_SW`.
3. `VBAT_SW` → HT7833 LDO → `3.3V`.
4. `3.3V` feeds the module's 3V3 pin, the joystick supply and the button and buzzer circuits. The module's 5V pin is not connected in schematics 2 and 3.

There is no battery sense circuit, so the controller cannot measure its own cell.

## 6. Pin Assignments

| Function | Macro in code | Schematics 1 and 2 | Schematic 3 |
| --- | --- | --- | --- |
| Down button | `DOWN_BUTTON_PIN` | GPIO0 | GPIO0 |
| Up button | `UP_BUTTON_PIN` | GPIO1 | GPIO1 |
| Joystick horizontal (analog) | `JOYSTICK_HORZ_PIN` | GPIO3 | **GPIO7** |
| Joystick vertical (analog) | `JOYSTICK_VERT_PIN` | GPIO4 | GPIO4 |
| Joystick press (Sel) | `SEL_BUTTON_PIN` | GPIO7 | **GPIO3** |
| OLED SDA | set in `setup()` | GPIO5 | GPIO5 |
| OLED SCL | set in `setup()` | GPIO6 | GPIO6 |
| Select button | `SELECT_BUTTON_PIN` | GPIO8 | GPIO8 |
| Menu/Bind button | `MENU_BUTTON_PIN` | GPIO10 | GPIO10 |
| Buzzer driver | `PIEZO_PIN` | GPIO20 | GPIO20 |

The ESP32-C3 has analog inputs on GPIO0 to GPIO5 only. In schematics 1 and 2 both joystick axes are on analog pins (GPIO3 and GPIO4). In Schematic 3 the horizontal axis is on GPIO7.

## 7. Observations

Severity is my estimate.

| ID | Severity | Observation |
| --- | --- | --- |
| HC-1 | High | **Schematic 3 cannot read the horizontal joystick axis.** GPIO7 has no analog input on an ESP32-C3. In this controller the horizontal axis is the throttle. The press button works on either pin because it is digital. Safest fix: swap the two nets back to match schematics 1 and 2 (Horz on GPIO3, Sel on GPIO7). This is a board fix first, then a matching firmware revision. Please confirm which schematic you actually built. |
| HC-2 | Medium | **No battery sense.** The controller cannot warn that its own cell is low. The only free analog pin is GPIO2, which is a boot strapping pin, so a divider there needs a check. A later board could add a divider on a different pin. |
| HC-3 | Low | **3.3 V rail has two possible sources.** The LDO feeds the module's 3V3 pin while USB can also power the module. With the switch on and USB plugged in, two regulators feed the same rail. Check the module's documentation, or switch off while on USB. |
| HC-4 | Medium | **Regulator part is unclear.** The symbol says HT7833 (3.3 V) but the part's manufacturer field says "78L05 TO92" (a 5 V part that cannot work from a 1S cell). Check the part actually fitted and its pin order. The same symbol is used on the car. |
| HC-5 | Medium | **Charge current.** TP4056 modules usually default to 1 A, which is 2C for a 500 mAh cell. Check the module's programming resistor ("122" is about 1 A, "242" is about 500 mA). |
| HC-6 | Low | **GPIO8 and GPIO20 are special pins.** GPIO8 (Select) is a boot strapping pin and may also be the module's LED pin. GPIO20 (buzzer) is the UART0 RX pin. Neither is a fault, but both matter if Serial or the LED are used. |
| HC-7 | Low | **OLED type needs confirming.** The firmware drives a 128×32 SSD1306. A 1.3 inch OLED is often 128×64 and may use an SH1106 controller. |
| HC-8 | Low | **Capacitor values are not in the project** (U6 and any decoupling). Check the LDO datasheet for required input and output capacitors. |

## 8. Open Questions

1. Which schematic revision did you actually build (HC-1)?
2. What is the exact OLED module and its controller chip (HC-7)?
3. What is U6, and its value (HC-8)?
4. TP4056 programming resistor marking (HC-5).
5. Do you want battery sense on the next board revision (HC-2)?
