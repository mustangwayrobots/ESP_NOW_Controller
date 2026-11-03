/**
 * ==============================================================================
 * setupNormalMode.ino (Controller - Send-Ack Watchdog Integration & Persistent Binding Fix)
 * ==============================================================================
 */

// Global tracker for outgoing send confirmation watchdog
unsigned long lastSendAckTime = 0;

// Forward declaration for your main operational mode transition if needed
void enterOperationalMode();

void setupNormalMode() {
  Serial.println("[INIT-NORM] Entering controller drive configuration setup...");
  
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm); // Set optimized transmit power

  // --- READ SAVED CAR MAC FROM NVS ---
  bool hasValidBinding = false;
  preferences.begin("rc-config", true);
  if (preferences.isKey("mac")) {
    String savedMac = preferences.getString("mac", "");
    if (savedMac.length() == 17 && savedMac.indexOf(':') == 2 && savedMac != "00:00:00:00:00:00") {
      sscanf(savedMac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
             &BindingMac[0], &BindingMac[1], &BindingMac[2],
             &BindingMac[3], &BindingMac[4], &BindingMac[5]);
      Serial.printf("[INIT-NORM] Loaded Car MAC from NVS: %s\n", savedMac.c_str());
      hasValidBinding = true;
    } else {
      Serial.println("[WARN] Saved car MAC in NVS is malformed or empty!");
    }
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
  esp_now_register_send_cb([](const wifi_tx_info_t *info, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
      lastSendAckTime = millis(); 
    }
  });

  // Receive callback exclusively handles incoming telemetry (battery packets)
  esp_now_register_recv_cb([](const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (len >= sizeof(RC_Packet_Structure)) {
      memcpy(&ReceivingData, data, sizeof(RC_Packet_Structure));
#if DEBUG_SERIAL_OUTPUT
      Serial.printf("[DEBUG-RX] Telemetry battery received: %d mV\n", ReceivingData.battery);
#endif
    }
  });

  // --- CONDITIONAL STATE CHECK ---
  if (hasValidBinding) {
    Serial.println("[INIT-NORM] Valid binding found in memory. Bypassing pairing screen.");
    // If you have a specific operational transition function, call it here:
    // enterOperationalMode();
  } else {
    Serial.println("[INIT-NORM] No valid binding found. Entering waiting/pairing state.");
    displayMsg("WAITING");
  }
  
  Serial.println("[INIT-NORM] Controller normal drive environment initialized successfully.");
}