/**
 * ==============================================================================
 * setupBindingMode.ino (Controller - Instrumented Diagnostic Layout)
 * ==============================================================================
 */
void setupBindingMode() {
  Serial.println("[INIT-BIND] Configuring controller radio layers for pairing...");
  
  WiFi.mode(WIFI_STA);
  Serial.print("[INIT-BIND] Controller MAC: "); Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERR] ESP-NOW initialization failed in binding mode!");
    ESP.restart();
  }
  
  esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE); // Channel 1 for pairing
  Serial.println("[INIT-BIND] ESP-NOW initialized successfully on Station Channel 1.");

  uint8_t broadTarget[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadTarget, 6);
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.ifidx   = WIFI_IF_STA;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // Deep diagnostic receive callback to catch ALL incoming traffic during binding
  esp_now_register_recv_cb([](const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    const uint8_t *src = recv_info->src_addr;
    Serial.printf("[DEBUG-RX] Packet heard! Length: %d bytes from %02X:%02X:%02X:%02X:%02X:%02X\n", 
                  len, src[0], src[1], src[2], src[3], src[4], src[5]);

    if (len >= sizeof(RC_Packet_Structure)) {
      RC_Packet_Structure *incoming = (RC_Packet_Structure*)data;
      Serial.printf("[DEBUG-RX] -> Parsed protocolMode: %d\n", incoming->protocolMode);

      if (incoming->protocolMode == 2 && !ReceiveBindingComplete) {
        ReceiveBindingComplete = true;
        memcpy(&ReceivingData, data, sizeof(ReceivingData));
        
        char carMacStr[18];
        snprintf(carMacStr, sizeof(carMacStr), "%02x:%02x:%02x:%02x:%02x:%02x",
                 src[0], src[1], src[2], src[3], src[4], src[5]);
                 
        preferences.begin("rc-config", false);
        preferences.putString("mac", carMacStr);
        preferences.end();
        
        Serial.printf("[COMM-BIND] SUCCESS! Car MAC Whitelisted and Saved to NVS: %s\n", carMacStr);
      }
    } else {
      Serial.println("[DEBUG-RX] -> Warning: Incoming packet is smaller than RC_Packet_Structure layout!");
    }
  });  

  esp_now_register_send_cb([](const wifi_tx_info_t *info, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
      SendBindingComplete = true; 
    }
  });
  
  Serial.println("[INIT-BIND] Binding setup complete. Ready to broadcast pairing requests.");
}