/**
 * ==============================================================================
 * ESP-NOW RC Transmitter Core (ESP32-C3 Architecture Layout)     
 * ==============================================================================
 */
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <Preferences.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "ConfigData.h"

// --- Board choice: change HARDWARE_V1 below to HARDWARE_V2 for the V2 board ---
// (A build that passes -DHARDWARE_Vx, like the GitHub flasher build, overrides this.)
#if !defined(HARDWARE_V1) && !defined(HARDWARE_V2)
  #define HARDWARE_V1
#endif
#include "HardwareConfig.h" // Pin numbers and screen type per board
#include "songs2.h"

Preferences preferences;

// --- Display driver, picked by the screen type in HardwareConfig.h ---
#if defined(OLED_SH1106_128X64)
  U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#elif defined(OLED_SSD1306_128X32)
  U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#else
  #error "No screen type set for this board in HardwareConfig.h"
#endif

// --- Shared variables (declared for every tab in ConfigData.h) ---
bool NormalMode = true;
bool TXConnectionStatus = false;
bool UpButton = false;
bool DownButton = false;
bool SpinButton = false;
bool MenuButtonTriggered = false;
bool SelectButtonTriggered = false;

volatile boolean ReceiveBindingComplete = false;

uint8_t BindingMac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t cachedCarMac[6];
uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
int stickThrottle = 2048;   // Start at stick center until the first real reading
int stickSteer = 2048;

// Adaptive Rate Tracking
unsigned long lastSendTime = 0;
int currentSendInterval = 20; // Default active 50Hz rate

// Function Declarations
void setupNormalMode();   
void setupBindingMode();
void loopNormalMode();    
void loopBindingMode();
void readInputs();
void displayMsg(const char *NewMessage);
void CalculateMotorDriverValues(int rawThrottle, int rawSteer);
void SendData();
boolean ActiveMenu();
void play_rtttl(int TonePin, const char *p);
void Beep(int BeepFrequency, int BeepPeriod, int BeepDuration);
void runtest();

void setup() {
  setCpuFrequencyMhz(80); 
  Serial.begin(115200); 
  Serial.println("");
  Serial.println("STARTING");
  Serial.println(BOARD_VERSION_STR);
  
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN); // Screen pins from HardwareConfig.h
  u8g2.setBusClock(400000);   // fast I2C: a full screen takes about 25 ms
  u8g2.begin();
  u8g2.setFlipMode(1); 
  u8g2.setContrast(255);

  pinMode(PIEZO_PIN, OUTPUT);
  
  pinMode(MENU_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SPIN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);

  // --- Load saved menu settings from flash (storage.ino) ---
  loadSettings();
  
  readInputs();    
  if (digitalRead(MENU_BUTTON_PIN) == HIGH){
    NormalMode = true;
    
    // Play power-on tune
    int tuneIndex = currentSettings[SET_POWER_ON_TUNE];
    if (tuneIndex > 0 && tuneIndex < NUM_SONGS) {
      play_rtttl(PIEZO_PIN, songs2[tuneIndex]);
    }
    setupNormalMode();      
  }else{
    NormalMode = false;
    setupBindingMode();  
  }  
}

void loop() {
  yield(); 
  if(NormalMode == true){
    loopNormalMode();
  }else{
    loopBindingMode();
  }
}
