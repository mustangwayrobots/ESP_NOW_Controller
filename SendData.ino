// ==============================================================================
// SendData.ino - ESP-NOW Packet Transmission Wrapper
// ==============================================================================
void SendData(){
    SendingData.protocolMode = 1; // Normal Drive Mode
    uint8_t TempData[sizeof(SendingData)];
    memcpy(TempData, &SendingData, sizeof(SendingData));
    
    uint8_t remoteMac[6];
    if (readStoredMac(remoteMac)) {
      esp_now_send(remoteMac, TempData, sizeof(TempData));        
    }
}