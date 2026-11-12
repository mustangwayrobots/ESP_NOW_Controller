// ==============================================================================
// TestRoutine.ino - Trim test: forward, pause, reverse, stop
// ==============================================================================
// "Moves the stick" in software by feeding CalculateMotorDriverValues() a
// position relative to the calibrated stick center. Press MENU to stop early.

// One part of the test. Returns true if MENU was pressed (stop the test).
static bool testLeg(const char* label, const char* footer, int throttleOffset,
                    uint32_t ms, int centerThrottle, int centerSteer) {
  uint32_t start = millis();
  while (millis() - start < ms) {
    if (digitalRead(MENU_BUTTON_PIN) == LOW) return true;
    CalculateMotorDriverValues(centerThrottle + throttleOffset, centerSteer);
    SendData();

    char big[12];
    unsigned long secondsLeft = (ms - (millis() - start) + 999) / 1000;
    snprintf(big, sizeof(big), "%s %lu", label, secondsLeft);
    showStatus("TRIM TEST", "MENU=stop", big, footer);
    delay(20);
  }
  return false;
}

void runtest(){
  uint32_t legMs     = currentSettings[SET_TEST_TRIM] * 1000UL;
  int centerThrottle = currentSettings[SET_ZERO_THROTTLE];   // forward/back axis
  int centerSteer    = currentSettings[SET_ZERO_STEER];      // left/right axis

  bool stopped = testLeg("FWD",   "Driving forward",  -STICK_FULL_RANGE, legMs, centerThrottle, centerSteer)
              || testLeg("PAUSE", "Stopped",           0,                1000,  centerThrottle, centerSteer)
              || testLeg("REV",   "Driving backward",  STICK_FULL_RANGE, legMs, centerThrottle, centerSteer);

  // Always finish with 1 s of stop packets
  uint32_t start = millis();
  while (millis() - start < 1000) {
    CalculateMotorDriverValues(centerThrottle, centerSteer);
    SendData();
    showStatus("TRIM TEST", "", stopped ? "Stopped" : "Done", "");
    delay(20);
  }

  // Wait for MENU release so it does not also close the menu
  while (digitalRead(MENU_BUTTON_PIN) == LOW) delay(10);
  Serial.println(stopped ? "test stopped" : "test done");
}
