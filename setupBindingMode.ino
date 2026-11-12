/**
 * ==============================================================================
 * setupBindingMode.ino (Controller Binding Setup & Length Validation)
 * ==============================================================================
 * The radio callback below only copies data and raises a flag.
 * Slow work (saving to flash, adding the car as a peer) is done in
 * loopBindingMode.ino, because callbacks run inside the Wi-Fi task and
 * must return quickly.
 */
void setupBindingMode() {
  Serial.println("[INIT-BIND] Configuring controller for pairing mode...");
  
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_19_5dBm); 

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ERR] ESP-NOW initialization failed in binding mode!");
    ESP.restart();
  }
  
  // Binding always occurs on BINDING_CHANNEL (ConfigData.h)
  esp_wifi_set_channel(BINDING_CHANNEL, WIFI_SECOND_CHAN_NONE);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastMac, 6);
  peerInfo.channel = BINDING_CHANNEL;
  peerInfo.ifidx   = WIFI_IF_STA;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // Receive callback: length check (O-08), then remember the first car that answers
  esp_now_register_recv_cb([](const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (len < (int)sizeof(RC_Packet_Structure)) return;   // too short, ignore
    if (ReceiveBindingComplete) return;                   // already have a car

    const RC_Packet_Structure *incoming = (const RC_Packet_Structure*)data;
    if (incoming->protocolMode == 2) {                    // car acknowledgement
      memcpy(BindingMac, recv_info->src_addr, 6);
      ReceiveBindingComplete = true;                      // loop takes it from here
    }
  });
}
