// ==============================================================================
// NVS Storage Helpers for MAC Address Binding
// ==============================================================================

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

void writeStoredMac(const uint8_t* macIn) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           macIn[0], macIn[1], macIn[2], macIn[3], macIn[4], macIn[5]);
           
  preferences.begin("rc-config", false); // Open read-write
  preferences.putString("mac", macStr);
  preferences.end();
}