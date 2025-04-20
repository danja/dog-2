#include "core.h"

boolean newData = false;
boolean readyToReceive = true;
static boolean recvInProgress = false;

void setup() {
  Serial.begin(57600);
  tm.reset();
  welcome();
  initRegisters();
  resetSystem();
  loadFromEEPROM();
  translateProg();
}

void loop() {
  if (!recvInProgress) {
    handleButtons();

    // trap overflows
    if (pc >= MAX_PROG_SIZE) pc = 0;
    if (pc < 0) pc = MAX_PROG_SIZE - 1;

    showStatus();

    if (!pause) {
      display();

      if (mode == RUN_MODE) {
        doOperation();
        pc++;
      }
    }
  }
  if (mode != RUN_MODE && !newData && readyToReceive) {
    receiveProg();
  }
  if (newData) {
    translateProg();
  }
}