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
#include "songs2.h"

Preferences preferences;

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define WIFI_CHANNEL 1

// Correct GPIO Pin Definitions (Avoiding bootstrap pins 2 & 9)
#define JOYSTICK_HORZ_PIN 3  
#define JOYSTICK_VERT_PIN 4  
#define MENU_BUTTON_PIN   10 
#define SELECT_BUTTON_PIN 8  
#define SEL_BUTTON_PIN    7  
#define UP_BUTTON_PIN     1  
#define DOWN_BUTTON_PIN   0  
#define PIEZO_PIN         20 

boolean NormalMode = true;
boolean TXConnectionStatus = false;
boolean RXConnectionStatus = false;
boolean MenuButton, SelectButton, UpButton, DownButton, SpinButton;
boolean NewDataFlag = false;

volatile boolean SendBindingComplete = false; 
volatile boolean ReceiveBindingComplete = false;

uint8_t BindingMac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x99};
uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
int Speed_A = 0;
int Speed_B = 0;
int joystickX;
int joystickY;
boolean MenuMode;

// Function Declarations
void setupNormalMode();   
void setupBindingMode();
void loopNormalMode();    
void loopBindingMode();
void readInputs();
void displayMsg(const char *NewMessage, int contrast = 255, int blinkMode = 1, bool smallFont = false);
void CalculateMotorDriverValues(int rawX, int rawY);
void SendData();
boolean ActiveMenu();
void play_rtttl(int TonePin, const char *p);
void Beep(int BeepFrequency, int BeepPeriod, int BeepDuration);
void runtest();

void setup() {
  setCpuFrequencyMhz(80); 
  Serial.begin(115200); 
  Serial.println("");
  
  Wire.begin(5, 6); // SDA on GPIO 5, SCL on GPIO 6
  u8g2.begin();
  u8g2.setFlipMode(1); 

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIEZO_PIN, OUTPUT);
  
  pinMode(MENU_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SEL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);

  // --- Load Settings and MAC from ESP32 NVS (Preferences) ---
  preferences.begin("rc-config", false);
  
  if (preferences.isKey("mac")) {
    String savedMac = preferences.getString("mac", "");
    sscanf(savedMac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &BindingMac[0], &BindingMac[1], &BindingMac[2],
           &BindingMac[3], &BindingMac[4], &BindingMac[5]);
    Serial.println("Loaded bound MAC from NVS.");
  }

  for(int i = 0; i <  MENU_ITEMS; i++) {
    char key[10];
    snprintf(key, sizeof(key), "set_%d", i);
    currentSettings[i] = preferences.getInt(key, defaultSettings[i]);
  }
  preferences.end();
  
  readInputs();    
  if (MenuButton == false){
    NormalMode = true;
    
    // Play power-on tune using name-based lookup
    int tuneIndex = getSetting("POWER ON TUNE");
    if (tuneIndex > 0 && tuneIndex < 94) {
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