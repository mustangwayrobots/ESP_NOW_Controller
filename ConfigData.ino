/**
 * ==============================================================================
 * ConfigData.ino - Menu Definitions, Default Values, and Help Text Strings
 * ==============================================================================
 */

// Menu Items Table (Index matching header declaration)
menuStruct Menu[] = {
  {"MAX SPEED", 0, 100},              // Index 0: Max Speed (0-100%)
  {"FORWARD TRIM", -50, 50},          // Index 1: Forward Trim
  {"REVERSE TRIM", -50, 50},          // Index 2: Reverse Trim
  {"ACCEL RATE", 0, 100},             // Index 3: Accel Rate (0-100%)
  {"STEER RATE", 0, 100},             // Index 4: Steering Rate (0-100%)
  {"STEER EXPO", 0, 100},             // Index 5: Steering Expo (0-100%)
  {"SPIN SPEED", 0, 100},             // Index 6: Spin-on-Demand Speed (0-100%)
  {"MOTOR \"A\" DIRECTION", 0, 1},    // Index 7: Motor A Normal/Reverse
  {"MOTOR \"B\" DIRECTION", 0, 1},    // Index 8: Motor B Normal/Reverse
  {"MOTOR A-B RIGHT-LEFT", 0, 1},     // Index 9: Swap Motor Channels
  {"WEAPON ON VALUE", 0, 180},        // Index 10: Weapon Servo/Actuator On Angle (deg)
  {"WEAPON OFF VALUE", 0, 180},       // Index 11: Weapon Servo/Actuator Off Angle (deg)
  {"TEST TRIM", 1, 10},               // Index 12: Test Trim Duration (sec) & Trigger
  {"JOYSTICK DEADZONE", 0, 150},      // Index 13: Joystick Center Deadzone
  {"JOYSTICK ZERO-X", 0, 4095},       // Index 14: Raw ADC X Center Calibration
  {"JOYSTICK ZERO-Y", 0, 4095},       // Index 15: Raw ADC Y Center Calibration
  {"POWER ON TUNE", 0, 93},           // Index 16: Startup Melody Selection (0-93 for 94 entries)
  {"DEFAULTS", 0, 1},                 // Index 17: Reset to Factory Defaults
  {"INFO / HELP", 0, 0}               // Index 18: Help Screen Viewer
};

// Factory Default Settings Array
int defaultSettings[MENU_ITEMS] = {
  100,  // MAX SPEED (%)
  0,    // FORWARD TRIM
  0,    // REVERSE TRIM
  50,   // ACCEL RATE (%)
  40,  // STEER RATE (%)
  60,    // STEER EXPO (%)
  100,  // SPIN SPEED (%)
  0,    // MOTOR A DIRECTION (Normal)
  0,    // MOTOR B DIRECTION (Normal)
  0,    // MOTOR A-B SWAP (Normal)
  180,  // WEAPON ON VALUE (deg)
  0,    // WEAPON OFF VALUE (deg)
  3,    // TEST TRIM (sec)
  20,   // JOYSTICK DEADZONE
  2048, // JOYSTICK ZERO-X (Default ADC Midpoint)
  2048, // JOYSTICK ZERO-Y (Default ADC Midpoint)
  0,    // POWER ON TUNE
  0,    // DEFAULTS
  0     // INFO / HELP
};

// Runtime arrays
int currentSettings[MENU_ITEMS];
int tempSettings[MENU_ITEMS];

// Help Screen Text Array
const char* const menuHelpText[] = {
  "MAX SPEED: Limits top motor speed %.",
  "FORWARD TRIM: Adjust straight-line drift.",
  "REVERSE TRIM: Adjust straight-line drift.",
  "ACCEL RATE: Controls throttle acceleration.",
  "STEER RATE: Scales overall steering.",
  "STEER EXPO: Softens stick response.",
  "SPIN SPEED: Sets Spin On Joystick Press.",
  "MOTOR A DIR: Reverses physical wiring.",
  "MOTOR B DIR: Reverses physical wiring.",
  "MOTOR SWAP: Swaps physical motor outputs.",
  "WEAPON ON: Active Servo angle (deg).",
  "WEAPON OFF: Idle Servo angle (deg).",
  "TEST TRIM: Timer to run Fwd & REV.",
  "DEADZONE: Ignores stick drift at center.",
  "ZERO-X: Calibrates vert. joystick center value.",
  "ZERO-Y: Calibrates horiz. joystick center value.",
  "STARTUP TUNE: Power on Melody (0-93).",
  "DEFAULTS: Restores factory defaults.",
  "INFO/HELP: Menu Options Definitions."
};

// --- ESP-NOW Global Packet Instances ---
RC_Packet_Structure SendingData;
RC_Packet_Structure ReceivingData;

// --- Name-Based Setting Lookup Helper ---
int getSetting(const char* labelName) {
  for (int i = 0; i < MENU_ITEMS; i++) {
    if (strcmp(Menu[i].label, labelName) == 0) {
      return currentSettings[i];
    }
  }
  return 0; // Fallback if not found
}