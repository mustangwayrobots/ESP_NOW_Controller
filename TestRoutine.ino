// ==============================================================================
// TestRoutine.ino - Automated Trim and Motor Testing Routine
// ==============================================================================
// Drives straight forward, pauses, then straight back, so you can check the
// FORWARD TRIM / REVERSE TRIM settings. It "moves the stick" in software by
// feeding CalculateMotorDriverValues() a position relative to the calibrated
// stick center (ZERO-X = forward/back axis, ZERO-Y = left/right axis).

const int TEST_FULL_STICK = 1800;   // Same full-deflection size MotorControl.ino uses

void runtest(){
  char buf2[20];
  int testSeconds = currentSettings[SET_TEST_TRIM];
  int centerX = currentSettings[SET_ZERO_X];   // forward/back axis
  int centerY = currentSettings[SET_ZERO_Y];   // left/right axis

  // 1. FORWARD TEST (stick pushed fully forward, no steering)
  uint32_t runtime = millis() + (testSeconds * 1000L);
  while (millis() < runtime){
    CalculateMotorDriverValues(centerX - TEST_FULL_STICK, centerY);
    SendData();
    sprintf(buf2, "FWD %4.1f s", (runtime - millis()) / 1000.0);
    displayMsg(buf2, 255, 1);
    delay(20);
    yield();
  }

  // 2. CLEAR VISUAL PAUSE (stick at center, keep sending stop packets)
  uint32_t pauseRuntime = millis() + 1000;
  while (millis() < pauseRuntime){
    CalculateMotorDriverValues(centerX, centerY);
    SendData();
    displayMsg("PAUSE", 255, 1);
    delay(20);
    yield();
  }

  // 3. REVERSE TEST (stick pulled fully back, no steering)
  runtime = millis() + (testSeconds * 1000L);
  while (millis() < runtime){
    CalculateMotorDriverValues(centerX + TEST_FULL_STICK, centerY);
    SendData();
    sprintf(buf2, "REV %4.1f s", (runtime - millis()) / 1000.0);
    displayMsg(buf2, 255, 1);
    delay(20);
    yield();
  }

  // 4. STOP SEQUENCE (stick at center, keep sending stop packets)
  uint32_t stopRuntime = millis() + 1000;
  while (millis() < stopRuntime) {
    CalculateMotorDriverValues(centerX, centerY);
    SendData();
    displayMsg("TEST DONE", 255, 1);
    delay(20);
    yield();
  }

  Serial.println("test done");
}
