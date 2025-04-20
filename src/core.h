#ifndef CORE_H
#define CORE_H

#include <Arduino.h>
#include <stdint.h>
#include <TM1638lite.h>

#define NOTE_C4 262
#define NOTE_G4 392
#define NOP 0x00
#define STOP 0x01
#define PLAYN 0x02
#define BNS 0x6D
#define ADDBx 0x88
#define ADCBx 0x90
#define SUBBx 0x98

#define MAX_PROG_SIZE 128
#define PC_STACK_SIZE 64
#define X_STACK_SIZE 64

#define PROG_MODE false
#define RUN_MODE true
#define STEP false
#define RUN true

#define NEGATIVE 0
#define OVERFLOW 1
#define ZERO 2
#define CARRY 3
#define LDAi 0x10
#define LDAa 0x11
#define LDAx 0x12
#define STAa 0x14
#define STAx 0x15
#define LDBi 0x18
#define LDBa 0x19
#define LDBx 0x1A
#define STBa 0x1C
#define STBx 0x1D
#define AND 0x20
#define OR 0x21
#define XOR 0x22
#define COMA 0x23
#define COMB 0x24
#define ROLA 0x25
#define RORA 0x26
#define ROLB 0x27
#define RORB 0x28
#define SWAP 0x29
#define LSL 0x2A
#define LSR 0x2B
#define CLRS 0x30
#define SETS 0x31
#define SETC 0x32
#define CLC 0x33
#define CLV 0x34
#define BITAi 0x35
#define BITAa 0x36
#define BITAx 0x37
#define BITBi 0x39
#define BITBa 0x3A
#define BITBx 0x3B
#define PUSHXA 0x40
#define POPXA 0x41
#define PUSHXB 0x42
#define POPXB 0x43
#define SWAPS 0x44
#define DUP 0x45
#define OVER 0x46
#define ROT 0x47
#define DROP 0x48
#define TUCK 0x49
#define LSPi 0x50
#define LSPa 0x51
#define LSPx 0x52
#define LDXi 0x54
#define LDXa 0x55
#define LDXx 0x56
#define SPCa 0x58
#define SPCx 0x59
#define PUSHA 0x5A
#define POPA 0x5B
#define PUSHB 0x5C
#define POPB 0x5D
#define JMPi 0x60
#define JMPa 0x61
#define BRA 0x63
#define JSRa 0x64
#define JSRr 0x65
#define RTS 0x66
#define BZS 0x68
#define BZC 0x69
#define BCS 0x6A
#define BCC 0x6B
#define BVS 0x6E
#define BVC 0x6F
#define BGE 0x70
#define BGT 0x71
#define BLT 0x72
#define ABA 0x80
#define ADDAi 0x81
#define ADDAa 0x82
#define ADDAx 0x83
#define ADCAi 0x89
#define ADCAa 0x8A
#define ADCAx 0x8B
#define SUBAi 0x91
#define SUBAa 0x92
#define SUBAx 0x93
#define ANDAi 0xA0
#define ANDAa 0xA1
#define ANDAx 0xA2
#define ANDBi 0xA4
#define ANDBa 0xA5
#define ANDBx 0xA6
#define ORAi 0xA8
#define ORAa 0xA9
#define ORAx 0xAA
#define ORBi 0xAC
#define ORBa 0xAD
#define ORBx 0xAE
#define EORAi 0xB0
#define EORAa 0xB1
#define EORAx 0xB2
#define EORBi 0xB4
#define EORBa 0xB5
#define EORBx 0xB6
#define CAB 0xB8
#define CMPAi 0xB9
#define CMPAa 0xBA
#define CMPAx 0xBB
#define CMPBi 0xBD
#define CMPBa 0xBE
#define CMPBx 0xBF
#define CLRA 0xC1
#define CLRB 0xC2
#define CLRa 0xC3
#define CLRx 0xC4
#define INCA 0xD0
#define INCB 0xD1
#define INCa 0xD2
#define INCx 0xD3
#define INXS 0xD5
#define INCX 0xD6
#define DECA 0xD7
#define DECB 0xD8
#define DECa 0xD9
#define DECx 0xDA
#define DEXS 0xDC
#define DECX 0xDD
#define USE 0xE0
#define UNUSE 0xE1
#define TEMPO 0xF0
#define REST 0xF1
#define TONE 0xF2
#define TONEAB 0xF3
#define TONEx 0xF4
#define RESET 0xF7
#define DUMP 0xF8
#define TEST 0xF9
#define RND 0xFA
#define PAUSE 0xFB
#define DEBUG 0xFC
#define OK 0xFD
#define ERR 0xFE
#define HALT 0xFF

#define NOTE_IDX_C4 0
#define NOTE_IDX_G4 1

extern TM1638lite tm;
extern const int speakerPin;
extern uint8_t tempo;
extern const unsigned int notes[];
extern bool debugOn;
extern bool mode;
extern bool runMode;
extern bool pause;
extern uint8_t ledStep;

extern uint8_t program[MAX_PROG_SIZE];
extern uint8_t pcStack[PC_STACK_SIZE];
extern uint8_t xStack[X_STACK_SIZE];

extern unsigned int pc;
extern unsigned int xReg;
extern unsigned int pcStackP;
extern uint8_t acc[2];
extern uint8_t xStackP;
extern uint8_t status;

// --- Function Prototypes ---
void handleButtons();
void initRegisters();
void resetSystem();
void loadFromEEPROM();
void doOperation();
void translateProg();
void setupCore();             // <-- Check this line exists
void processNextProgramStep(); // <-- Check this line exists
void doTone(uint8_t noteIndex, uint8_t duration);
void initRegisters();
void resetSystem();
void resetRegisters();
void welcome();
void receiveProg();
uint16_t readLong(uint16_t startPos);
void translateProg();
void doOperation();
void doReset();
void doTone(uint8_t noteIndex, uint8_t duration);
void doToneI();
void doToneAB();
void doToneX();
void doRest();
uint16_t readAbsoluteAddr();
void setFlag(uint8_t flag, bool value);
bool getFlag(uint8_t flag);
void testFlags();
void doFlags(uint16_t val);
void LDi(uint8_t id);
void LDa(uint8_t id);
void LDx(uint8_t id);
void STa(uint8_t id);
void STx(uint8_t id);
void ROL(uint8_t id);
void ROR(uint8_t id);
void EORi(uint8_t id);
void BITi(uint8_t id);
void BITa(uint8_t id);
void BITx(uint8_t id);
void BIT(uint8_t id, uint8_t val);
void pushX(uint8_t id);
void popX(uint8_t id);
void swapX();
void dupX();
void display();
uint8_t hexCharToValue(uint8_t hexChar);
void showStatus();
void stepLED();
void waitForButton();
void handleButtons();
void showError(String message);
void debug(String message);
void debug(long value);
void flashMessage(String message);
void flashMessage(long value);
void displayCode();
void displayPC();
void displayHex(uint8_t startDisplay, uint8_t nBytes, long data);
void displayMode();

#endif