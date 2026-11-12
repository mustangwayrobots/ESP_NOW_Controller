// ==============================================================================
// ConfigData.h - Global Menu Parameter Structs, Defaults, and Packet Layouts
// ==============================================================================
#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

// ------------------------------------------------------------------------------
// Named list of settings. The ORDER here must match the Menu[] table (and the
// defaultSettings[] and menuHelpText[] lists) in ConfigData.ino exactly.
// Use it like this:   currentSettings[SET_MAX_SPEED]
// To add a new setting: add it here (before SETTING_COUNT) and add one line to
// each of the three lists in ConfigData.ino. The compiler checks the counts.
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
  SET_ZERO_X,          // 14
  SET_ZERO_Y,          // 15
  SET_POWER_ON_TUNE,   // 16
  SET_DEFAULTS,        // 17
  SET_INFO_HELP,       // 18
  SETTING_COUNT        // always last: this is the number of settings
};

#define MENU_ITEMS SETTING_COUNT

struct __attribute__((packed)) RC_Packet_Structure {
  uint8_t protocolMode; // 0 = Bind Request, 1 = Normal Drive, 2 = Bind Acknowledgement
  int motor_A1; int motor_A2;
  int motor_B1; int motor_B2;
  int weapon;   int battery;
};

struct menuStruct {
  char label[21];
  int min;
  int max;
};

// Declare globals as extern so they can be shared safely across files
extern RC_Packet_Structure SendingData;
extern RC_Packet_Structure ReceivingData;
extern menuStruct Menu[];
extern int defaultSettings[];
extern int tempSettings[];
extern int currentSettings[];
extern const char* const menuHelpText[];

// Radio channel (1, 6 or 11) worked out from this board's MAC. See RadioChannel.ino
uint8_t getRadioChannel();

#endif
