/**
 * ==============================================================================
 * MenuActions.ino - Parameter Adjustments, NVS Saving, and Value Formatting
 * ==============================================================================
 */

// Forward declarations
void runtest();
void displayHelpScreen(const char *helpText);
void DisplayParameterValue();

extern boolean InfoModeActive;
extern int InfoItemIndex;

//**********************************************************************************************
// ==============================================================================
// MenuActions.ino - Parameter Adjustments and Button Actions
// ==============================================================================

void SelectButtonPressed() {
  if (InfoModeActive) {
    InfoModeActive = false;
    SelectActive = false;
    displayMsg(Menu[CurrentMenuItem].label, 255, 1);
    return;
  }

  if (SelectActive == false) {
    SelectActive = true;
    
    if (CurrentMenuItem == SET_INFO_HELP) {
      InfoModeActive = true;
      InfoItemIndex = 0;
      displayHelpScreen(menuHelpText[InfoItemIndex]);
      return;
    }

    for (int i = 0; i < MENU_ITEMS; i++) {
      tempSettings[i] = currentSettings[i];
    }

    // Stick-center items start from the live stick position. joystickX / joystickY
    // are the same readings the driving code uses, so the saved center always
    // matches the axis it is applied to (X = forward/back, Y = left/right).
    if (CurrentMenuItem == SET_ZERO_X) tempSettings[SET_ZERO_X] = joystickX;
    if (CurrentMenuItem == SET_ZERO_Y) tempSettings[SET_ZERO_Y] = joystickY;
    DisplayParameterValue();
  } else {
    SelectActive = false;     
    
    preferences.begin("rc-config", false);
    for (int i = 0; i < MENU_ITEMS; i++) {
      if (CurrentMenuItem == SET_DEFAULTS) {
        tempSettings[i] = defaultSettings[i];
      }
      currentSettings[i] = tempSettings[i];
      
      char key[10];
      snprintf(key, sizeof(key), "set_%d", i);
      preferences.putInt(key, currentSettings[i]);
    }
    preferences.end();
#if DEBUG_SERIAL_OUTPUT
    Serial.println("Settings saved to NVS successfully.");
#endif
    
    if (CurrentMenuItem == SET_DEFAULTS) {
      displayMsg("RESET OK!", 255, 1);
      delay(1000); 
      MenuActive = false; 
      displayMsg("WAITING", 40, 2);
      return;
    }
    
    if (CurrentMenuItem == SET_TEST_TRIM) {      
#if DEBUG_SERIAL_OUTPUT
      Serial.println("Running trim test...");
#endif
      runtest(); 
    }
    displayMsg(Menu[CurrentMenuItem].label, 255, 1);
  }  
}

//**********************************************************************************************
void AdjustParameterValues() {
  if (InfoModeActive) {
    HandleHelpNavigation(); // Defined in MenuHelp.ino
    return;
  }

  int stepSize = 1;
  // (Every setting steps by 1 for now. To make one step faster, e.g. the
  //  stick-center items, change stepSize here based on CurrentMenuItem.)

  if (UpButton == true && (millis() - navDebounceTime) > 150) {
    navDebounceTime = millis();
    tempSettings[CurrentMenuItem] = constrain(tempSettings[CurrentMenuItem] + stepSize, Menu[CurrentMenuItem].min, Menu[CurrentMenuItem].max);
    DisplayParameterValue();
  }
  if (DownButton == true && (millis() - navDebounceTime) > 150) {
    navDebounceTime = millis();
    tempSettings[CurrentMenuItem] = constrain(tempSettings[CurrentMenuItem] - stepSize, Menu[CurrentMenuItem].min, Menu[CurrentMenuItem].max);
    DisplayParameterValue();
  }
}

//**********************************************************************************************
void DisplayParameterValue() {
  char buf2[50];
  if (CurrentMenuItem == SET_TEST_TRIM) {
    sprintf (buf2, "%2i sec", tempSettings[CurrentMenuItem]);
    displayMsg(buf2, 255, 1);
  } else if (CurrentMenuItem == SET_MAX_SPEED ||
             CurrentMenuItem == SET_SPIN_SPEED ||
             CurrentMenuItem == SET_STEER_RATE ||
             CurrentMenuItem == SET_STEER_EXPO ||
             CurrentMenuItem == SET_ACCEL_RATE) {
    sprintf (buf2, "%3i%%", tempSettings[CurrentMenuItem]);
    displayMsg(buf2, 255, 1);
  } else if (CurrentMenuItem == SET_WEAPON_ON ||
             CurrentMenuItem == SET_WEAPON_OFF) {
    sprintf (buf2, "%3i deg", tempSettings[CurrentMenuItem]);
    displayMsg(buf2, 255, 1);
  } else if (CurrentMenuItem == SET_MOTOR_A_DIR ||
             CurrentMenuItem == SET_MOTOR_B_DIR ||
             CurrentMenuItem == SET_MOTOR_SWAP) {
    if (tempSettings[CurrentMenuItem] == 0) displayMsg("NORMAL", 255, 1);
    if (tempSettings[CurrentMenuItem] == 1) displayMsg("REVERSE", 255, 1);
  } else if (CurrentMenuItem == SET_DEFAULTS) {
    displayMsg("CONFIRM", 255, 1);
  } else if (CurrentMenuItem == SET_POWER_ON_TUNE) {
    char* copy = strdup(songs2[tempSettings[CurrentMenuItem]]);
    displayMsg(strtok(copy,":"), 255, 1);
    free(copy);
  } else {
    sprintf (buf2, "%5i", tempSettings[CurrentMenuItem]);
    displayMsg(buf2, 255, 1);
  }
}
