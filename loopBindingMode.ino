/**
 * ==============================================================================
 * loopBindingMode.ino - Controller Binding Loop with Verified Handshake Confirmation
 * ==============================================================================
 */
void loopBindingMode() {
  static unsigned long lastBindSend = 0;
  static unsigned long lastConfirmSend = 0;
  static unsigned long bindStartTime = millis();
  static unsigned long ackTime = 0; // when the car answered
  
  readInputs();

  if (digitalRead(MENU_BUTTON_PIN) == LOW) { 
    displayMsg("RELEASE BTN", 100, 1);
    delay(50);
    return; 
  }

  // Failsafe timeout: 20 seconds max binding window
  if (!ReceiveBindingComplete && millis() - bindStartTime > 20000) {
    Serial.println("[ERR] Binding timed out. Restarting...");
    ESP.restart();
  }

  // Step 1: Broadcast bind request (mode 0) until car acknowledges
  if (!ReceiveBindingComplete) {
    displayMsg("BINDING...", 100, 2);
    if (millis() - lastBindSend > 200) {
      lastBindSend = millis();
      RC_Packet_Structure bindPacket;
      memset(&bindPacket, 0, sizeof(bindPacket));
      bindPacket.protocolMode = 0; 
      esp_now_send(broadcastMac, (uint8_t *)&bindPacket, sizeof(bindPacket));
    }
  } 
  // Step 2: Car acknowledged! Blast mode-3 confirmation until the car exits
  else {
    displayMsg("PAIRED!", 100, 1);
    if (ackTime == 0) ackTime = millis();
    if (millis() - lastConfirmSend > 100) {
      lastConfirmSend = millis();
      RC_Packet_Structure confirmPacket;
      memset(&confirmPacket, 0, sizeof(confirmPacket));
      confirmPacket.protocolMode = 3; // Verified confirmation (O-04)
      esp_now_send(BindingMac, (uint8_t *)&confirmPacket, sizeof(confirmPacket));
    }

    // Keep sending for 1.5 seconds after the car answered, then reboot
    if (millis() - ackTime > 1500) {
      Serial.println("[SUCCESS] Handshake verified! Rebooting into Normal Mode...");
      delay(500);
      ESP.restart();
    }
  }

  delay(50);
}