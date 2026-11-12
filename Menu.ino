/**
 * ==============================================================================
 * Menu.ino - Core Menu Navigation and State Management  
 * ==============================================================================
 */
static uint32_t navDebounceTime = 0;
uint32_t debounceDelay = 200;
static boolean MenuActive = false;
static int CurrentMenuItem = 0;
static boolean SelectActive = false;

// Bring in external flags from other files
extern boolean InfoModeActive;
extern bool MenuButtonTriggered;
extern bool SelectButtonTriggered;

// Forward declarations for functions defined in other menu files
void SelectButtonPressed();
void AdjustParameterValues();
void CycleMenuItems();

//**********************************************************************************************
boolean ActiveMenu(){
  // Check edge-triggered menu flag
  if (MenuButtonTriggered){
    if(MenuActive == false){
      MenuActive = true;
      CurrentMenuItem = 0;
      SelectActive = false;
      InfoModeActive = false; 
      displayMsg(Menu[CurrentMenuItem].label, 255, 1);
    }else{
      MenuActive = false;
      SelectActive = false;
      InfoModeActive = false;
      displayMsg("WAITING", 40, 2);            
    }
  }  

  if(MenuActive == true){
    // Check edge-triggered select flag
    if (SelectButtonTriggered){      
      SelectButtonPressed();
    }
    
    if(SelectActive == true){      
      AdjustParameterValues();  
    }else{  
      CycleMenuItems();         
    }
  }
  return MenuActive;
}

//**********************************************************************************************
void CycleMenuItems(){
  if (UpButton == true && (millis() - navDebounceTime) > 150){
    navDebounceTime = millis();  
    CurrentMenuItem = (CurrentMenuItem + 1) % MENU_ITEMS;
    displayMsg(Menu[CurrentMenuItem].label, 255, 1);   
  }
  if (DownButton == true && (millis() - navDebounceTime) > 150){
    navDebounceTime = millis();  
    CurrentMenuItem = (CurrentMenuItem + MENU_ITEMS - 1) % MENU_ITEMS;
    displayMsg(Menu[CurrentMenuItem].label, 255, 1);   
  }
}