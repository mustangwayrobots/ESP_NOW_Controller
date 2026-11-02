// ==============================================================================
// loopNormalMode.ino - Normal Driving Loop & Connection State Management
// ==============================================================================

// Reference external send-ack watchdog tracker from setupNormalMode.ino
extern unsigned long lastSendAckTime;

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
    
    // WAITING: Low contrast (40) and blink mode 2 (will now toggle every 1 second)
    displayMsg("WAITING", 40, 2);

    // Instead of spamming packets every 20ms while disconnected, 
    // drop transmission attempts to twice a second (500ms) to save massive power.
    static unsigned long lastWaitingSend = 0;
    if (millis() - lastWaitingSend > 500) {
      lastWaitingSend = millis();
      // Send a ping packet to check if the car has come online
      CalculateMotorDriverValues(joystickX, joystickY);
    }

    // Rest longer in the waiting loop (50ms instead of 20ms)
    delay(50); 
    return; // Skip active driving loop overhead while waiting
  } else {
    // Convert millivolts to Volts (e.g., 4100 -> 4.1V)
    float voltage = ReceivingData.battery / 1000.0;

    // Stable two-line format: Line 1 = CONN, Line 2 = Bat=X.X
    char displayBuffer[32];
    snprintf(displayBuffer, sizeof(displayBuffer), "CONN Bat=%.1fV", voltage);
    
    // CONN: Low contrast (40) and solid display (1). 
    // Change '1' to '2' here if you want CONN to blink too!
    displayMsg(displayBuffer, 40, 1);
  }

  // 5. --- Normal Driving Telemetry (Active Connection) ---
  delay(1);     
  CalculateMotorDriverValues(joystickX, joystickY);
  SendData();   
  
  delay(20); 
}