// ==============================================================================
// Screens.ino - Every OLED screen (128x64 SH1106 layout)
// ==============================================================================
// All screens share one layout:
//   rows  0-10  header bar (white bar, black text): title on the left, info on the right
//   rows 12-52  main area (big value or a list)
//   rows 54-63  footer: a short hint in small text
// Sending a full frame over I2C takes about 25 ms, so status screens only
// redraw when something on them actually changes.

static char lastStatusKey[80] = "";

// Forget what is on screen, so the next showStatus() always redraws
void screenInvalidate() {
  lastStatusKey[0] = 0;
}

// White header bar with black text
static void drawHeader(const char* left, const char* right) {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setDrawColor(1);
  u8g2.drawBox(0, 0, 128, 11);
  u8g2.setDrawColor(0);
  u8g2.drawStr(2, 9, left);
  if (right && right[0]) u8g2.drawStr(126 - u8g2.getStrWidth(right), 9, right);
  u8g2.setDrawColor(1);
}

// Small text centered across the screen (font must already be set)
static void drawCentered(const char* text, int baseline) {
  u8g2.drawStr((128 - u8g2.getStrWidth(text)) / 2, baseline, text);
}

// Text as big as fits: large font, then medium, then small
static void drawBigCentered(const char* text, int baseline) {
  u8g2.setFont(u8g2_font_logisoso22_tf);
  if (u8g2.getStrWidth(text) > 126) u8g2.setFont(u8g2_font_helvB12_tf);
  if (u8g2.getStrWidth(text) > 126) u8g2.setFont(u8g2_font_6x10_tf);
  drawCentered(text, baseline);
}

// General status screen: header, one big line, footer hint.
// Pass "" for any part you do not want.
void showStatus(const char* header, const char* right, const char* big, const char* footer) {
  char key[80];
  snprintf(key, sizeof(key), "%s|%s|%s|%s", header, right, big, footer);
  if (strcmp(key, lastStatusKey) == 0) return;   // nothing changed, skip the I2C transfer
  strcpy(lastStatusKey, key);

  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  if (header[0]) drawHeader(header, right);
  drawBigCentered(big, header[0] ? 41 : 38);
  if (footer[0]) {
    u8g2.setFont(u8g2_font_6x10_tf);
    drawCentered(footer, 62);
  }
  u8g2.sendBuffer();
}

// Turns a setting value into display text.
// longForm = true for the edit screen, false for the short list column.
void formatSettingValue(int item, int value, char* buf, size_t len, bool longForm) {
  switch (item) {
    case SET_MAX_SPEED: case SET_ACCEL_RATE: case SET_STEER_RATE:
    case SET_STEER_EXPO: case SET_SPIN_SPEED:
      snprintf(buf, len, "%d%%", value);
      break;
    case SET_FORWARD_TRIM: case SET_REVERSE_TRIM:
      if (value == 0) snprintf(buf, len, "0");
      else            snprintf(buf, len, "%+d", value);
      break;
    case SET_MOTOR_A_DIR: case SET_MOTOR_B_DIR:
      snprintf(buf, len, "%s", value ? (longForm ? "Reversed" : "Rev") : "Normal");
      break;
    case SET_MOTOR_SWAP:
      snprintf(buf, len, "%s", value ? (longForm ? "Swapped" : "Yes") : (longForm ? "Normal" : "No"));
      break;
    case SET_WEAPON_ON: case SET_WEAPON_OFF:
      snprintf(buf, len, "%d\xb0", value);   // degree sign
      break;
    case SET_TEST_TRIM:
      snprintf(buf, len, "%d s", value);
      break;
    case SET_POWER_ON_TUNE:
      if (value == 0) {
        snprintf(buf, len, "Off");
      } else if (!longForm) {
        snprintf(buf, len, "#%d", value);
      } else {                                // tune name = text before the ':'
        const char* s = songs2[value];
        size_t i = 0;
        while (s[i] && s[i] != ':' && i < len - 1) { buf[i] = s[i]; i++; }
        buf[i] = 0;
      }
      break;
    case SET_DEFAULTS:
      snprintf(buf, len, "%s", longForm ? "Reset?" : ">");
      break;
    case SET_INFO_HELP:
      snprintf(buf, len, ">");
      break;
    default:
      snprintf(buf, len, "%d", value);
  }
}

// Settings list: 4 rows visible, the selected row is highlighted
void drawMenuList(int current) {
  const int ROWS = 4;
  static int top = 0;                         // first item shown on screen
  if (current < top) top = current;
  if (current >= top + ROWS) top = current - ROWS + 1;

  char right[8];
  snprintf(right, sizeof(right), "%d/%d", current + 1, MENU_ITEMS);

  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  drawHeader("SETTINGS", right);
  u8g2.setFont(u8g2_font_6x10_tf);

  for (int r = 0; r < ROWS; r++) {
    int item = top + r;
    if (item >= MENU_ITEMS) break;
    int y = 12 + r * 13;                      // row top
    if (item == current) {
      u8g2.drawRBox(0, y, 128, 13, 2);
      u8g2.setDrawColor(0);
    }
    char val[16];
    formatSettingValue(item, currentSettings[item], val, sizeof(val), false);
    u8g2.drawStr(3, y + 10, Menu[item].label);
    u8g2.drawStr(125 - u8g2.getStrWidth(val), y + 10, val);
    u8g2.setDrawColor(1);
  }
  u8g2.sendBuffer();
  screenInvalidate();
}

// Edit screen: setting name, big value, range bar, hint
void drawEdit(int item, int value) {
  char val[24];
  formatSettingValue(item, value, val, sizeof(val), true);

  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  drawHeader(Menu[item].label, "EDIT");
  drawBigCentered(val, 39);

  bool showBar = !(item == SET_MOTOR_A_DIR || item == SET_MOTOR_B_DIR ||
                   item == SET_MOTOR_SWAP  || item == SET_POWER_ON_TUNE ||
                   item == SET_DEFAULTS);
  if (showBar) {
    int span = Menu[item].max - Menu[item].min;
    int w = (span > 0) ? (long)(value - Menu[item].min) * 124 / span : 0;
    u8g2.drawFrame(0, 44, 128, 6);
    u8g2.drawBox(2, 46, w, 2);
  }

  const char* hint = "UP/DN change SEL=save";
  if (item == SET_DEFAULTS)                                   hint = "SEL=reset MENU=cancel";
  if (item == SET_ZERO_THROTTLE || item == SET_ZERO_STEER)    hint = "Center stick SEL=save";
  if (item == SET_TEST_TRIM)                                  hint = "SEL=save + run test";
  u8g2.setFont(u8g2_font_6x10_tf);
  drawCentered(hint, 62);
  u8g2.sendBuffer();
  screenInvalidate();
}

// Help page: setting name in the header, up to 4 word-wrapped lines below
void drawHelp(int index) {
  char right[8];
  snprintf(right, sizeof(right), "%d/%d", index + 1, MENU_ITEMS);

  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  drawHeader(Menu[index].label, right);
  u8g2.setFont(u8g2_font_6x10_tf);

  const int MAX_CHARS = 21;                   // characters per line in this font
  const char* p = menuHelpText[index];
  char line[MAX_CHARS + 1];
  for (int y = 23; *p && y <= 56; y += 11) {
    int n = strlen(p);
    if (n > MAX_CHARS) {                      // break at the last space that fits
      n = MAX_CHARS;
      while (n > 0 && p[n] != ' ') n--;
      if (n == 0) n = MAX_CHARS;
    }
    memcpy(line, p, n);
    line[n] = 0;
    u8g2.drawStr(0, y, line);
    p += n;
    while (*p == ' ') p++;
  }
  u8g2.sendBuffer();
  screenInvalidate();
}
