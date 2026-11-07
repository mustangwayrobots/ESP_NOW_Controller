/**
 * ==============================================================================
 * HardwareConfig.h - Multi-Revision Pin & Radio Layout Definitions
 * ==============================================================================
 */
#pragma once

#ifdef HARDWARE_V1
  #define BOARD_VERSION_STR "ESP32-C3 RC Controler V1"
  #define DEBUG_SERIAL_OUTPUT false  // Set to true only when troubleshooting inputs/radio
  #define WIFI_CHANNEL 1
  #define JOYSTICK_HORZ_PIN 3  
  #define JOYSTICK_VERT_PIN 4  
  #define MENU_BUTTON_PIN   10 
  #define SELECT_BUTTON_PIN 8  
  #define SEL_BUTTON_PIN    7  
  #define UP_BUTTON_PIN     1  
  #define DOWN_BUTTON_PIN   0  
  #define PIEZO_PIN         20 
#elif defined(HARDWARE_V2)
  #define BOARD_VERSION_STR "ESP32-C3 RC Controler V2"
  #define DEBUG_SERIAL_OUTPUT false  // Set to true only when troubleshooting inputs/radio
  #define WIFI_CHANNEL 1
  #define JOYSTICK_HORZ_PIN 3  
  #define JOYSTICK_VERT_PIN 4  
  #define MENU_BUTTON_PIN   10 
  #define SELECT_BUTTON_PIN 8  
  #define SEL_BUTTON_PIN    7  
  #define UP_BUTTON_PIN     1  
  #define DOWN_BUTTON_PIN   0  
  #define PIEZO_PIN         20 
#else
  #define BOARD_VERSION_STR "ESP32-C3 RC Controler V3"
  #define DEBUG_SERIAL_OUTPUT false  // Set to true only when troubleshooting inputs/radio
  #define WIFI_CHANNEL 1
  #define JOYSTICK_HORZ_PIN 3  
  #define JOYSTICK_VERT_PIN 4  
  #define MENU_BUTTON_PIN   10 
  #define SELECT_BUTTON_PIN 8  
  #define SEL_BUTTON_PIN    7  
  #define UP_BUTTON_PIN     1  
  #define DOWN_BUTTON_PIN   0  
  #define PIEZO_PIN         20 
#endif