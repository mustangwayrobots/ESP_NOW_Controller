/**
 * ==============================================================================
 * ConfigData.ino - Menu Definitions, Default Values, and Help Text Strings
 * ==============================================================================
 */

// Menu Items Table (Index matching header declaration)
menuStruct Menu[] = {
  {"Max speed",     0, 100},            // 0  Max Speed (0-100%)
  {"Fwd trim",    -50, 50},             // 1  Forward Trim
  {"Rev trim",    -50, 50},             // 2  Reverse Trim
  {"Accel rate",    0, 100},            // 3  Accel Rate (0-100%)
  {"Steer rate",    0, 100},            // 4  Steering Rate (0-100%)
  {"Steer expo",    0, 100},            // 5  Steering Expo (0-100%)
  {"Spin speed",    0, 100},            // 6  Spin-on-Demand Speed (0-100%)
  {"Motor A dir",   0, 1},              // 7  Motor A Normal/Reverse
  {"Motor B dir",   0, 1},              // 8  Motor B Normal/Reverse
  {"Swap motors",   0, 1},              // 9  Swap Motor Channels
  {"Weapon on",     0, 180},            // 10 Weapon servo angle, UP held (deg)
  {"Weapon off",    0, 180},            // 11 Weapon servo angle, UP released (deg)
  {"Trim test",     1, 10},             // 12 Test Trim Duration (sec) & Trigger
  {"Deadzone",     20, 150},            // 13 Joystick Center Deadzone
  {"Throttle zero", 0, 4095},           // 14 Forward/back stick center (raw ADC)
  {"Steer zero",    0, 4095},           // 15 Left/right stick center (raw ADC)
  {"Startup tune",  0, NUM_SONGS - 1},  // 16 Startup Melody Selection
  {"Factory reset", 0, 1},              // 17 Reset to Factory Defaults
  {"Help",          0, 0}               // 18 Help pages
};

// Compile-time safety check: the Menu table must have one row per SettingId in ConfigData.h
static_assert(sizeof(Menu) / sizeof(Menu[0]) == MENU_ITEMS, "Menu[] rows do not match the SettingId list in ConfigData.h");

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
  2048, // THROTTLE ZERO (Default ADC Midpoint)
  2048, // STEER ZERO (Default ADC Midpoint)
  0,    // POWER ON TUNE
  0,    // DEFAULTS
  0     // INFO / HELP
};

// Runtime arrays
int currentSettings[MENU_ITEMS];
int tempSettings[MENU_ITEMS];

// Help Screen Text Array
// Help text: up to about 75 characters each (4 lines of 21 on the screen)
const char* const menuHelpText[] = {
  "Limits the car's top speed. 100% is full power.",
  "Fixes drift going forward. + slows left wheel, - slows right wheel.",
  "Fixes drift in reverse. + slows left wheel, - slows right wheel.",
  "Soft start. Lower is a gentler take-off. 100% is instant.",
  "How strongly the car turns. Lower gives gentler turns.",
  "Softens steering near center for fine control. 0 is linear.",
  "Speed when spinning in place with the stick button held.",
  "Reverses motor A if it runs the wrong way.",
  "Reverses motor B if it runs the wrong way.",
  "Swaps left and right motors if steering is mirrored.",
  "Weapon servo angle while UP is held.",
  "Weapon servo angle when UP is released.",
  "Test length. Saving drives forward, then back, to check trim.",
  "Ignores small stick movement at center. Raise it if the car creeps.",
  "Stick center, forward/back. Leave the stick centered and press SEL twice.",
  "Stick center, left/right. Leave the stick centered and press SEL twice.",
  "Tune played at power on. Off for silence.",
  "Puts all settings back to factory values. Pairing is kept.",
  "UP/DN to page through help. SEL or MENU to go back."
};

// --- ESP-NOW Global Packet Instances ---
RC_Packet_Structure SendingData;
RC_Packet_Structure ReceivingData;
