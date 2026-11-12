// ==============================================================================
// displayMsg.ino - Quick one-message screen
// ==============================================================================
// Shows one message big and centered (used by the startup tune for its name).
void displayMsg(const char *NewMessage) {
  showStatus("", "", NewMessage, "");
}
