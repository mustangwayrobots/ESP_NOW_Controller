/**
 * ==============================================================================
 * setupBindingMode.ino (Controller Binding Setup & Length Validation)
 * ==============================================================================
 */
void setupBindingMode() {
  Serial.println("[INIT-BIND] Configuring controller for pairing mode...");
  
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_19_5dBm); 

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERR] ESP-NOW initialization failed in binding mode!");
    ESP.restart();
  }
  
  // Binding always occurs on channel 1
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  esp_now_peer_info_t peerInfo = {};
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastMac, 6);
  peerInfo.channel = 1;
  peerInfo.ifidx   = WIFI_IF_STA;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // Receive callback with strict packet length check (O-08)
  esp_now_register_recv_cb([](const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (len >= sizeof(RC_Packet_Structure)) {
      RC_Packet_Structure *incoming = (RC_Packet_Structure*)data;
      if (incoming->protocolMode == 2 && !ReceiveBindingComplete) {
        memcpy(BindingMac, recv_info->src_addr, 6);
        ReceiveBindingComplete = true;

        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
                 BindingMac[0], BindingMac[1], BindingMac[2],
                 BindingMac[3], BindingMac[4], BindingMac[5]);
        
        Serial.printf("[COMM-BIND] Received car acknowledgement from: %s\n", macStr);

        Preferences prefs;
        prefs.begin("rc-config", false);
        prefs.putString("mac", macStr);
        prefs.end();
        Serial.println("[NVS] Car MAC successfully committed to flash.");

        esp_now_peer_info_t carPeer = {};
        memcpy(carPeer.peer_addr, BindingMac, 6);
        carPeer.channel = 1;
        carPeer.ifidx   = WIFI_IF_STA;
        carPeer.encrypt = false;
        esp_now_add_peer(&carPeer);
      }
    }
  });

  esp_now_register_send_cb([](const esp_now_send_info_t *tx_info, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
      SendBindingComplete = true;
    }
  });
}