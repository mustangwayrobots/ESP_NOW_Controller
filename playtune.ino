// ==============================================================================
// playtune.ino - ESP32-C3 RTTTL Music & Buzzer Player for GPIO 20 (MOSFET Gate)
// ==============================================================================
#define isdigit(n) (n >= '0' && n <= '9')
#define OCTAVE_OFFSET 0

int notes[] = { 0,
262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988,
1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951
};

void blinkLED(int delayTime) {
  // Optional stub
}

void play_rtttl(int TonePin, const char *p) {
  byte default_dur = 4;
  byte default_oct = 6;
  int bpm = 63;
  int num;
  long wholenote;
  long duration;
  byte note;
  byte scale;

  String TuneName;
  while(*p != ':'){
    TuneName += *p; 
    p++;    
  }
  int str_len = TuneName.length() + 1;
  char char_array[str_len];
  TuneName.toCharArray(char_array, str_len);
  displayMsg(char_array);

  p++;                     

  if(*p == 'd') {
    p++; p++;              
    num = 0;
    while(isdigit(*p)) {
      num = (num * 10) + (*p++ - '0');
    }
    if(num > 0) default_dur = num;
    p++;                   
  }

  if(*p == 'o') {
    p++; p++;              
    num = *p++ - '0';
    if(num >= 3 && num <= 7) default_oct = num;
    p++;                   
  }

  if(*p == 'b') {
    p++; p++;              
    num = 0;
    while(isdigit(*p)) {
      num = (num * 10) + (*p++ - '0');
    }
    bpm = num;
    p++;                   
  }

  wholenote = (60 * 1000L / bpm) * 4;  

  while(*p) {
    num = 0;
    while(isdigit(*p)) {
      num = (num * 10) + (*p++ - '0');
    }
    
    if(num) duration = wholenote / num;
    else duration = wholenote / default_dur;  

    note = 0;
    switch(*p) {
      case 'c': note = 1; break;
      case 'd': note = 3; break;
      case 'e': note = 5; break;
      case 'f': note = 6; break;
      case 'g': note = 8; break;
      case 'a': note = 10; break;
      case 'b': note = 12; break;
      case 'p':
      default: note = 0;
    }
    p++;

    if(*p == '#') {
      note++;
      p++;
    }

    if(*p == '.') {
      duration += duration / 2;
      p++;
    }
  
    if(isdigit(*p)) {
      scale = *p - '0';
      p++;
    } else {
      scale = default_oct;
    }

    scale += OCTAVE_OFFSET;

    if(*p == ',') p++;       

    if(note) {
      int freq = notes[(scale - 4) * 12 + note];
      tone(TonePin, freq);
      delay(duration);
      noTone(TonePin);
    } else {
      delay(duration);
    }
  }
}