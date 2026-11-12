/**
 * ==============================================================================
 * Menu.ino - Core Menu Navigation and State Management
 * ==============================================================================
 * MENU button:  closed -> open the settings list
 *               editing or reading help -> back to the list (edit not saved)
 *               on the list -> close the menu
 */
static uint32_t navDebounceTime = 0;
static boolean MenuActive = false;
static int CurrentMenuItem = 0;
static boolean SelectActive = false;

// Forward declarations for functions defined in other menu files
void SelectButtonPressed();
void AdjustParameterValues();
void CycleMenuItems();

//**********************************************************************************************
boolean ActiveMenu(){
  if (MenuButtonTriggered){
    if (!MenuActive){
      MenuActive = true;
      CurrentMenuItem = 0;
      SelectActive = false;
      InfoModeActive = false;
      drawMenuList(CurrentMenuItem);
    } else if (SelectActive || InfoModeActive){
      SelectActive = false;          // cancel the edit / leave help
      InfoModeActive = false;
      drawMenuList(CurrentMenuItem);
    } else {
      MenuActive = false;            // close the menu
      screenInvalidate();            // driving screen redraws on the next pass
    }
  }

  if (MenuActive){
    if (SelectButtonTriggered){
      SelectButtonPressed();
    }
    if (SelectActive){
      AdjustParameterValues();
    } else {
      CycleMenuItems();
    }
  }
  return MenuActive;
}

//**********************************************************************************************
void CycleMenuItems(){
  if (UpButton == true && (millis() - navDebounceTime) > 150){
    navDebounceTime = millis();
    CurrentMenuItem = (CurrentMenuItem + MENU_ITEMS - 1) % MENU_ITEMS; // UP = previous item (higher on screen)
    drawMenuList(CurrentMenuItem);
  }
  if (DownButton == true && (millis() - navDebounceTime) > 150){
    navDebounceTime = millis();
    CurrentMenuItem = (CurrentMenuItem + 1) % MENU_ITEMS; // DOWN = next item (lower on screen)
    drawMenuList(CurrentMenuItem);
  }
}
