/**
 * ==============================================================================
 * setupNormalMode.ino (Clean & Lightweight Architecture)  
 * ==============================================================================
 */
unsigned long lastSendAckTime = 0;

void setupNormalMode() {
  Serial.println("[INIT-NORM] Entering normal drive mode setup...");
  
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
//  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR); // Enable Long Range mode

  // --- CHECK IF BINDING MAC IS VALID (Loaded previously by main setup) ---
  bool hasValidBinding = false;
  if (!(BindingMac[0] == 0 && BindingMac[1] == 0 && BindingMac[2] == 0 && 
        BindingMac[3] == 0 && BindingMac[4] == 0 && BindingMac[5] == 0)) {
    hasValidBinding = true;
    Serial.println("[INIT-NORM] Valid binding detected in memory.");
  } else {
    Serial.println("[WARN] No valid binding found! MAC is all zeros.");
  }

  // --- AUTOMATIC DYNAMIC CHANNEL CALCULATION ---
  uint8_t baseMac[6];
  WiFi.macAddress(baseMac);
  uint8_t calculatedChannel = 1;
  uint8_t macRemainder = baseMac[5] % 3; 
  if (macRemainder == 1)      calculatedChannel = 6;
  else if (macRemainder == 2) calculatedChannel = 11;
  
  Serial.printf("[INIT-NORM] Calculated Channel -> Channel %d\n", calculatedChannel);

  if (esp_now_init() != ESP_OK) {
    displayMsg("ESPNOW FAIL");
    Serial.println("[ERR] ESP-NOW initialization failed!");
    return;
  }
  
  WiFi.setSleep(true);
  esp_wifi_set_channel(calculatedChannel, WIFI_SECOND_CHAN_NONE);
 
  // --- REGISTER PEER ONLY IF BOUND ---
  if (hasValidBinding) {
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
  }

  // --- REGISTER CALLBACKS ---
  esp_now_register_send_cb([](const wifi_tx_info_t *info, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
      lastSendAckTime = millis(); 
    }
  });

  esp_now_register_recv_cb([](const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (len >= sizeof(RC_Packet_Structure)) {
      memcpy(&ReceivingData, data, sizeof(RC_Packet_Structure));
#if DEBUG_SERIAL_OUTPUT
      Serial.printf("[DEBUG-RX] Telemetry received: %d mV\n", ReceivingData.battery);
#endif
    }
  });

  // --- DISPLAY ROUTING & FRESH FLASH NOTICE ---
  if (hasValidBinding) {
    Serial.println("[INIT-NORM] Drive mode active. Bypassing pairing screen.");
  } else {
    displayMsg("WAITING");
    Serial.println("==================================================");
    Serial.println("[NOTICE] Fresh web flash detected: NVS was cleared.");
    Serial.println("[NOTICE] ACTION REQUIRED: Please rebind with the car.");
    Serial.println("==================================================");
  }
}