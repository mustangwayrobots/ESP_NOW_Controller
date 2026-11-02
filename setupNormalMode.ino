/**
 * ==============================================================================
 * setupNormalMode.ino (Controller - Send-Ack Watchdog Integration)
 * ==============================================================================
 */

// Global tracker for outgoing send confirmation watchdog
unsigned long lastSendAckTime = 0;

void setupNormalMode() {
  Serial.println("[INIT-NORM] Entering controller drive configuration setup...");
  
  WiFi.mode(WIFI_STA);

  // --- READ SAVED CAR MAC FROM NVS ---
  preferences.begin("rc-config", true);
  if (preferences.isKey("mac")) {
    String savedMac = preferences.getString("mac", "");
    sscanf(savedMac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &BindingMac[0], &BindingMac[1], &BindingMac[2],
           &BindingMac[3], &BindingMac[4], &BindingMac[5]);
    Serial.printf("[INIT-NORM] Loaded Car MAC from NVS: %s\n", savedMac.c_str());
  } else {
    Serial.println("[WARN] No saved car MAC found in NVS! Using default fallback MAC.");
  }
  preferences.end();

  // --- AUTOMATIC DYNAMIC CHANNEL CALCULATION (Matches Car Logic) ---
  uint8_t baseMac[6];
  WiFi.macAddress(baseMac);
  uint8_t calculatedChannel = 1;
  uint8_t macRemainder = baseMac[5] % 3; 
  if (macRemainder == 1)      calculatedChannel = 6;
  else if (macRemainder == 2) calculatedChannel = 11;
  
  Serial.printf("[INIT-NORM] Controller MAC loaded. Calculated Channel -> Channel %d\n", calculatedChannel);

  if (esp_now_init() != ESP_OK) {
    displayMsg("ESPNOW FAIL");
    Serial.println("[ERR] ESP-NOW initialization crashed in drive mode setup!");
    return;
  }
  WiFi.setSleep(true);
  esp_wifi_set_channel(calculatedChannel, WIFI_SECOND_CHAN_NONE);
 
  // --- REGISTER CAR PEER USING LOADED MAC ---
  esp_now_peer_info_t peerInfo = {};
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, BindingMac, 6);
  peerInfo.channel = calculatedChannel; 
  peerInfo.ifidx   = WIFI_IF_STA;       
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[WARN] Failed to add unicast peer or peer already exists.");
  } else {
    Serial.println("[INIT-NORM] Car peer registered successfully.");
  }

  // --- REGISTER COMMUNICATION CALLBACKS ---
  // Send callback ONLY refreshes the timer on success, preventing single-packet drop jitter
  esp_now_register_send_cb([](const wifi_tx_info_t *info, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
      lastSendAckTime = millis(); 
    }
  });

  // Receive callback exclusively handles incoming telemetry (battery packets every 5s)
  esp_now_register_recv_cb([](const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (len >= sizeof(RC_Packet_Structure)) {
      memcpy(&ReceivingData, data, sizeof(RC_Packet_Structure));
      Serial.printf("[DEBUG-RX] Telemetry battery received: %d mV\n", ReceivingData.battery);
    }
  });

  displayMsg("WAITING");
  Serial.println("[INIT-NORM] Controller normal drive environment initialized successfully.");
}