// ==============================================================================
// RadioChannel.ino - The one place that decides which Wi-Fi channel we use
// ==============================================================================
// Spreads controllers over 3 non-overlapping channels (1, 6, 11) using the last
// byte of this board's MAC address, so a group of cars/controllers don't all
// crowd the same channel. Call getRadioChannel() anywhere a channel is needed.

uint8_t getRadioChannel() {
  uint8_t myMac[6];
  WiFi.macAddress(myMac);

  switch (myMac[5] % 3) {
    case 1:  return 6;
    case 2:  return 11;
    default: return 1;
  }
}
