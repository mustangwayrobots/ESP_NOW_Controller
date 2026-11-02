// ==============================================================================
// Menu.ino - Core Menu Navigation and State Management
// ==============================================================================
static uint32_t menuDebounceTime = 0;
static uint32_t selectDebounceTime = 0;
static uint32_t navDebounceTime = 0;
uint32_t debounceDelay = 200;
static boolean MenuActive = false;
static int CurrentMenuItem = 0;
static boolean SelectActive = false;

// Bring in InfoModeActive from MenuHelp.ino
extern boolean InfoModeActive;

// Forward declarations for functions defined in other menu files
void SelectButtonPressed();
void AdjustParameterValues();
void CycleMenuItems();

//**********************************************************************************************
boolean ActiveMenu(){
  if (MenuButton == true && (millis() - menuDebounceTime) > debounceDelay){
    menuDebounceTime = millis();
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
    if (SelectButton == true && (millis() - selectDebounceTime) > debounceDelay){      
      selectDebounceTime = millis();
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