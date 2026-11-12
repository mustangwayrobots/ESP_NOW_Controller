/**
 * ==============================================================================
 * HardwareConfig.h - Board Pin Definitions (the ONLY place pin numbers live)
 * ==============================================================================
 * Pins are named for what they DO, not how the stick is mounted:
 *   THROTTLE pin = the forward/back stick axis
 *   STEER pin    = the left/right stick axis
 *
 * Choosing the board: near the top of ESP_NOW_Controller.ino, set
 *   #define HARDWARE_V1   or   #define HARDWARE_V2
 * A build that passes -DHARDWARE_Vx (like the GitHub flasher build) overrides it.
 * (Software-wide settings and shared variables live in ConfigData.h.)
 *
 * Screen type: each board sets ONE of these, and ESP_NOW_Controller.ino
 * picks the matching display driver:
 *   OLED_SSD1306_128X32   0.91" 128x32 screen
 *   OLED_SH1106_128X64    1.3"  128x64 screen
 */
#pragma once

#if defined(HARDWARE_V1) && defined(HARDWARE_V2)
  #error "Pick only one board: HARDWARE_V1 or HARDWARE_V2"
#endif

#if defined(HARDWARE_V1)
  #define BOARD_VERSION_STR "ESP32-C3 RC Controler V1"
  #define DEBUG_SERIAL_OUTPUT false  // Set to true only when troubleshooting inputs/radio
  #define OLED_SH1106_128X64         // V2 screen type (1.3")
//  #define OLED_SSD1306_128X32        // V1 screen type
  #define OLED_SDA_PIN      5
  #define OLED_SCL_PIN      6
  #define JOYSTICK_THROTTLE_PIN 3    // forward/back
  #define JOYSTICK_STEER_PIN    4    // left/right
  #define MENU_BUTTON_PIN   10
  #define SELECT_BUTTON_PIN 8
  #define SPIN_BUTTON_PIN   7
  #define UP_BUTTON_PIN     1
  #define DOWN_BUTTON_PIN   0
  #define PIEZO_PIN         20

#elif defined(HARDWARE_V2)
  #define BOARD_VERSION_STR "ESP32-C3 RC Controler V2"
  #define DEBUG_SERIAL_OUTPUT false  // Set to true only when troubleshooting inputs/radio
  #define OLED_SH1106_128X64         // V2 screen type (1.3")
  #define OLED_SDA_PIN      5        // CHECK against V2 schematic
  #define OLED_SCL_PIN      6        // CHECK against V2 schematic
  #define JOYSTICK_THROTTLE_PIN 3    // forward/back
  #define JOYSTICK_STEER_PIN    4    // left/right
  #define MENU_BUTTON_PIN   10
  #define SELECT_BUTTON_PIN 8
  #define SPIN_BUTTON_PIN   7
  #define UP_BUTTON_PIN     1
  #define DOWN_BUTTON_PIN   0
  #define PIEZO_PIN         20
#else
  #error "No board chosen. Set the board line at the top of ESP_NOW_Controller.ino."
#endif
