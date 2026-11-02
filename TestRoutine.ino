// ==============================================================================
// TestRoutine.ino - Automated Trim and Motor Testing Routine  
// ==============================================================================
void runtest(){
  char buf2[20];
  int testSeconds = getSetting("TEST TRIM");
  
  // 1. FORWARD TEST
  uint32_t runtime = millis() + (testSeconds * 1000L); 
  while (millis() < runtime){
    CalculateMotorDriverValues(0, -1023);  // Forward throttle
    sprintf(buf2, "FWD %4.1f s", (runtime - millis()) / 1000.0);
    displayMsg(buf2, 255, 1);
    delay(20);
    yield();
  }
  
  // 2. CLEAR VISUAL PAUSE (Continuously ramp down and stream zero packets)
  uint32_t pauseRuntime = millis() + 1000;  
  while (millis() < pauseRuntime){
    CalculateMotorDriverValues(0, 0);  
    displayMsg("PAUSE", 255, 1);
    delay(20);
    yield();
  }
  
  // 3. REVERSE TEST
  runtime = millis() + (testSeconds * 1000L);  
  while (millis() < runtime){
    CalculateMotorDriverValues(0, 1023);  // Reverse throttle
    sprintf(buf2, "REV %4.1f s", (runtime - millis()) / 1000.0);
    displayMsg(buf2, 255, 1);
    delay(20);
    yield();
  }
  
  // 4. STOP SEQUENCE (Continuously ramp down to a complete stop)
  uint32_t stopRuntime = millis() + 1000;
  while (millis() < stopRuntime) {
    CalculateMotorDriverValues(0, 0);  
    displayMsg("TEST DONE", 255, 1);
    delay(20);
    yield();
  }
  
  Serial.println("test done");
}