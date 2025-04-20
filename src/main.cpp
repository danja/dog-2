#include <Arduino.h>
#include "core.h"
#include <TM1638lite.h>

#define STB_PIN 7
#define CLK_PIN 10
#define DIO_PIN 11
TM1638lite tm(STB_PIN, CLK_PIN, DIO_PIN);
const int speakerPin = 9;

void setup()
{
  Serial.begin(9600);
  Serial.println("Serial started");

  tm.reset();
  Serial.println("TM1638 reset");

  setupCore();
  Serial.println("Core setup done");

  initRegisters();
  Serial.println("Registers initialized");

  resetSystem();
  Serial.println("reseted System");
  loadFromEEPROM();
  Serial.println("loaded FromEEPROM");
  for (uint8_t i = 0; i < 8; i++)
  {
    tm.setLED(i, 1); // switch off
  }
  tm.displayASCII(4, 'r'); //
                           // while(1) {
  doTone(0, 10);
  doTone(1, 100);
  // }
  translateProg();
  Serial.println("translated Prog");
  // while(1) loop();
}

void loop()
{
  Serial.println("loop");
  processNextProgramStep();
  handleButtons();
  display();
  if (mode == RUN_MODE)
  {
    doOperation();
  }
  delay(10);
}
