// ==============================================================================
// displayMsg.ino - Optimized I2C Display Routine (C-03 Bus Optimization)
// ==============================================================================
void displayMsg(const char *NewMessage, int contrast, int blinkMode, bool smallFont) {
  static char currentMessage[50] = "";
  static uint32_t blinkMillis = 0;
  static boolean displayState = true;
  static int prevContrast = -1;
  static int prevPowerSave = -1;
  
  char *orignal;
  char *token, *theRest;
  String myLine[5];
  
  // 1. I2C Optimization: Only update hardware contrast if the value has changed
  if (contrast != prevContrast) {
    prevContrast = contrast;
    u8g2.setContrast(contrast);
  }

  // 2. Handle blink duty cycle & power save state logic
  bool targetPowerSave = false;
  if (blinkMode > 1) {
    unsigned long interval = 1000 * (blinkMode - 1); 
    if (millis() - blinkMillis > interval) {
      blinkMillis = millis();
      displayState = !displayState;
    }    
    if (!displayState) {
      targetPowerSave = true;
    }
  }

  // Only update I2C power save state if it actually changed
  if (targetPowerSave != prevPowerSave) {
    prevPowerSave = targetPowerSave;
    u8g2.setPowerSave(targetPowerSave ? 1 : 0);
  }

  // If display is currently blanked by blink mode, skip buffer rendering
  if (targetPowerSave) {
    return; 
  }

  // 3. Only redraw buffer and send I2C frame if the message text has changed
  if (strcmp(currentMessage, NewMessage) != 0) {
    strcpy(currentMessage, NewMessage);
    
    u8g2.clearBuffer();          
    
    if (smallFont) {
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(0, 9, currentMessage); 
    } else if (strlen(currentMessage) <= 9){
      u8g2.setFont(u8g2_font_inb16_mr); 
      u8g2.drawStr(0, 24, currentMessage); 
    } else {
      int count = 0;
      orignal = strdup(NewMessage);
      theRest = orignal;
      myLine[count] = "";
      while((token = strtok_r(theRest, " ", &theRest))){
        if (myLine[count].length() + strlen(token) < 9){
          if (myLine[count].length() > 0){
            myLine[count] += " ";          
          }
          myLine[count] += token;
        }else{
          count++;
          myLine[count] = String(token);          
        }
      } 
      u8g2.setFont(u8g2_font_luBS14_tf);      
      for(int i = 0; i <= count; ++i){
        int str_len = myLine[i].length() + 1;
        char char_array[str_len];
        myLine[i].toCharArray(char_array, str_len);        
        u8g2.drawStr(0, ((i+1)*16)-2, char_array); 
      }
      free(orignal);
    }
    
    u8g2.sendBuffer();          
  }
}