// ==============================================================================
// loopNormalMode.ino - Normal Driving Loop & Connection State Management  
// ==============================================================================

// Reference external send-ack watchdog tracker from setupNormalMode.ino
extern unsigned long lastSendAckTime;

// Reference adaptive rate tracking variables from ESP_NOW_Controller.ino
extern unsigned long lastSendTime;
extern int currentSendInterval;

void loopNormalMode() {
  // 1. ALWAYS read inputs first so button states are fresh on every loop cycle
  readInputs();  

  // 2. Check the menu next. If active, run ONLY the menu and stop everything else.
  if (ActiveMenu() == true) {
    delay(10); 
    return;    
  }

  // 3. --- SEND-ACK CONNECTION WATCHDOG ---
  if (millis() - lastSendAckTime > 1000) {
    TXConnectionStatus = false;
  } else {
    TXConnectionStatus = true;
  }

  // 4. --- WAITING STATE POWER OPTIMIZATION ---
  if (TXConnectionStatus == false) {
    Beep(1000, 10000, 100); // 1000Hz tone, 10-second period, 100ms duration
    
    // WAITING: Low contrast (40) and blink mode 2
    displayMsg("WAITING", 40, 2);

    // Drop transmission attempts to twice a second (500ms) while disconnected to save power
    static unsigned long lastWaitingSend = 0;
    if (millis() - lastWaitingSend > 500) {
      lastWaitingSend = millis();
      CalculateMotorDriverValues(joystickX, joystickY);
    }

    delay(50); 
    return; 
  } else {
    // Convert millivolts to Volts (e.g., 4100 -> 4.1V)
    float voltage = ReceivingData.battery / 1000.0;

    char displayBuffer[32];
    snprintf(displayBuffer, sizeof(displayBuffer), "CONN Bat=%.1fV", voltage);
    displayMsg(displayBuffer, 40, 1);
  }

  // 5. --- ADAPTIVE PACKET RATE LOGIC ---
  // Check if sticks are actively moved outside the center deadzone
  bool isMoving = (abs(joystickX - 2048) > 50) || (abs(joystickY - 2048) > 50);
  
  if (isMoving) {
    currentSendInterval = 20;  // Fast 50Hz response when driving
  } else {
    currentSendInterval = 100; // Slow 10Hz idle heartbeat when parked/steady
  }

  // 6. --- TRANSMIT ON ADAPTIVE INTERVAL ---
  if (millis() - lastSendTime >= currentSendInterval) {
    lastSendTime = millis();
    CalculateMotorDriverValues(joystickX, joystickY);
    SendData();   
  }
  
  delay(1); 
}