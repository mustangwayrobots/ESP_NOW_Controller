// ==============================================================================
// Beep.ino - Buzzer Beep Control for ESP32-C3 (GPIO 20 MOSFET Driver)
// ==============================================================================
void Beep(int BeepFrequency, int BeepPeriod, int BeepDuration){
  static uint32_t previousMillis;
  if ((millis() - previousMillis) > (BeepPeriod - BeepDuration)) {
    previousMillis = millis();
    tone(PIEZO_PIN, BeepFrequency);
    delay(BeepDuration);
    noTone(PIEZO_PIN);
  }   
}