// ==============================================================================
// MenuHelp.ino - Info / Help Mode Rendering and Navigation
// ==============================================================================
boolean InfoModeActive = false;
int InfoItemIndex = 0;

// Renders the help text split across two lines with a "PRE    NEXT" footer
void displayHelpScreen(const char *helpText) {
  u8g2.setContrast(255);
  u8g2.setPowerSave(0);
  u8g2.clearBuffer();
  
  u8g2.setFont(u8g2_font_6x10_tf);
  
  // Split long strings cleanly so they don't get cut off on the 128x32 display
  String text = String(helpText);
  if (text.length() > 21) {
    int splitIndex = text.lastIndexOf(' ', 21); // Find last space before 21 chars
    if (splitIndex == -1) splitIndex = 21;
    
    String line1 = text.substring(0, splitIndex);
    String line2 = text.substring(splitIndex + 1);
    
    u8g2.drawStr(0, 9, line1.c_str());
    u8g2.drawStr(0, 19, line2.c_str());
  } else {
    u8g2.drawStr(0, 12, helpText);
  }
  
  // Bottom line: Navigation footer with PRE on left and NEXT on right
  u8g2.drawStr(0, 31, "PRE");
  u8g2.drawStr(102, 31, "NEXT");
  
  u8g2.sendBuffer();
}

// Handles Up and Down button scrolling while inside the Help browser
void HandleHelpNavigation() {
  extern uint32_t navDebounceTime;
  
  if (UpButton == true && (millis() - navDebounceTime) > 150){
    navDebounceTime = millis();
    InfoItemIndex = (InfoItemIndex + 1) % MENU_ITEMS;
    displayHelpScreen(menuHelpText[InfoItemIndex]);
  }
  if (DownButton == true && (millis() - navDebounceTime) > 150){
    navDebounceTime = millis();
    InfoItemIndex = (InfoItemIndex + MENU_ITEMS - 1) % MENU_ITEMS; 
    displayHelpScreen(menuHelpText[InfoItemIndex]);
  }
}