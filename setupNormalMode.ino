/**
 * ==============================================================================
 * setupNormalMode.ino - Controller Normal Mode Setup & Valid Binding Check (C-05)
 * ==============================================================================
 */
extern uint8_t cachedCarMac[6]; 
unsigned long lastSendAckTime = 0;

void setupNormalMode() {
  Serial.println("[INIT-NORM] Configuring controller normal driving mode...");

  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);

  // Operating channel comes from the one shared function (RadioChannel.ino)
  uint8_t calculatedChannel = getRadioChannel();

  Serial.printf("[INIT-NORM] Controller Channel Allocation -> Channel %d\n", calculatedChannel);

  // Load bound car MAC from NVS preferences
  Preferences prefs;
  prefs.begin("rc-config", true);
  bool hasKey = prefs.isKey("mac");
  String savedMac = prefs.getString("mac", "00:00:00:00:00:00");
  prefs.end();

  // If no stored MAC key exists or it is empty/zero, force fallback to binding mode or alert user
  if (!hasKey || savedMac == "00:00:00:00:00:00") {
    Serial.println("[WARN] No valid car MAC bound in flash! Please pair controller.");
  }

  sscanf(savedMac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &cachedCarMac[0], &cachedCarMac[1], &cachedCarMac[2],
         &cachedCarMac[3], &cachedCarMac[4], &cachedCarMac[5]);

  Serial.printf("[INIT-NORM] Target Car MAC Cached in RAM: %s\n", savedMac.c_str());

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERR] ESP-NOW initialization failed!");
    ESP.restart();
  }

  esp_wifi_set_channel(calculatedChannel, WIFI_SECOND_CHAN_NONE);

  // Register unicast peer
  esp_now_peer_info_t peerInfo = {};
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, cachedCarMac, 6);
  peerInfo.channel = calculatedChannel;
  peerInfo.ifidx   = WIFI_IF_STA;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[WARN] Failed to add car peer or peer already exists.");
  }

  esp_now_register_send_cb([](const esp_now_send_info_t *tx_info, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
      lastSendAckTime = millis();
    }
  });

  esp_now_register_recv_cb([](const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (len >= sizeof(RC_Packet_Structure)) {
      if (memcmp(recv_info->src_addr, cachedCarMac, 6) == 0) {
        memcpy(&ReceivingData, data, sizeof(RC_Packet_Structure));
      }
    }
  });

  Serial.println("[INIT-NORM] Normal drive setup complete.");
}