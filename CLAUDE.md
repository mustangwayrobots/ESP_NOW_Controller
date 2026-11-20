# CLAUDE.md

This file gives Claude Code the rules and commands for this repository.

ESP NOW Controller: firmware for the Mustang Way Robots handheld controller for the ESP NOW CAR (ESP32-C3, 1.3" SH1106 OLED, two-axis joystick, five buttons, piezo, 1S 500 mAh LiPo).
This folder is the clone of github.com/mustangwayrobots/ESP_NOW_Controller (the club's account; the user, hogfanboy, works on it as a collaborator).
Pushing to `main` makes GitHub Actions build every board version and publish the web flasher to https://mustangwayrobots.github.io/ESP_NOW_Controller/flash/ , which students use. Treat `main` as live.

**Sister repo:** the car is `../ESP_NOW_CAR` (github.com/mustangwayrobots/ESP_NOW_CAR).

## Docs (one job each)
- `README.md`: for students and end users (what it is, flashing, pairing, screens, building).
- `docs/Controller_Firmware_FSD.md` and `docs/Controller_Hardware_FSD.md`: the spec and settled truth. Read only the part you need.
- `CHANGELOG.md`: user-facing changes by date, newest first. Add an entry for any change students would notice.
- `CLAUDE.md`: permanent rules and commands only.

## Rules
- `Protocol.h` must stay byte-for-byte identical to `../ESP_NOW_CAR/Protocol.h`. If you change it, change both, bump the protocol version, and note it in both CHANGELOGs.
- Pin numbers live only in `HardwareConfig.h`; software-wide settings and shared variables live in `ConfigData.h`. The board is chosen with the `#define HARDWARE_Vx` line at the top of `ESP_NOW_Controller.ino` (V1, V2). CI finds the boards by scanning `HardwareConfig.h`.
- Web flashing must not erase the saved pairing or settings: keep the separate-pieces flashing and `new_install_prompt_erase: true` in `.github/workflows/build.yml`.
- `build.yml` and `flash/index.html` are shared, generic copies used by all Mustang Way Robots projects. Keep them identical across repos.
- `libraries/U8g2` is a pinned copy (2.36.19) for CI. Don't read it unless asked; don't edit it.
- Each menu item has a help page (`MenuHelp.ino`); keep them in step when adding or changing settings.
- Commit as hogfanboy (noreply), never a Gmail address. Don't push to `main` unless the user asks.

## Build (same toolchain as CI: esp32 core 3.3.12, board MakerGO ESP32 C3 SuperMini, U8g2 2.36.19)
```bash
A="/c/Users/hogfa/AppData/Local/Programs/arduino-cli/arduino-cli.exe"   # not on PATH
"$A" compile --fqbn esp32:esp32:makergo_c3_supermini --libraries ./libraries --build-property "compiler.cpp.extra_flags=-DHARDWARE_V2" --build-property "compiler.c.extra_flags=-DHARDWARE_V2" --build-path build/v2 .
```
This PC has esp32 core 3.3.13 installed (CI pins 3.3.12); both compile. `build/` is git-ignored. The user uploads to boards from Arduino IDE or the web flasher.
