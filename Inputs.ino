/**
 * ==============================================================================
 * readInputs.ino - Input Polling, ADC Reads, and Button Edge Detection
 * ==============================================================================
 */

// External declarations for global flags and pins used across files
extern int joystickX;
extern int joystickY;
extern bool UpButton;
extern bool DownButton;
extern bool SpinButton;
extern bool MenuButtonTriggered;
extern bool SelectButtonTriggered;

void readInputs() {
  // Read raw joystick axes (12-bit ADC)
  joystickX = analogRead(JOYSTICK_HORZ_PIN);
  joystickY = analogRead(JOYSTICK_VERT_PIN);

  // Read action buttons (active LOW with internal pull-ups)
  UpButton = (digitalRead(UP_BUTTON_PIN) == LOW);
  DownButton = (digitalRead(DOWN_BUTTON_PIN) == LOW);
  SpinButton = (digitalRead(SPIN_BUTTON_PIN) == LOW);

  // --- Menu Button Press-Edge Detection (C-11 Fix) ---
  static bool lastMenuRawState = HIGH; 
  bool currentMenuRawState = digitalRead(MENU_BUTTON_PIN);

  MenuButtonTriggered = false;
  if (currentMenuRawState == LOW && lastMenuRawState == HIGH) {
    static unsigned long lastMenuPressTime = 0;
    if (millis() - lastMenuPressTime > 200) {
      lastMenuPressTime = millis();
      MenuButtonTriggered = true; 
    }
  }
  lastMenuRawState = currentMenuRawState;

  // --- Select Button Press-Edge Detection ---
  static bool lastSelectRawState = HIGH;
  bool currentSelectRawState = digitalRead(SELECT_BUTTON_PIN);

  SelectButtonTriggered = false;
  if (currentSelectRawState == LOW && lastSelectRawState == HIGH) {
    static unsigned long lastSelectPressTime = 0;
    if (millis() - lastSelectPressTime > 200) {
      lastSelectPressTime = millis();
      SelectButtonTriggered = true;
    }
  }
  lastSelectRawState = currentSelectRawState;
}