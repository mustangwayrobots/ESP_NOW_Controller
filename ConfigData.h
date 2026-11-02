// ==============================================================================
// ConfigData.h - Global Menu Parameter Structs, Defaults, and Packet Layouts
// ==============================================================================
#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

#define MENU_ITEMS 19


#define MENU_ITEMS 19



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

#endif