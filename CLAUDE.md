# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

DOG-2 (Danny's Obtuse Gadget v2) is a 2025 reboot of DOG-1, a 1970s-style single-board computer inspired by the KIM-1 and MK14. It runs as a virtual machine on an Arduino Nano (ATmega328P) with a TM1638 I/O card for user interface (8 LEDs, 8 seven-segment displays, 8 buttons).

This is an embedded systems project implementing a custom instruction set architecture with:
- 16-bit addressing
- 8-bit instructions and data
- Two 8-bit accumulators (A & B)
- Custom assembly language with ~100+ opcodes
- Stack-oriented programming support

**Current Status**: Code compiles but hardware validation is pending (display may be faulty after 7 years in storage).

## Build & Development Commands

### Build
```bash
pio run
```

### Upload to Arduino
```bash
pio run --target upload
```

### Clean build
```bash
pio run --target clean
```

### Monitor serial output
```bash
pio device monitor
```

### Build and upload in one command
```bash
pio run --target upload && pio device monitor
```

## Hardware Configuration

- **Target Board**: Arduino Nano (ATmega328P)
- **Framework**: Arduino
- **TM1638 Pins**: STB=7, CLK=10, DIO=11 (defined in src/main.cpp:5-7)
- **Speaker Pin**: 9 (for TONE opcodes)
- **Serial Baud**: 9600

## Architecture Overview

### Memory Layout
- `program[MAX_PROG_SIZE]`: 128 bytes of program memory
- `pcStack[PC_STACK_SIZE]`: 64-byte PC/subroutine stack
- `xStack[X_STACK_SIZE]`: 64-byte auxiliary stack for stack-oriented operations
- EEPROM: Used for persistent program storage

### Registers (defined in core.h)
**16-bit**:
- `pc`: Program Counter
- `xReg`: Index Register
- `pcStackP`: PC Stack Pointer

**8-bit**:
- `acc[2]`: Accumulators A (acc[0]) and B (acc[1])
- `status`: Status register with flags (bits 0-3: NEGATIVE, OVERFLOW, ZERO, CARRY)
- `xStackP`: Auxiliary Stack Pointer

### Addressing Modes
The instruction set supports 5 addressing modes (similar to 6800/6502):
1. **Immediate** (i): Value follows opcode - e.g., `LDAi 0xF6`
2. **Absolute** (a): 2-byte address follows opcode - e.g., `STAa 0x99 0x01`
3. **Indexed** (x): Offset added to xReg - e.g., `LDAx 0x10`
4. **Accumulator**: Operations on accumulators - e.g., `ABA`, `SWAP`
5. **Relative**: For branches using 2's complement offset - e.g., `BZS 0xFE` (branch -2)

### Operating Modes
- **PROG_MODE**: Program editing mode (navigate with buttons 0-3, edit with buttons 6-7)
- **RUN_MODE**: Execution mode (STEP or RUN)
- Mode toggled with button 4

## Code Structure

### Main Entry Points
- **src/main.cpp**: Arduino setup() and loop()
  - `setup()`: Initializes serial, TM1638, registers, loads from EEPROM
  - `loop()`: Main execution loop - processes program steps, handles buttons, updates display

### Core Implementation (src/core.cpp & core.h)
- **doOperation()** (core.cpp:381): Main opcode dispatch switch statement - handles all ~100+ opcodes
- **Instruction Categories**:
  - Load/Store: LDAi/LDBi/STAa/STBa family
  - Arithmetic: ABA, ADDAi, SUBAi, INCA/DECA
  - Logic: AND, OR, XOR, COMA/COMB, ROL/ROR, LSL/LSR
  - Branches: BRA, BZS/BZC, BCS/BCC, BVS/BVC, BGE/BGT/BLT
  - Stack ops: PUSHXA/POPXA, SWAPS, DUP, OVER, ROT, DROP, TUCK
  - Sound: TONE, TONEAB, TONEx, REST, TEMPO
  - Debug: PAUSE, DUMP, DEBUG, TEST, OK, ERR, HALT

### Key Functions
- **handleButtons()** (core.cpp:154): Button input processing - dual-button combos for system functions
- **display()** (core.cpp:338): Updates TM1638 display showing PC, opcode, and mode
- **translateProg()** (core.cpp:103): Converts hex buffer from serial into program memory
- **loadFromEEPROM()** (core.cpp:57): Loads program from EEPROM
- **Helper functions**: LDi/LDa/LDx, STa/STx, ROL/ROR, BIT operations, stack operations

### Display Layout (8 seven-segment digits)
```
[0][1][2][3]  [4][5]  [6][7]
    PC         Mode     Opcode
```
- Digits 0-3: Program Counter (hex)
- Digits 4-5: Mode indicator (P=Program, R=Run, up/down arrow for inc/dec)
- Digits 6-7: Current opcode/instruction (hex)

### Button Controls
**Dual-button combinations** (must press simultaneously):
- 0+1: Full reset & wipe
- 0+2: Load from EEPROM
- 4+5: Reset PC to 0
- 0+4: Display accumulators A & B
- 0+5: Display index register
- 0+6: Display EEPROM at PC
- 0+7: Display stack pointer & status
- 0+3 (in RUN_MODE): Toggle single-step/free-run

**Single buttons**:
- 4: Toggle PROG_MODE/RUN_MODE
- 5 (in PROG_MODE): Toggle increment/decrement
- 0-3: Navigate PC (button 3=±1, 2=±16, 1=±256)
- 6-7: Edit opcode at PC (button 6=high nibble, 7=low nibble)

## Opcode Definitions

All opcodes are defined as #define constants in src/core.h (lines 8-167). The naming convention follows addressing mode suffixes:
- `i` = immediate
- `a` = absolute
- `x` = indexed
- No suffix = accumulator/implied

Examples:
- `LDAi 0x10`: Load A immediate
- `LDAa 0x11`: Load A absolute
- `STAa 0x14`: Store A absolute
- `BZS 0x68`: Branch if zero set

See docs/opcodes.md and docs/manual.md for complete instruction reference.

## Development Notes

### Serial Protocol
Programs can be uploaded via USB serial (9600 baud). Format is ASCII hex:
- First 4 hex chars: Starting address (2 bytes)
- Remaining chars: Program bytes as hex pairs
- Buffer size: 2 * MAX_PROG_SIZE (256 bytes)

### Known Issues (from README)
- Flow control timing issues - may require button fumbling
- Loading long programs can be problematic
- Some branch opcodes have issues with 2's complement arithmetic
- Line 226 in core.cpp: Compiler warning about `runMode == RUN` having no effect (should be `=`)

### Flag Operations
Status register layout (bits 0-3 used):
- Bit 0: NEGATIVE (N) - set if bit 7 of result is set
- Bit 1: OVERFLOW (V) - set on arithmetic overflow
- Bit 2: ZERO (Z) - set if result is zero
- Bit 3: CARRY (C) - set on carry/borrow

Critical bug in setFlag() (core.cpp:367): Uses `!mask` instead of `~mask` for bitwise negation when clearing flags.

### Dependencies
All defined in platformio.ini:
- TM1638lite library: https://github.com/danja/TM1638lite
- Dog library: https://github.com/danja/dog.git
- ArduinoSTL: Provides C++ standard library support on AVR

### Testing
The TEST opcode (0xF9) can be used to trigger test mode. The DUMP opcode (0xF8) outputs register state as JSON over serial for debugging.

## Assembly Programming

Example program (from README):
```
LDAi F6      ; Load 0xF6 into accumulator A
STAa 99 01   ; Store A at address 0x0199
LDBa 99 01   ; Load B from address 0x0199
CAB          ; Compare A with B
BZS 01       ; Branch forward 1 if zero set
ERR          ; Display error
OK           ; Display OK
HALT         ; Stop execution
```

Note: Branches use 2's complement relative offsets from the **next** instruction.
