// ==============================================================================
// MenuHelp.ino - Help page navigation (drawing is in Screens.ino)
// ==============================================================================
boolean InfoModeActive = false;
int InfoItemIndex = 0;

// UP / DOWN page through the help text while help is open
void HandleHelpNavigation() {
  extern uint32_t navDebounceTime;

  if (UpButton == true && (millis() - navDebounceTime) > 150){
    navDebounceTime = millis();
    InfoItemIndex = (InfoItemIndex + MENU_ITEMS - 1) % MENU_ITEMS; // UP = previous item (higher on screen)
    drawHelp(InfoItemIndex);
  }
  if (DownButton == true && (millis() - navDebounceTime) > 150){
    navDebounceTime = millis();
    InfoItemIndex = (InfoItemIndex + 1) % MENU_ITEMS; // DOWN = next item (lower on screen)
    drawHelp(InfoItemIndex);
  }
}
