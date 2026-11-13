/**
 * ==============================================================================
 * SendData.ino - Zero-Cost ESP-NOW Transmission Using RAM-Cached MAC
 * ==============================================================================
 */
void SendData() {
  setPacketHeader(SendingData, PKT_DRIVE); // Normal drive packet (Protocol.h)
  
  // Send packet directly using RAM-cached MAC (eliminates flash churn C-02)
  esp_now_send(cachedCarMac, (uint8_t *)&SendingData, sizeof(SendingData));
}