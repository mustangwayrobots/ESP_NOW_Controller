/**
 * ==============================================================================
 * loopBindingMode.ino - Controller Binding Loop with Release Button Prompt
 * ==============================================================================
 */
void loopBindingMode() {
  static unsigned long lastBindSend = 0;
  static unsigned long lastDebugStatus = 0;
  
  // 1. Always refresh inputs on every loop cycle to catch button releases
  readInputs();

  // 2. If the user is still holding down the menu button at boot, prompt them to let go
  if (MenuButton == true) { // MenuButton is true (LOW) when pressed
    displayMsg("RELEASE BTN", 100, 1);
    delay(50);
    return; // Stay here and skip broadcasting until the button is released
  }

  // 3. Once released, proceed with active binding display and broadcasts
  displayMsg("BINDING...", 100, 2);

  if (millis() - lastDebugStatus > 1000) {
    lastDebugStatus = millis();
    Serial.printf("[LOOP-BIND] Controller searching... ReceiveBindingComplete flag = %d\n", ReceiveBindingComplete);
  }

  if (millis() - lastBindSend > 200) {
    lastBindSend = millis();
    
    RC_Packet_Structure bindPacket;
    memset(&bindPacket, 0, sizeof(bindPacket));
    bindPacket.protocolMode = 0; 
    
    Serial.println("[BIND] Broadcasting pairing request to car...");
    esp_now_send(broadcastMac, (uint8_t *)&bindPacket, sizeof(bindPacket));
  }

  if (ReceiveBindingComplete) {
    Serial.println("[SUCCESS] Binding handshake confirmed by car! Rebooting into Normal Mode...");
    delay(1000);
    ESP.restart();
  }

  delay(50);
}