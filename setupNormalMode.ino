/**
 * ==============================================================================
 * setupNormalMode.ino - Controller Normal Mode Setup & Valid Binding Check (C-05)
 * ==============================================================================
 * Two different "the car heard us" signals:
 *   lastSendAckTime   the car's RADIO acknowledged a packet. Proves the car is
 *                     on and in range, but not that it is listening to us.
 *   lastTelemetryTime the car REPLIED with telemetry. It only does that while
 *                     it accepts our drive packets, so this is the real link (C-25).
 */
volatile unsigned long lastSendAckTime   = 0;
volatile unsigned long lastTelemetryTime = 0;
volatile unsigned long lastMismatchTime  = 0;
volatile int carBatteryMv = 0;
bool carPaired = false;

void setupNormalMode() {
  Serial.println("[INIT-NORM] Configuring controller normal driving mode...");

  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);

  // Operating channel comes from the one shared function (RadioChannel.ino)
  uint8_t calculatedChannel = getRadioChannel();

  Serial.printf("[INIT-NORM] Controller Channel Allocation -> Channel %d\n", calculatedChannel);

  // Load the paired car's MAC from flash (storage.ino)
  carPaired = readStoredMac(cachedCarMac);
  if (!carPaired) {
    Serial.println("[WARN] No valid car MAC bound in flash! Please pair controller.");
  }
  Serial.printf("[INIT-NORM] Target Car MAC Cached in RAM: %02x:%02x:%02x:%02x:%02x:%02x\n",
                cachedCarMac[0], cachedCarMac[1], cachedCarMac[2],
                cachedCarMac[3], cachedCarMac[4], cachedCarMac[5]);

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
  
  if (carPaired && esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[WARN] Failed to add car peer or peer already exists.");
  }

  esp_now_register_send_cb([](const esp_now_send_info_t *tx_info, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
      lastSendAckTime = millis();
    }
  });

  // Receive callback: sender check, version and size check (Protocol.h), type check (C-26)
  esp_now_register_recv_cb([](const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (memcmp(recv_info->src_addr, cachedCarMac, 6) != 0) return;   // not our car
    unsigned long now = millis();
    if (!isCurrentProtocol(data, len)) {                             // our car, other firmware
      lastMismatchTime = now ? now : 1;                              // 0 means "never"
      return;
    }
    const RC_Packet_Structure *incoming = (const RC_Packet_Structure*)data;
    if (incoming->packetType != PKT_TELEMETRY) return;
    carBatteryMv = incoming->battery;
    lastTelemetryTime = now ? now : 1;
  });

  Serial.println("[INIT-NORM] Normal drive setup complete.");
}