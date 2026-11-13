/**
 * ==============================================================================
 * loopBindingMode.ino - Controller Binding Loop with Verified Handshake Confirmation
 * ==============================================================================
 */

const unsigned long BIND_TIMEOUT_MS   = 20000; // give up if no car answers
const unsigned long BIND_SEND_MS      = 200;   // bind request repeat
const unsigned long CONFIRM_SEND_MS   = 100;   // confirmation repeat
const unsigned long CONFIRM_TIME_MS   = 1500;  // keep confirming this long, then reboot

// Runs once, right after the car answers: save its MAC and add it as a peer.
// Done here (not in the radio callback) because flash writes are slow.
void finishBinding() {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           BindingMac[0], BindingMac[1], BindingMac[2],
           BindingMac[3], BindingMac[4], BindingMac[5]);
  Serial.printf("[COMM-BIND] Received car acknowledgement from: %s\n", macStr);

  // Only write flash if this is a different car (saves flash wear)
  uint8_t oldMac[6];
  if (readStoredMac(oldMac) && memcmp(oldMac, BindingMac, 6) == 0) {
    Serial.println("[NVS] Same car as before, no flash write needed.");
  } else {
    writeStoredMac(BindingMac);
    Serial.println("[NVS] Car MAC successfully committed to flash.");
  }

  esp_now_peer_info_t carPeer = {};
  memcpy(carPeer.peer_addr, BindingMac, 6);
  carPeer.channel = BINDING_CHANNEL;
  carPeer.ifidx   = WIFI_IF_STA;
  carPeer.encrypt = false;
  esp_now_add_peer(&carPeer);
}

void loopBindingMode() {
  static unsigned long lastBindSend = 0;
  static unsigned long lastConfirmSend = 0;
  static unsigned long bindStartTime = millis();
  static unsigned long ackTime = 0; // when the car answered
  
  readInputs();

  // Wait for MENU release; the binding window starts after release
  if (digitalRead(MENU_BUTTON_PIN) == LOW) { 
    showStatus("PAIRING", "", "Release", "Let go of MENU");
    bindStartTime = millis();
    delay(50);
    return; 
  }

  // Failsafe timeout. Old pairing stays saved, controller restarts normally.
  if (!ReceiveBindingComplete && millis() - bindStartTime > BIND_TIMEOUT_MS) {
    Serial.println("[ERR] Binding timed out. Restarting...");
    showStatus("PAIRING", "", "Timed out", "Old car is kept");
    delay(1500);
    ESP.restart();
  }

  // Step 1: Broadcast bind request until a car acknowledges
  if (!ReceiveBindingComplete) {
    char footer[24];
    unsigned long secondsLeft = (BIND_TIMEOUT_MS - (millis() - bindStartTime)) / 1000 + 1;
    snprintf(footer, sizeof(footer), "Switch car on  %lus", secondsLeft);
    showStatus("PAIRING", "CH 1", "Searching", footer);
    if (millis() - lastBindSend > BIND_SEND_MS) {
      lastBindSend = millis();
      RC_Packet_Structure bindPacket;
      memset(&bindPacket, 0, sizeof(bindPacket));
      setPacketHeader(bindPacket, PKT_BIND_REQUEST, bindDriveChannel);
      esp_now_send(broadcastMac, (uint8_t *)&bindPacket, sizeof(bindPacket));
    }
  } 
  // Step 2: Car acknowledged! Send the confirmation, then reboot
  else {
    if (ackTime == 0) {
      ackTime = millis();
      finishBinding();
    }
    char footer[24];
    snprintf(footer, sizeof(footer), "Car ..:%02x:%02x", BindingMac[4], BindingMac[5]);
    showStatus("PAIRING", "CH 1", "Paired!", footer);

    if (millis() - lastConfirmSend > CONFIRM_SEND_MS) {
      lastConfirmSend = millis();
      RC_Packet_Structure confirmPacket;
      memset(&confirmPacket, 0, sizeof(confirmPacket));
      setPacketHeader(confirmPacket, PKT_BIND_CONFIRM, bindDriveChannel); // Verified confirmation (O-04)
      esp_now_send(BindingMac, (uint8_t *)&confirmPacket, sizeof(confirmPacket));
    }

    if (millis() - ackTime > CONFIRM_TIME_MS) {
      Serial.println("[SUCCESS] Handshake verified! Rebooting into Normal Mode...");
      delay(500);
      ESP.restart();
    }
  }

  delay(50);
}
