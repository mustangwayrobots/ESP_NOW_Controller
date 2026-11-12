// ==============================================================================
// loopNormalMode.ino - Normal Driving Loop & Connection State Management
// ==============================================================================

void loopNormalMode() {
  // 1. ALWAYS read inputs first so button states are fresh on every loop cycle
  readInputs();

#if DEBUG_SERIAL_OUTPUT
  // Raw stick readings 5 times a second (set DEBUG_SERIAL_OUTPUT true in HardwareConfig.h)
  static unsigned long lastDebugPrint = 0;
  if (millis() - lastDebugPrint > 200) {
    lastDebugPrint = millis();
    Serial.printf("stick raw: throttle=%d steer=%d\n", stickThrottle, stickSteer);
  }
#endif

  // 2. Check the menu next. If active, run ONLY the menu and stop everything else.
  if (ActiveMenu() == true) {
    delay(10); 
    return;    
  }

  // 3. --- SEND-ACK CONNECTION WATCHDOG ---
  if (millis() - lastSendAckTime > CONNECTION_TIMEOUT_MS) {
    TXConnectionStatus = false;
  } else {
    TXConnectionStatus = true;
  }

  static uint8_t channel = getRadioChannel();
  char chText[8];
  snprintf(chText, sizeof(chText), "CH %d", channel);

  // 4. --- WAITING STATE & AUTO-RECONNECTION ---
  if (TXConnectionStatus == false) {
    Beep(1000, 10000, 100); // 1000Hz tone, 10-second period, 100ms duration
    
    if (carPaired) showStatus("NO SIGNAL", chText, "Waiting", "Turn on the car");
    else           showStatus("NOT PAIRED", chText, "No car", "Hold MENU at power on");

    // Periodically re-add the peer every 2 seconds while waiting, 
    // allowing the controller to connect even if the car is turned on later!
    // Not paired: nothing to talk to, so skip the peer and the sends
    if (!carPaired) {
      delay(1);
      return;
    }

    static unsigned long lastPeerCheck = 0;
    if (millis() - lastPeerCheck > PEER_RECHECK_MS) {
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
    if (millis() - lastWaitingSend > SEND_INTERVAL_WAITING_MS) {
      lastWaitingSend = millis();
      CalculateMotorDriverValues(stickThrottle, stickSteer);
      SendData(); 
    }

    delay(1); 
    return; 
  } else {
    // Car battery, redrawn at most 4 times a second (each redraw takes ~25 ms)
    static unsigned long lastDraw = 0;
    if (millis() - lastDraw > 250) {
      lastDraw = millis();
      char big[12];
      if (ReceivingData.battery > 0) snprintf(big, sizeof(big), "%.2f V", ReceivingData.battery / 1000.0);
      else                           snprintf(big, sizeof(big), "-.-- V");   // no reading yet
      showStatus("CONNECTED", chText, big, "Car battery");
    }
  }

  // 5. --- ADAPTIVE PACKET RATE LOGIC ---
  // Work out what we WOULD send, then use that to decide how often to send.
  // (Raw stick values never read zero, so we judge by the finished motor command.)
  CalculateMotorDriverValues(stickThrottle, stickSteer);

  bool isActive = (SendingData.motor_A1 != 0) || (SendingData.motor_A2 != 0) ||
                  (SendingData.motor_B1 != 0) || (SendingData.motor_B2 != 0) ||
                  UpButton || SpinButton;

  static bool wasActive = false;
  if (wasActive && !isActive) {
    lastSendTime = 0;  // Just let go of the stick: send the STOP packet right now
  }
  wasActive = isActive;

  if (isActive) {
    currentSendInterval = SEND_INTERVAL_ACTIVE_MS;  // Fast 50Hz response when driving
  } else {
    currentSendInterval = SEND_INTERVAL_IDLE_MS;   // Slow idle heartbeat when parked/steady
  }

  // 6. --- TRANSMIT ON ADAPTIVE INTERVAL ---
  if (millis() - lastSendTime >= currentSendInterval) {
    lastSendTime = millis();
    SendData();   
  }
  
  delay(1); 
}
