#include "core.h"
#include <Arduino.h>
#include <EEPROM.h>
#include <ArduinoSTL.h>

// const int speakerPin = 9;

boolean mode = PROG_MODE;

uint8_t program[] = {
    PLAYN, NOTE_IDX_C4, 10,
    PLAYN, NOTE_IDX_G4, 20,
    STOP
};
boolean runMode = STEP;
boolean debugOn = true; // DEBUG
unsigned int pc = 0;
uint8_t xStack[X_STACK_SIZE]; // experimental stack-oriented programming/maths stack
uint8_t tempo = 120;
boolean loadToEEPROM;
uint16_t end;
uint8_t buffer [2 * MAX_PROG_SIZE]; // maybe buffer isn't needed now I've improved the serial..?
static uint8_t rawSize;
uint8_t pcStack[PC_STACK_SIZE]; // PC/subroutine-oriented stack
unsigned long buttonMillis = 0; // time since last button press
unsigned long buttonDelay = 200;
uint8_t previousButtons = 0;

uint8_t incdec = true; // true for increment, false for decrement

// static boolean recvInProgress = false;
boolean newData = false;
boolean readyToReceive = true;

const uint16_t noteFrequencies[] = {
    NOTE_C4,
    NOTE_G4,
};

uint8_t xStackP; // stack pointer, 8 bits, 0 to 255
unsigned int xReg; // index register, 16 bits, 0 to 65535  (or rather, MAX_PROG_SIZE?)
unsigned int pcStackP; // stack pointer, 16 bits

uint8_t acc[2]; // accumulators A & , 8 - bits, -128 to 127

boolean pause = false;

uint8_t ledStep = 0;

uint8_t status; // status register (flags), initialised with a vaguely helpful test pattern, LEDs over system displays only (4 & 5)


void setupCore() {
    pc = 0;
}

void loadFromEEPROM() {
    flashMessage("EEPROM");
    for (uint16_t i = 0; i < end; i++) {
      program[i] = EEPROM.read(i);
    }
  }


void resetRegisters() {
  pc = 0; // program counter
  acc[0] = 0; // accumulator A, 8-bits
  acc[1] = 0; // accumulator B, 8-bits
  xReg = 0; // index register, 16 bits
  pcStackP = 0; // PC stack pointer, 16 bits
  xStackP = 0; // Auxiliary stack pointer, 8 bits
  status = 0x30; // status register (flags), initialised with a vaguely helpful test pattern, LEDs over system displays only (4 & 5)
}

void initRegisters() {

    resetRegisters();
  
    for (unsigned int i = 0; i < MAX_PROG_SIZE; i++) { // wipe all instructions
      program[i] = 0; // NOP
    }
  
    for (unsigned int i = 0; i < PC_STACK_SIZE; i++) { // wipe contents of PC stack
      pcStack[i] = 0;
    }
  
    for (unsigned int i = 0; i < X_STACK_SIZE; i++) { // wipe contents of ALU stack
      xStack[i] = 0;
    }
  
    // misc resetting
  }

  void resetSystem() {
    mode = PROG_MODE;
    ledStep = 0;
    pause = false;
  }

  /*
   Move data from buffer into program array
*/
void translateProg() {

    uint16_t start = 0;
  
    // first two bytes specify program location
    if (!loadToEEPROM) {
      start = readLong(0) * 256 + readLong(2);
    }
    
           uint8_t hi = hexCharToValue(buffer[0]);
        uint8_t lo = hexCharToValue(buffer[1]);
        tm.displayHex(4, hi);
        tm.displayHex(5, lo);
        start = readLong(0);
        hi = hexCharToValue(buffer[2]);
        lo = hexCharToValue(buffer[3]);
  
    pc = start;
    Serial.println("pc = "+pc);
    for (uint8_t pos = 4; pos < rawSize - 1; pos = pos + 2) {
      Serial.write(hi);
     Serial.write(lo);
      uint8_t hi = hexCharToValue(buffer[pos]); // USE READLONG
      uint8_t lo = hexCharToValue(buffer[pos + 1]);
      uint8_t code = hi * 16 + lo;
      program[pc++] = code;
      delay(10);
      stepLED();
    }
    end = pc;
    newData = false;
    flashMessage("Loaded.");
  
    pc = start;
    display();
    showStatus();
  
    if (program[0] == TEST) {
      flashMessage("testing");
      mode = RUN_MODE;
    }
  
    if (loadToEEPROM) {
      flashMessage("EEPROM");
      for (uint16_t i = 0; i < end; i++) {
        EEPROM[i] = program[i];
      }
    }
  }

  
void handleButtons() {
    uint8_t buttons = tm.readButtons();
  
    // puts a delay on the buttons if they haven't changed so they don't run away
    unsigned long currentMillis = millis();
    if ((currentMillis - buttonMillis > buttonDelay) || (buttons != previousButtons)) {
      buttonMillis = currentMillis;
      previousButtons = buttons;
  
      // #################### DUAL BUTTONS ######################
      // ################ System Buttons ###################
  
      // 0 & 1 full-on reset & wipe
      if ( (buttons & (1 << 0)) && (buttons & (1 << 1))) {
        initRegisters();
        resetSystem();
        return;
      }
  
      // 0 & 2 load from EEPROM
      if ( (buttons & (1 << 0)) && (buttons & (1 << 2))) {
        initRegisters();
        loadFromEEPROM();
        return;
      }
  
      // 4 & 5 - reset pc
      if ( (buttons & (1 << 4)) && (buttons & (1 << 5))) {
        pc = 0;
        return;
      }
  
      // ################ Display Register Buttons ###################
  
      // 0 & 4 - display Accumulators A, B
      if ( (buttons & (1 << 0)) && (buttons & (1 << 4))) {
        displayPC();
        displayHex(4, 2, acc[0]);
        displayHex(6, 2, acc[1]);
        waitForButton();
        return;
      }
  
      // 0 & 5 - display Index Register
      if ( (buttons & (1 << 0)) && (buttons & (1 << 5))) {
        displayPC();
        displayHex(4, 4, xReg);
        waitForButton();
        return;
      }
  
      // 0 & 6 - display EEPROM
      if ( (buttons & (1 << 0)) && (buttons & (1 << 6))) {
        displayPC();
        displayHex(6, 2, EEPROM[pc]);
        waitForButton();
        return;
      }
  
      // 0 & 7 - display Auxiliary Stack Pointer & status
      if ( (buttons & (1 << 0)) && (buttons & (1 << 7))) {
        displayPC();
        displayHex(4, 2, xStackP);
        displayHex(6, 2, status);
        waitForButton();
        return;
      }
  
      // ################ Mode Buttons ###################
  
      // 0 & 3 - flip between single-step & free run
      if ( (buttons & (1 << 0)) && (buttons & (1 << 3)) && mode == RUN_MODE) {
        runMode == RUN; // compiler says it has no effect TODO
        flashMessage("run");
        return;
      }
  
  
      if (pause) {
        if (buttons & (1 << 4)) {
          pause = false;
          return;
        }
        return;
      }
  
      // #################### SINGLE BUTTON ######################
      if (buttons & (1 << 4)) {
        mode = !mode;
        if (mode == PROG_MODE) {
          pc = 0;
        } else {
          flashMessage("run");
        }
        return;
      }
  
      if (mode == PROG_MODE) {
        if (buttons & (1 << 5)) {
          incdec = !incdec;
          return;
        }
      }
  
      // if (mode == RUN_MODE) {
      //   return;
      //}
  
      // ################ Edit Buttons ###################
  
      // do PC buttons (0-3) - inc/dec value as appropriate, have removed wrap below zero, it was getting too confusing
      if (buttons & (1 << 3)) {
        if (incdec) {
          pc++;
        } else {
          if (pc > 0) pc--;
        }
        return;
      }
  
      if (buttons & (1 << 2)) {
        if (incdec) {
          pc += 16;
        } else {
          if (pc > 16) pc -= 16;
        }
        return;
      }
  
      if (buttons & (1 << 1)) {
        if (incdec) {
          pc += 256 ;
        } else {
          if (pc > 256) pc -= 256;
        }
        return;
      }
  
      /*
          if (buttons & (1 << 0)) { // is out of reach of current size
            if (incdec) {
              pc += 4096;
            } else {
              if (pc > 4096) pc -= 4096;
            }
            return;
          }
      */
  
      /**
         ################################# do code buttons ############
         (6 & 7)- inc/dec value as appropriate, note there's no carry or wrap
      */
      uint8_t codeLow = program[pc] & 0xF; // mask
      uint8_t codeHigh = (program[pc] & 0xF0) >> 4; // mask & shift
  
      if (buttons & (1 << 6)) {
        if (incdec) {
          codeHigh++;
          if (codeHigh >= 16) codeHigh = 0;
        } else {
          if (codeHigh > 0) {
            codeHigh--;
          }
        }
      }
  
      if (buttons & (1 << 7)) {
        if (incdec) {
          codeLow++;
          if (codeLow >= 16) codeLow = 0;
        } else {
          if (codeLow > 0) {
            codeLow--;
          }
        }
      }
      program[pc] = (codeHigh << 4) + codeLow;
    }
  }
  
  /**
   ###################
*/
void display() {
    displayMode();
    displayPC();
    displayCode();
  }

/**
    ################## Buttons #####################################
*/
void waitForButton() { // freezes display until button pressed
  delay(500); // a bit dirty, allow time for button release
  while (tm.readButtons() == 0) {
    delay(100);
  }
}
// 0 through 127 are, in hex, $00 through $7F, respectively.
// -128 through -1 are, in hex, $80 through $FF, respectively.

// NEGATIVE 0
// OVERFLOW 1
// ZERO 2
// CARRY 3

void setFlag(uint8_t flag, boolean value) {
  uint8_t mask = (1 << flag);

  if (value) {
    status = status | mask;
  } else {
    status = status & !mask;
  }
  showStatus();
}

boolean getFlag(uint8_t flag) {
  uint8_t mask = (1 << flag);
  return status & mask;
}
  /*
   ###########################################################
   ####################### Operations ########################
   ###########################################################
*/
void doOperation() {
    uint8_t op = program[pc];
    uint8_t val8; // temporary 8 bit value
    uint8_t val16; // temporary 16 bit value
  
    switch (op) {
  
      // ############### system ###############
  
      case NOP: // no operation
        return;
  
      //accumulator A load and store
  
      case LDAi:               // Load accumulator A immediate
        LDi(0);
        debug("LDAi");
        return;
  
      case LDAa:                            // Load accumulator A absolute
        LDa(0);
        return;
  
      case LDAx:                            // Load accumulator A indexed (6502 calls it Indexed Indirect)
        LDx(0);
        return;
  
      case STAa:                            // Store accumulator A absolute
        STa(0);
        return;
  
      case STAx:                            // Store accumulator A indexed (6502 calls it Indexed Indirect)
        STx(0);
        return;
  
      // ############### accumulator B load and store
  
      case LDBi:               // Load accumulator B immediate
        LDi(1);
        return;
  
      case LDBa:                            // Load accumulator B absolute
        LDa(1);
        return;
  
      case LDBx:                            // Load accumulator B indexed (6502 calls it Indexed Indirect)
        LDx(1);
        return;
  
      case STBa:                            // Store accumulator B absolute
        STa(1);
        return;
  
      case STBx:                            // Store accumulator B indexed (6502 calls it Indexed Indirect)
        STx(1);
        return;
  
      // ############### Branches
  
      case BRA:
        pc++;
        pc += (int8_t)program[pc];
        return;
  
      case BZS:                           // branch if zero set
        pc++;
        if (getFlag(ZERO)) {
          pc += (int8_t)program[pc]; // TODO overflow check, 2's comp
        }
        return;
  
      case BZC:                           // branch if zero clear
        pc++;
        if (!getFlag(ZERO)) {
          pc += (int8_t)program[pc]; // TODO overflow check, 2's comp
        }
        return;
  
      //
      case BCS:                           // branch if carry set
        pc++;
        if (getFlag(CARRY)) {
          pc += (int8_t)program[pc]; // TODO overflow check, 2's comp
        }
        return;
  
      case BCC:                           // branch if carry clear
        pc++;
        if (!getFlag(CARRY)) {
          pc += (int8_t)program[pc]; // TODO overflow check, 2's comp
        }
        return;
  
      // ############### acc logical operators ###############
  
      case CAB:                             // compare B with A, only NVZ status flags affected
        if (acc[0] == acc[1]) {
          setFlag(ZERO, true);
          setFlag(OVERFLOW, false);
          setFlag(NEGATIVE, false);
          return;
        } else {
          setFlag(ZERO, false);
        }
        val8 = acc[0] + (!acc[1] + 1);
        setFlag(CARRY, val8 & 128);
        // C bit set if subtraction would require a borrow in the most significant bit of result, otherwise cleared. V flag????
  
        return;
  
      case AND: // bitwise AND of accumulators A & B, result in A
        acc[0] = acc[0] & acc[1];
        return;
  
      case OR: // bitwise OR of accumulators A & B, result in A
        acc[0] = acc[0] | acc[1];
        return;
  
      case XOR: // bitwise XOR of accumulators A & B, result in A
        acc[0] = acc[0] ^ acc[1];
        return;
  
      case COMA: // bitwise complement, accumulator A
        acc[0] = !acc[0];
        return;
  
      case COMB: // bitwise complement, accumulator B
        acc[1] = !acc[1];
        return;
  
      case ROLA: // rotate left accumulator A
        ROL(0);
        return;
  
      case RORA: // rotate right accumulator A
        ROR(0);
        return;
  
      case ROLB:  // rotate left accumulator B
        ROL(1);
        return;
  
      case RORB: // rotate right accumulator B
        ROR(1);
        return;
  
      case LSL: // logical shift left
        setFlag(CARRY, acc[0] & 0x80);
        acc[0] = acc[0] << 1;
        acc[0] = acc[0] | (acc[1]  & 0x80);
        acc[1] = acc[1] << 1;
        return;
  
      case LSR: // shift right through both accumulators 0 -> A -> B -> CARRY
  
        setFlag(CARRY, acc[1] & 1);
        // uint8_t val8 = acc[1] & 1; // get 0th bit of acc B
        acc[1] = acc[1] >> 1; // shift right
        acc[1] = ((acc[0] & 1) << 7) | acc[1]; // move lsb of acc A across
        acc[0] = acc[0] >> 1;
        setFlag(ZERO, acc[0] == 0);
        return;
  
      case EORAi:
        EORi(0);
        return;
  
      case EORBi:
        EORi(1);
        return;
  
      case SWAP: // swap values between accumulator A & B
        val8 = acc[0];
        acc[0] = acc[1];
        acc[1] = val8;
        return;
  
      // ############# status flag ops #############
  
      // 76543210
      // ----CZVN
  
      case CLRS: // clear status flags
        status = 0;
        return;
  
      case SETS: // set status (immediate)
        status = program[++pc];
        return;
  
      case SETC: // set carry
        setFlag(CARRY, true);
        return;
  
      case CLC: // clear carry
        setFlag(CARRY, false);
        return;
  
      case CLV: // clear overflow
        setFlag(OVERFLOW, false);
        return;
  
      // Increment operators : affect ZNO
  
      /*
        #define DECA 0xD7 // increment accumulator A
        #define DECB 0xD8 // increment accumulator B
        #define DECa 0xD9 // increment absolute address
        #define DECx 0xDA // increment indexed address
        // #define DECS 0xDB  // increment PC Stack pointer
        #define DEXS 0xDC  // increment Auxiliary Stack pointer
        #define DECX 0xDD // increment Index Register
      */
  
      case INCA:
        if (acc[0] == 0xFF) setFlag(OVERFLOW, true);
        acc[0]++;
        doFlags(acc[0]);
        return;
  
      case DECA:
        if (acc[0] == 0xFF) setFlag(OVERFLOW, true);
        acc[0]++;
        doFlags(acc[0]);
        return;
  
      case INCB:
        if (acc[1] == 0xFF) setFlag(OVERFLOW, true);
        acc[1]++;
        doFlags(acc[1]);
        return;
  
      case INCa:
        val16 = readAbsoluteAddr();
        if (program[val16] == 0xFF) {
          setFlag(OVERFLOW, true);
        }
        program[val16]++;
        doFlags(program[val16]);
        return;
  
      case INCx:
        val16 = xReg;          // start with the index register value
        val16 += program[++pc];              // add the next byte in the program
        val8 = program[val16];               // look up the value at the total
        if (program[val16] == 0xFF) {
          setFlag(OVERFLOW, true);
        }
        program[val16]++;
        doFlags(program[val16]);
        return;
  
      case INXS:
        if (xStackP == 0xFF) {
          setFlag(OVERFLOW, true);
        }
        xStackP++;
        doFlags(xStackP);
        return;
  
      case INCX:
        if (xReg == 0xFFFF) {
          setFlag(OVERFLOW, true);
        }
        xReg++;
        doFlags(xReg);
        return;
  
      /*
           bits 7 and 6 of operand are transfered to bit 7 and 6 of SR (N,V);
           the zeroflag is set to the result of operand AND accumulator.
      */
      case BITAi: // memory contents AND acc, immediate, only status affected
        BITi(0);
        return;
  
      case BITAa: // memory contents AND acc, absolute, only status affected
       BITa(0);
        return;
  
      case BITAx: // memory contents AND acc, indexed, only status affected
      BITx(0);
        return;
  
      case BITBi: // memory contents AND acc, immediate, only status affected
      BITi(1);
        return;
  
      case BITBa: // memory contents AND acc, absolute, only status affected
        BITa(1);
        return;
  
      case BITBx: // memory contents AND acc, indexed, only status affected
        BITx(0);
        return;
  
      // ############# Auxiliary Stack-related #############
  
      case PUSHXA: // push value in accumulator A to top of ALU Stack
        pushX(0);
        return;
  
      case POPXA: // pop value from top of ALU Stack into accumulator A
        popX(0);
        return;
  
      case PUSHXB: // push value in accumulator B to top of X Stack
        pushX(1);
        return;
  
      case POPXB: // pop value from top of ALU Stack into accumulator B
        popX(1);
        return;
  
      case SWAPS: // a b c => b a c
        swapX();
        return;
  
      case DUP: // a b c => a a b c
        dupX();
        return;
  
      case OVER: // a b c => b a b c
        if (xStackP >= X_STACK_SIZE - 1) {
          showError("OvEr");
        }
        xStackP++;
        xStack[xStackP] = xStack[xStackP - 2];
        return;
  
      case ROT: //  a b c => c a b
        val8 = xStack[xStackP];
        xStack[xStackP] = xStack[xStackP - 2];
        xStack[xStackP - 2] = xStack[xStackP - 1];
        xStack[xStackP - 1] = val8;
        showError("tESt");
        return;
  
      case DROP: // a b c => b c
        if (xStackP <= 0) {
          showError("Undr");
        }
        xStackP--;
        showError("tESt");
        return;
  
      case TUCK: // a b c => a b a c
        swapX();
        dupX();
        showError("tESt");
        return;
  
  /*
   * 
  case LSPi: // load Stack Pointer, immediate
   // xStackP =
  return;
  
  case LSPa: // load Stack Pointer, absolute
  return;
  
  case LSPx: // load Stack Pointer, indexed
  return;
  
  case LDXi: // load Index Register, immediate
  return;
  
  case LDXai: // load Index Register, absolute
  return;
  
  case LDXx: // load Index Register, indexed
  return;
  
   */
      // PC Stack
      // unconditional jumps
      // subroutine jumps
      // conditional, relative branches
      // arithmetic
      // logical
      // increment/decrement registers
      // hardware-related
  
      // ################# fun
      case TEMPO:
        tempo = program[++pc];
        return;
  
      case TONE:
        doToneI();
        return;
  
      case TONEAB:
        doToneAB();
        return;
  
      case TONEx:
        doToneX();
        return;
  
      case REST:
        doRest();
        return;
  
      // ################# debugging/testing
  
      case RESET:
        flashMessage("reset");
        doReset();
        return;
  
      case TEST:
        //   resetRegisters();
        // testFlags();
        return;
  
      case DUMP:
        flashMessage("dump");
        Serial.begin(9600);
        while (!Serial) {
          // wait for serial port to connect. Needed for native USB port only
        }
        delay(1);
        Serial.println("<");
        Serial.println("{");
        Serial.print("\"pc\": ");
        Serial.println(pc);
        Serial.print(",\n\"status\": ");
        Serial.println(status & 0x0F); // ignore extra flags for now
        Serial.print(",\n\"accA\": ");
        Serial.println(acc[0]);
        Serial.print(",\n\"accB\": ");
        Serial.println(acc[1]);
        Serial.print(",\n\"xReg\": ");
        Serial.println(xReg);
        Serial.print(",\n\"pcStackP\": ");
        Serial.println(pcStackP);
        Serial.print(",\n\"xStackP\": ");
        Serial.println(xStackP);
        Serial.println("}");
        Serial.println(">");
        // delay(1000); // wait a sec
        // Serial.end();
        // resetRegisters();
        mode = PROG_MODE;
        flashMessage("dumped");
        newData = false;
        resetRegisters();
        readyToReceive = true;
        // pc = 0;
        return;
  
      case DEBUG:// flips the state of debug
        debugOn = !debugOn;
        return;
  
      case RND: // load accumulators A & B with random values
        pause = true;
        return;
  
      case PAUSE: //
        pause = true;
        displayHex(4, 2, acc[0]);
        displayHex(6, 2, acc[1]);
        waitForButton();
        return;
  
      case OK:
        tm.displayText("- OK -");
        waitForButton();
        return;
  
      case ERR:
        showError("Err.");
        return;
  
      // ## and finally... ##
  
      case HALT:
        if (!pause) {
          tm.displayASCII (4, 'r') ; // 'run done'
          tm.displayASCII (5, 'd') ;
        }
        waitForButton(); // pause would be better here
        mode = PROG_MODE;
        runMode = STEP;
        pc = 0;
        tm.displayASCII (0, 'S') ;
        waitForButton();
        return;
  
      default:
        showError("noPE");
    }
  }
  
  /**
     #############################################################
     ##################### Operation Helpers #####################
     #############################################################
  */

  
void doTone(uint8_t noteIndex, uint8_t duration) {
    if (noteIndex < (sizeof(noteFrequencies) / sizeof(noteFrequencies[0]))) {
        uint16_t frequency = noteFrequencies[noteIndex];
        Serial.print("Playing note index: ");
        Serial.print(noteIndex);
        Serial.print(" (Freq: ");
        Serial.print(frequency);
        Serial.print(") for duration: ");
        Serial.println(duration);
    delay(duration * 10);
            } else {
        Serial.print("Error: Invalid note index: ");
        Serial.println(noteIndex);
        delay(duration * 10);
            }
    }

void processNextProgramStep() {
    if (pc >= (sizeof(program) / sizeof(program[0]))) {
        pc = 0;
        return;
}
}
void swapX() {
    uint8_t temp = xStack[xStackP - 1];
    xStack[xStackP - 1] = xStack[xStackP];
    xStack[xStackP] = temp;
  }
  
  void dupX() {
    if (xStackP >= X_STACK_SIZE - 1) {
      showError("OvEr");
    }
    xStackP++;
    xStack[xStackP] = xStack[xStackP - 1];
  }


void showError(String message) {
    tm.displayText("    " + message);
    displayPC();
    waitForButton();
    mode = PROG_MODE;
  }
  
  /*
    uint8_t instruction = program[pc];

    switch (instruction) {
        case NOP:
            pc++;
            break;
        case STOP:
            break;
        case PLAYN:
            if (pc + 2 < sizeof(program) / sizeof(program[0])) {
                uint8_t noteIndex = program[pc + 1];
                uint8_t duration = program[pc + 2];
                doTone(noteIndex, duration);
                pc += 3;
            } else {
                pc = sizeof(program) / sizeof(program[0]);
            }
            break;
        default:
            pc++;
            break;
    }
}
*/
/**
   System mode displays, 4 & 5
*/
void displayMode() {
    if (mode == PROG_MODE) {
      tm.displayASCII(4, 'P');
      if (incdec) {
        tm.displaySS(5, 1); // just the top segment
      } else {
        tm.displaySS(5, 8); // just the tbottom segment
      }
    } else { // RUN_MODE
      tm.displayASCII(4, 'R');
      if (runMode == RUN) {
        tm.displayASCII(5, 'R'); // Run
      } else {
        tm.displayASCII(5, 'S'); // Step
      }
    }
  }
  

void flashMessage(String message) {
  tm.displayText(message);
  delay(1000);
}


void flashMessage(long value) {
  displayHex(4, 4, value);
  delay(1000);
}

void displayCode() {
  displayHex(6, 2, program[pc]);
}

void displayPC() {
  displayHex(0, 4, pc);
}

/**
   ##################### LEDs #####################################
*/
void showStatus() {
  uint8_t shifty = status;
  for (uint8_t i = 0; i < 8; i++) {
    tm.setLED(i, shifty & 1);
    shifty = shifty >> 1;
  }
}

/**
    Display hex values
*/
void displayHex(uint8_t startDisplay, uint8_t nBytes, long data) {

  for (uint8_t i = 0; i < nBytes; i++) { // I tried decrementing, but got errors every time
    if (data > 0) {
      tm.displayHex(nBytes - i + startDisplay - 1, data % 16);
      data = data / 16;
    } else {
      tm.displayHex(nBytes - i + startDisplay - 1, 0);
    }
  }
}

  /////////////////////////////////////////////
  ////////////////////////////////////////////////
  ///////////////////////////////////////////
  
void LDi(uint8_t id) {             // Load accumulator <id> immediate
  acc[id] = program[++pc]; // move to next position in program, load into acc
  setFlag(OVERFLOW, false);
  doFlags(acc[id]);
}

void LDa(uint8_t id) { // Load accumulator <id> absolute
  acc[id] = program[readAbsoluteAddr()]; // read next 2 bytes, lookup the value at that address
}

void LDx(uint8_t id) { // Load accumulator A indexed (6502 calls it Indexed Indirect)
  unsigned long addr = xReg;          // start with the index register value
  addr += program[++pc];              // add the next byte in the program
  acc[id] = program[addr];               // look up the value at the total
}

void STa(uint8_t id) {  // Store accumulator <id> absolute
  program[readAbsoluteAddr()] = acc[id]; // read next 2 bytes, store acc value at that address
}

void STx(uint8_t id) { // Store accumulator <id> indexed (6502 calls it Indexed Indirect)
  unsigned long addr = xReg;          // start with the index register value
  addr += program[++pc];              // add the next byte in the program
  program[addr] = acc[id];               // store acc value at the total
}

void ROL(uint8_t id) { // rotate left accumulator <id>
  uint8_t temp = acc[id] & 128; // get 7th bit
  acc[id] = acc[id] << 1; // shift left
  acc[id] = acc[id] + (status & CARRY); // load carry flag to bit 0
  status = status | temp; // put previous 7th bit in carry flag
}

void ROR(uint8_t id) { // rotate left accumulator <id>
  uint8_t temp = acc[id] & 1; // get 0th bit
  acc[id] = acc[id] >> 1; // shift right
  acc[id] = acc[id] + 128 * (status & CARRY); // load carry flag to bit 7
  status = status | temp; // put previous 7th bit in carry flag
}

void EORi(uint8_t id) { // EXOR
  acc[id] = acc[id] ^ program[++pc]; // move to next position in program, exor with acc acc
  setFlag(OVERFLOW, false);
  doFlags(acc[id]);
}

// ####################################################################################


void BIT(uint8_t id, uint8_t val){
  setFlag(NEGATIVE, val & 128);
  setFlag(OVERFLOW, val & 64); // wrong!?
  if (val && acc[id] == 0) {
    setFlag(ZERO, true);
  } else {
    setFlag(ZERO, false);
  }
}


void BITi(uint8_t id) {
  uint8_t val = program[pc++];
BIT(id, val);
}

void BITa(uint8_t id) {
uint8_t val = program[readAbsoluteAddr()];
  BIT(id, val);
}


void BITx(uint8_t id) {
  uint8_t val = program[xReg];
BIT(id, val);
}


void popX(uint8_t id) {
  if (xStackP <= 0) {
    showError("Undr");
  }
  acc[id] = xStack[--xStackP];
}


void doToneI() {
  uint8_t rawNote = program[++pc];
  uint8_t rawDuration = program[++pc];
  doTone(rawNote, rawDuration);
}

void doToneAB() {
  doTone(acc[0], acc[1]);
}

void doToneX() {
  unsigned int addr = readAbsoluteAddr();
  doTone(program[addr], program[addr + 1]);
}

void doRest() {
  unsigned long duration = program[++pc] * 6000 / tempo;
  delay(duration);
}

void doReset() {
  asm volatile ("  jmp 0");
}

void pushX(uint8_t id) {
  if (xStackP >= X_STACK_SIZE - 1) {
    showError("OvEr");
  }
  xStack[xStackP++] = acc[id];
}


void debug(long value) {
  if (debugOn) flashMessage(value);
}

void debug(String message) {
  if (debugOn) flashMessage(message);
}


void doFlags(uint16_t val) {
  setFlag(ZERO, val == 0); /////////////////////////// acc[id] == 0
  setFlag(NEGATIVE, val & 128);
}

uint16_t readLong(uint16_t startPos) {
  uint8_t hi = hexCharToValue(buffer[startPos]);
  uint8_t lo = hexCharToValue(buffer[startPos + 1]);
  return hi * 16 + lo;
}

/**
   read two bytes from program (updating PC 2), combine & return
*/
uint16_t readAbsoluteAddr() {
  uint8_t lo = program[++pc];
  uint8_t hi = program[++pc];
  return (hi << 8) + lo;
}

uint8_t hexCharToValue(uint8_t hexChar) {
  if (hexChar >= 48 && hexChar <= 57) return hexChar - 48; // '0'...'9' -> 0...9
  if (hexChar >= 65 && hexChar <= 70) return hexChar - 55; // 'A'...'F' -> 10...15
  if (hexChar >= 97 && hexChar <= 102) return hexChar - 87; // 'a'...'f' -> 10...15
  showError("Char");
  mode = PROG_MODE;
  return 48; // pointless?
}
