/**
 * ==============================================================================
 * MenuActions.ino - SELECT button, value changes and saving
 * ==============================================================================
 */

// Forward declarations
void runtest();
void HandleHelpNavigation();

//**********************************************************************************************
void SelectButtonPressed() {
  // Leaving the help pages
  if (InfoModeActive) {
    InfoModeActive = false;
    SelectActive = false;
    drawMenuList(CurrentMenuItem);
    return;
  }

  // First press: start editing (or open help)
  if (SelectActive == false) {
    SelectActive = true;

    if (CurrentMenuItem == SET_INFO_HELP) {
      InfoModeActive = true;
      InfoItemIndex = 0;
      drawHelp(InfoItemIndex);
      return;
    }

    for (int i = 0; i < MENU_ITEMS; i++) tempSettings[i] = currentSettings[i];

    // Stick-center items start from the live stick position
    if (CurrentMenuItem == SET_ZERO_THROTTLE) tempSettings[SET_ZERO_THROTTLE] = stickThrottle;
    if (CurrentMenuItem == SET_ZERO_STEER)    tempSettings[SET_ZERO_STEER]    = stickSteer;
    drawEdit(CurrentMenuItem, tempSettings[CurrentMenuItem]);
    return;
  }

  // Second press: save
  SelectActive = false;
  if (CurrentMenuItem == SET_DEFAULTS) {
    for (int i = 0; i < MENU_ITEMS; i++) tempSettings[i] = defaultSettings[i];
  }
  for (int i = 0; i < MENU_ITEMS; i++) currentSettings[i] = tempSettings[i];
  saveSettings();   // storage.ino

  if (CurrentMenuItem == SET_DEFAULTS) {
    showStatus("SETTINGS", "", "Reset done", "Pairing is kept");
    delay(1200);
  } else if (CurrentMenuItem == SET_TEST_TRIM) {
    runtest();
  } else {
    showStatus(Menu[CurrentMenuItem].label, "", "Saved", "");
    delay(500);
  }
  drawMenuList(CurrentMenuItem);
}

//**********************************************************************************************
void AdjustParameterValues() {
  if (InfoModeActive) {
    HandleHelpNavigation(); // MenuHelp.ino
    return;
  }

  int stepSize = 1;
  if (UpButton == true && (millis() - navDebounceTime) > 150) {
    navDebounceTime = millis();
    tempSettings[CurrentMenuItem] = constrain(tempSettings[CurrentMenuItem] + stepSize, Menu[CurrentMenuItem].min, Menu[CurrentMenuItem].max);
    drawEdit(CurrentMenuItem, tempSettings[CurrentMenuItem]);
  }
  if (DownButton == true && (millis() - navDebounceTime) > 150) {
    navDebounceTime = millis();
    tempSettings[CurrentMenuItem] = constrain(tempSettings[CurrentMenuItem] - stepSize, Menu[CurrentMenuItem].min, Menu[CurrentMenuItem].max);
    drawEdit(CurrentMenuItem, tempSettings[CurrentMenuItem]);
  }
}
