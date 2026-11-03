/**
 * ==============================================================================
 * setupNormalMode.ino (Controller - Full Debug Trace Integration)
 * ==============================================================================
 */

// Global tracker for outgoing send confirmation watchdog
unsigned long lastSendAckTime = 0;

void setupNormalMode() {
  Serial.begin(115200);
  delay(1000); // Give serial monitor time to catch boot logs
  Serial.println("\n[DEBUG-BOOT] ========================================");
  Serial.println("[DEBUG-BOOT] Controller boot sequence initiated.");
  Serial.println("[INIT-NORM] Entering controller drive configuration setup...");
  
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm); // Set optimized transmit power

  // --- READ SAVED CAR MAC FROM NVS WITH FULL DEBUG TRACE ---
  bool hasValidBinding = false;
  Serial.println("[DEBUG-NVS] Attempting to open preferences namespace 'rc-config'...");
  
  preferences.begin("rc-config", true);
  if (preferences.isKey("mac")) {
    Serial.println("[DEBUG-NVS] Key 'mac' found in NVS.");
    String savedMac = preferences.getString("mac", "");
    Serial.printf("[DEBUG-NVS] Raw string read from NVS: '%s' (Length: %d)\n", savedMac.c_str(), savedMac.length());
    
    memset(BindingMac, 0, 6);
    int matchedBytes = sscanf(savedMac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                              &BindingMac[0], &BindingMac[1], &BindingMac[2],
                              &BindingMac[3], &BindingMac[4], &BindingMac[5]);
                              
    Serial.printf("[DEBUG-NVS] sscanf parsed %d matched bytes.\n", matchedBytes);
    
    if (matchedBytes == 6) {
      if (!(BindingMac[0] == 0 && BindingMac[1] == 0 && BindingMac[2] == 0 && 
            BindingMac[3] == 0 && BindingMac[4] == 0 && BindingMac[5] == 0)) {
        Serial.printf("[INIT-NORM] Loaded Car MAC from NVS successfully: %s\n", savedMac.c_str());
        hasValidBinding = true;
      } else {
        Serial.println("[WARN] Saved car MAC consists entirely of zero bytes!");
      }
    } else {
      Serial.println("[WARN] Saved car MAC failed to parse 6 hex bytes!");
    }
  } else {
    Serial.println("[WARN] Key 'mac' does NOT exist in NVS. (Fresh web flash or uninitialized storage).");
  }
  preferences.end();

  // --- AUTOMATIC DYNAMIC CHANNEL CALCULATION (Matches Car Logic) ---
  uint8_t baseMac[6];
  WiFi.macAddress(baseMac);
  Serial.printf("[DEBUG-WIFI] Controller Base MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
                
  uint8_t calculatedChannel = 1;
  uint8_t macRemainder = baseMac[5] % 3; 
  if (macRemainder == 1)      calculatedChannel = 6;
  else if (macRemainder == 2) calculatedChannel = 11;
  
  Serial.printf("[INIT-NORM] Controller MAC loaded. Calculated Channel -> Channel %d\n", calculatedChannel);

  // --- ESP-NOW INIT ---
  Serial.println("[DEBUG-ESPNOW] Initializing ESP-NOW...");
  if (esp_now_init() != ESP_OK) {
    displayMsg("ESPNOW FAIL");
    Serial.println("[ERR] ESP-NOW initialization crashed in drive mode setup!");
    return;
  }
  Serial.println("[DEBUG-ESPNOW] ESP-NOW initialized successfully.");
  
  WiFi.setSleep(true);
  esp_wifi_set_channel(calculatedChannel, WIFI_SECOND_CHAN_NONE);
 
  // --- REGISTER CAR PEER USING LOADED MAC ---
  if (hasValidBinding) {
    Serial.println("[DEBUG-PEER] Attempting to register stored car peer...");
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
  } else {
    Serial.println("[DEBUG-PEER] Skipping peer registration because no valid binding was found.");
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

  // --- FINAL ROUTING & DIAGNOSTIC NOTICE ---
  if (hasValidBinding) {
    Serial.println("[INIT-NORM] Valid binding found in memory. Bypassing pairing screen.");
    Serial.println("[DEBUG-BOOT] Controller entering normal operational loop.");
  } else {
    Serial.println("==================================================");
    Serial.println("[NOTICE] Fresh deployment detected or NVS was cleared.");
    Serial.println("[NOTICE] ACTION REQUIRED: You must rebind with the car.");
    Serial.println("==================================================");
    displayMsg("WAITING");
  }
  
  Serial.println("[INIT-NORM] Controller normal drive environment initialized successfully.");
  Serial.println("[DEBUG-BOOT] ========================================\n");
}