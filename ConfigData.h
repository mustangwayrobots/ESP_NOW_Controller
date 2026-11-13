// ==============================================================================
// ConfigData.h - Everything the whole program shares (software side)
// ==============================================================================
// Two headers, two jobs:
//   HardwareConfig.h = pin numbers for each board revision (hardware facts)
//   ConfigData.h     = this file: settings list, tuning numbers, packet layout,
//                      and the declarations of variables shared between tabs
// Because the Arduino IDE glues all .ino tabs together, a variable defined in one
// tab and used in another only needs to be declared ONCE, here, with "extern".
// ==============================================================================
#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

#include "Protocol.h"   // Packet layout shared with the car (keep both copies identical)

// ------------------------------------------------------------------------------
// Named list of settings. The ORDER here must match the Menu[] table (and the
// defaultSettings[] and menuHelpText[] lists) in ConfigData.ino exactly.
// Use it like this:   currentSettings[SET_MAX_SPEED]
// To add a new setting: add it at the END of this list (before SETTING_COUNT) and
// add one line to each of the three lists in ConfigData.ino. The settings are
// saved in flash by position, so inserting one in the middle would shift every
// saved value after it.
// ------------------------------------------------------------------------------
enum SettingId {
  SET_MAX_SPEED,       // 0
  SET_FORWARD_TRIM,    // 1
  SET_REVERSE_TRIM,    // 2
  SET_ACCEL_RATE,      // 3
  SET_STEER_RATE,      // 4
  SET_STEER_EXPO,      // 5
  SET_SPIN_SPEED,      // 6
  SET_MOTOR_A_DIR,     // 7
  SET_MOTOR_B_DIR,     // 8
  SET_MOTOR_SWAP,      // 9
  SET_WEAPON_ON,       // 10
  SET_WEAPON_OFF,      // 11
  SET_TEST_TRIM,       // 12
  SET_DEADZONE,        // 13
  SET_ZERO_THROTTLE,   // 14 (saved where "ZERO-X" used to be)
  SET_ZERO_STEER,      // 15 (saved where "ZERO-Y" used to be)
  SET_POWER_ON_TUNE,   // 16
  SET_DEFAULTS,        // 17
  SET_INFO_HELP,       // 18
  SETTING_COUNT        // always last: this is the number of settings
};

#define MENU_ITEMS SETTING_COUNT

// ------------------------------------------------------------------------------
// Tuning numbers (change them here, not inside the code)
// ------------------------------------------------------------------------------
const int  STICK_FULL_RANGE         = 1800;  // ADC counts from stick center to full push
const int  DEADZONE_MIN             = 20;    // Smallest deadzone allowed (matches the menu minimum)
const int  MOTOR_MIN_PWM            = 15;    // Motor commands smaller than this are sent as 0
const int  SEND_INTERVAL_ACTIVE_MS  = 20;    // Packet every 20 ms (50 Hz) while driving
const int  SEND_INTERVAL_IDLE_MS    = 50;    // Heartbeat every 50 ms when sticks are parked
const int  SEND_INTERVAL_WAITING_MS = 100;   // Packet rate while the car is not answering
const unsigned long LINK_TIMEOUT_MS       = 650;   // No reply (telemetry) from the car for this long = not linked
const unsigned long CAR_RADIO_TIMEOUT_MS  = 500;   // No radio-level ACK for this long = car is off or out of range
const unsigned long MISMATCH_SHOW_MS      = 2500;  // Show "firmware mismatch" this long after the last mismatched packet
const unsigned long PEER_RECHECK_MS       = 2000;  // How often to re-add the car while waiting
const int  ACCEL_FULL_RAMP_MS       = 1000;  // Stop-to-full-speed time when ACCEL RATE is 0% (100% = no limit)
const int  ACCEL_MAX_DT_MS          = 50;    // Longest time step the ramp will use (so the first packet after a pause still starts gently)
const int  NUM_SONGS                = 94;    // Entries in songs2.h
// (The packet layout, BINDING_CHANNEL and the telemetry rate are in Protocol.h)

struct menuStruct {
  char label[21];
  int min;
  int max;
};

// ------------------------------------------------------------------------------
// Shared variables (each one is created in the tab named on its line)
// ------------------------------------------------------------------------------
// ConfigData.ino
extern RC_Packet_Structure SendingData;
extern menuStruct Menu[];
extern int defaultSettings[];
extern int tempSettings[];
extern int currentSettings[];
extern const char* const menuHelpText[];

// ESP_NOW_Controller.ino
extern bool NormalMode;
extern bool TXConnectionStatus;
extern bool UpButton;
extern bool DownButton;
extern bool SpinButton;
extern bool MenuButtonTriggered;
extern bool SelectButtonTriggered;
extern volatile boolean ReceiveBindingComplete;
extern uint8_t BindingMac[6];
extern uint8_t cachedCarMac[6];     // The paired car's MAC, kept in RAM for fast sending
extern uint8_t broadcastMac[6];
extern int stickThrottle;           // Raw ADC reading, forward/back stick
extern int stickSteer;              // Raw ADC reading, left/right stick
extern unsigned long lastSendTime;
extern int currentSendInterval;

// setupNormalMode.ino
extern volatile unsigned long lastSendAckTime;    // last time the car's radio acknowledged a send
extern volatile unsigned long lastTelemetryTime;  // last time the car replied (0 = never)
extern volatile unsigned long lastMismatchTime;   // last packet from our car in another protocol version (0 = never)
extern volatile int carBatteryMv;                 // car battery from the last telemetry
extern bool carPaired;              // false if no car MAC is saved in flash

// setupBindingMode.ino
extern uint8_t bindDriveChannel;    // drive channel offered to the car while pairing

// MenuHelp.ino
extern boolean InfoModeActive;
extern int InfoItemIndex;

// ------------------------------------------------------------------------------
// Functions defined in a tab that other tabs call before it is defined
// ------------------------------------------------------------------------------
uint8_t getRadioChannel();                    // RadioChannel.ino: channel 1, 6 or 11 from this board's MAC
void    loadSettings();                       // storage.ino
void    saveSettings();                       // storage.ino
bool    readStoredMac(uint8_t* macOut);       // storage.ino
void    writeStoredMac(const uint8_t* macIn); // storage.ino
// Screens.ino
void showStatus(const char* header, const char* right, const char* big, const char* footer);
void drawMenuList(int current);
void drawEdit(int item, int value);
void drawHelp(int index);
void screenInvalidate();
void formatSettingValue(int item, int value, char* buf, size_t len, bool longForm);

#endif
