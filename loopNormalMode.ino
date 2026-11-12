// ==============================================================================
// loopNormalMode.ino - Normal Driving Loop & Connection State Management
// ==============================================================================

// Reference external send-ack watchdog tracker from setupNormalMode.ino   
extern unsigned long lastSendAckTime;

// Reference adaptive rate tracking variables from ESP_NOW_Controller.ino
extern unsigned long lastSendTime;
extern int currentSendInterval;

// Reference cached car MAC for auto-reconnection peer checks
extern uint8_t cachedCarMac[6];

void loopNormalMode() {
  // 1. ALWAYS read inputs first so button states are fresh on every loop cycle
  readInputs();  

  // 2. Check the menu next. If active, run ONLY the menu and stop everything else.
  if (ActiveMenu() == true) {
    delay(10); 
    return;    
  }

  // 3. --- SEND-ACK CONNECTION WATCHDOG ---
  if (millis() - lastSendAckTime > 500) {
    TXConnectionStatus = false;
  } else {
    TXConnectionStatus = true;
  }

  // 4. --- WAITING STATE & AUTO-RECONNECTION ---
  if (TXConnectionStatus == false) {
    Beep(1000, 10000, 100); // 1000Hz tone, 10-second period, 100ms duration
    
    // WAITING: Low contrast (40) and blink mode 2
    displayMsg("WAITING", 40, 2);

    // Periodically re-add the peer every 2 seconds while waiting, 
    // allowing the controller to connect even if the car is turned on later!
    static unsigned long lastPeerCheck = 0;
    if (millis() - lastPeerCheck > 2000) {
      lastPeerCheck = millis();
      
      esp_now_peer_info_t peerInfo = {};
      memset(&peerInfo, 0, sizeof(peerInfo));
      memcpy(peerInfo.peer_addr, cachedCarMac, 6);
      
      peerInfo.channel = getRadioChannel();
      peerInfo.ifidx   = WIFI_IF_STA;
      peerInfo.encrypt = false;
      
      if (!esp_now_is_peer_exist(cachedCarMac)) {
        esp_now_add_peer(&peerInfo);
      }
    }

    // Drop transmission attempts to 100ms while disconnected to sync with car's timeout
    static unsigned long lastWaitingSend = 0;
    if (millis() - lastWaitingSend > 100) {
      lastWaitingSend = millis();
      CalculateMotorDriverValues(joystickX, joystickY);
      SendData(); 
    }

    delay(1); 
    return; 
  } else {
    // Convert millivolts to Volts (e.g., 4100 -> 4.1V)
    float voltage = ReceivingData.battery / 1000.0;

    char displayBuffer[32];
    snprintf(displayBuffer, sizeof(displayBuffer), "CONN Bat=%.1fV", voltage);
    displayMsg(displayBuffer, 40, 1);
  }

  // 5. --- ADAPTIVE PACKET RATE LOGIC ---
  // Work out what we WOULD send, then use that to decide how often to send.
  // (Raw stick values never read zero, so we judge by the finished motor command.)
  CalculateMotorDriverValues(joystickX, joystickY);

  bool isActive = (SendingData.motor_A1 != 0) || (SendingData.motor_A2 != 0) ||
                  (SendingData.motor_B1 != 0) || (SendingData.motor_B2 != 0) ||
                  UpButton || SpinButton;

  static bool wasActive = false;
  if (wasActive && !isActive) {
    lastSendTime = 0;  // Just let go of the stick: send the STOP packet right now
  }
  wasActive = isActive;

  if (isActive) {
    currentSendInterval = 20;  // Fast 50Hz response when driving
  } else {
    currentSendInterval = 100; // Slow 100ms idle heartbeat when parked/steady
  }

  // 6. --- TRANSMIT ON ADAPTIVE INTERVAL ---
  if (millis() - lastSendTime >= currentSendInterval) {
    lastSendTime = millis();
    SendData();   
  }
  
  delay(1); 
}
