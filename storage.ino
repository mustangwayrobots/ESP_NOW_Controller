// ==============================================================================
// storage.ino - Everything that reads or writes the ESP32's flash (NVS)
// ==============================================================================
// Two kinds of data are saved under the name "rc-config":
//   set_0 ... set_18   the menu settings (by position, see SettingId in ConfigData.h)
//   mac                the paired car's MAC address

// Load all menu settings into currentSettings[] (factory default if never saved)
void loadSettings() {
  preferences.begin("rc-config", false);
  for (int i = 0; i < MENU_ITEMS; i++) {
    char key[10];
    snprintf(key, sizeof(key), "set_%d", i);
    int v = preferences.getInt(key, defaultSettings[i]);
    if (v < Menu[i].min || v > Menu[i].max) v = defaultSettings[i];  // out of range: use the default
    currentSettings[i] = v;
  }
  preferences.end();
}

// Save currentSettings[] to flash
void saveSettings() {
  preferences.begin("rc-config", false);
  for (int i = 0; i < MENU_ITEMS; i++) {
    char key[10];
    snprintf(key, sizeof(key), "set_%d", i);
    preferences.putInt(key, currentSettings[i]);
  }
  preferences.end();
#if DEBUG_SERIAL_OUTPUT
  Serial.println("Settings saved to NVS successfully.");
#endif
}

// Read the paired car's MAC into macOut (6 bytes). Returns false if none is saved.
bool readStoredMac(uint8_t* macOut) {
  preferences.begin("rc-config", true); // Open read-only
  String macStr = preferences.getString("mac", "");
  preferences.end();

  if (macStr.length() == 17) {
    sscanf(macStr.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &macOut[0], &macOut[1], &macOut[2], &macOut[3], &macOut[4], &macOut[5]);
    return true;
  }
  return false;
}

// Save the paired car's MAC (6 bytes)
void writeStoredMac(const uint8_t* macIn) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           macIn[0], macIn[1], macIn[2], macIn[3], macIn[4], macIn[5]);

  preferences.begin("rc-config", false); // Open read-write
  preferences.putString("mac", macStr);
  preferences.end();
}
