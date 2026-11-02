/**
 * ==============================================================================
 * MenuActions.ino - Parameter Adjustments, NVS Saving, and Value Formatting
 * ==============================================================================
 */

// Forward declarations
void runtest();
void displayHelpScreen(const char *helpText);

extern boolean InfoModeActive;
extern int InfoItemIndex;

//**********************************************************************************************
void SelectButtonPressed(){
  selectDebounceTime = millis();
  
  if (InfoModeActive) {
    InfoModeActive = false;
    SelectActive = false;
    displayMsg(Menu[CurrentMenuItem].label, 255, 1);
    return;
  }

  if(SelectActive == false){
    SelectActive = true;
    
    if (strcmp(Menu[CurrentMenuItem].label, "INFO / HELP") == 0) {
      InfoModeActive = true;
      InfoItemIndex = 0;
      displayHelpScreen(menuHelpText[InfoItemIndex]);
      return;
    }

    for (int i = 0; i < MENU_ITEMS; i++){
      tempSettings[i] = currentSettings[i];
      
      if (strcmp(Menu[CurrentMenuItem].label, "JOYSTICK ZERO-X") == 0 ) {
        tempSettings[CurrentMenuItem] = analogRead(JOYSTICK_VERT_PIN);                    
      }
      if (strcmp(Menu[CurrentMenuItem].label, "JOYSTICK ZERO-Y") == 0 ) {
        tempSettings[CurrentMenuItem] = analogRead(JOYSTICK_HORZ_PIN);                    
      }
    }
    DisplayParameterValue();
  }else{
    SelectActive = false;     
    
    preferences.begin("rc-config", false);
    for (int i = 0; i < MENU_ITEMS; i++){
      if (strcmp(Menu[CurrentMenuItem].label, "DEFAULTS") == 0 ){
        tempSettings[i] = defaultSettings[i];
      }
      currentSettings[i] = tempSettings[i];
      
      char key[10];
      snprintf(key, sizeof(key), "set_%d", i);
      preferences.putInt(key, currentSettings[i]);
    }
    preferences.end();
    Serial.println("Settings saved to NVS successfully.");
    
    if (strcmp(Menu[CurrentMenuItem].label, "DEFAULTS") == 0 ){
      displayMsg("RESET OK!", 255, 1);
      delay(1000); 
      MenuActive = false; 
      displayMsg("WAITING", 40, 2);
      return;
    }
    
    if (strcmp(Menu[CurrentMenuItem].label, "TEST TRIM") == 0){      
      Serial.println("Running trim test...");
      runtest(); 
    }
    displayMsg(Menu[CurrentMenuItem].label, 255, 1);
  }  
}

//**********************************************************************************************
void AdjustParameterValues(){
  if (InfoModeActive) {
    HandleHelpNavigation(); // Defined in MenuHelp.ino
    return;
  }

  int stepSize = 1;
  if (strcmp(Menu[CurrentMenuItem].label, "JOYSTICK ZERO-X") == 0 || 
      strcmp(Menu[CurrentMenuItem].label, "JOYSTICK ZERO-Y") == 0) {
    stepSize = 1; 
  }

  if (UpButton == true && (millis() - navDebounceTime) > 150){
    navDebounceTime = millis();
    tempSettings[CurrentMenuItem] = constrain(tempSettings[CurrentMenuItem] + stepSize, Menu[CurrentMenuItem].min, Menu[CurrentMenuItem].max);
    DisplayParameterValue();
  }
  if (DownButton == true && (millis() - navDebounceTime) > 150){
    navDebounceTime = millis();
    tempSettings[CurrentMenuItem] = constrain(tempSettings[CurrentMenuItem] - stepSize, Menu[CurrentMenuItem].min, Menu[CurrentMenuItem].max);
    DisplayParameterValue();
  }
}

//**********************************************************************************************
void DisplayParameterValue(){
  char buf2[50];
  if(strcmp(Menu[CurrentMenuItem].label, "TEST TRIM") == 0 ){
    sprintf (buf2, "%2i sec", tempSettings[CurrentMenuItem]);        
    displayMsg(buf2, 255, 1);  
  }else if(strcmp(Menu[CurrentMenuItem].label, "MAX SPEED") == 0 ||
           strcmp(Menu[CurrentMenuItem].label, "SPIN SPEED") == 0 ||
           strcmp(Menu[CurrentMenuItem].label, "STEER RATE") == 0 ||
           strcmp(Menu[CurrentMenuItem].label, "STEER EXPO") == 0 ||
           strcmp(Menu[CurrentMenuItem].label, "ACCEL RATE") == 0 ){
    sprintf (buf2, "%3i%%", tempSettings[CurrentMenuItem]); 
    displayMsg(buf2, 255, 1);
  }else if(strcmp(Menu[CurrentMenuItem].label, "WEAPON ON VALUE") == 0 ||
           strcmp(Menu[CurrentMenuItem].label, "WEAPON OFF VALUE") == 0 ){
    sprintf (buf2, "%3i deg", tempSettings[CurrentMenuItem]); 
    displayMsg(buf2, 255, 1);
  }else if(strcmp(Menu[CurrentMenuItem].label, "MOTOR \"A\" DIRECTION") == 0 ||
           strcmp(Menu[CurrentMenuItem].label, "MOTOR \"B\" DIRECTION") == 0 ||
           strcmp(Menu[CurrentMenuItem].label, "MOTOR A-B RIGHT-LEFT")  == 0 ){
    if(tempSettings[CurrentMenuItem] == 0) displayMsg("NORMAL", 255, 1);
    if(tempSettings[CurrentMenuItem] == 1) displayMsg("REVERSE", 255, 1);
  }else if(strcmp(Menu[CurrentMenuItem].label, "DEFAULTS") == 0 ){
    displayMsg("CONFIRM", 255, 1);      
  }else if(strcmp(Menu[CurrentMenuItem].label, "POWER ON TUNE") == 0 ){    
    char* copy = strdup(songs2[tempSettings[CurrentMenuItem]]);   
    displayMsg(strtok(copy,":"), 255, 1);   
    free(copy);   
  }else{ 
    sprintf (buf2, "%5i", tempSettings[CurrentMenuItem]);
    displayMsg(buf2, 255, 1);      
  }    
}