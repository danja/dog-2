# DOG-2
'Danny's Obtuse Gadget v2'

This is a 2025 reboot of [Dog-1](https://github.com/danja/dog), updated for PlatformIO.

**Status 2025-12-06: Code compiles and 98% of tests pass! Major bug fixes completed.**

![DOG-1](https://github.com/danja/dog/blob/master/docs/dog-1.JPG?raw=true)

A 1970's-style single-board computer using a TM1638 card driven by an Arduino Nano. Inspired by the KIM-1 and MK14, with a custom instruction set.

## Quick Start

### Build and Upload

```bash
# Build the firmware
pio run

# Upload to Arduino Nano
pio run --target upload

# Monitor serial output
pio device monitor
```

### Run Tests

```bash
# Run all tests on connected device
~/.platformio/penv/bin/pio test -e test

# Run specific test
~/.platformio/penv/bin/pio test -e test -f test_flags
```

**Current test results: 56/57 tests passing (98.2%)**

### Upload a Program

First, install pyserial:

```bash
# Option 1: System package manager (recommended)
sudo apt install python3-serial

# Option 2: Virtual environment
python3 -m venv venv
source venv/bin/activate
pip install pyserial
```

Then upload a program:

```bash
# Auto-detect serial port
python python/upload.py -i dog-code/fibtone.hex

# Or specify port
python python/upload.py -i dog-code/fibtone.hex -p /dev/ttyUSB0
```

See [Fibonacci Tone Tutorial](docs/fibtone.md) for a complete example.

## Documentation

- **[CLAUDE.md](CLAUDE.md)** - Project overview, architecture, and development guide
- **[DOG-1 Manual](docs/manual.md)** - Instruction set and architecture reference
- **[Opcode Reference](docs/opcodes.md)** - Complete opcode listing
- **[Fibonacci Tutorial](docs/fibtone.md)** - Example program with step-by-step guide
- **[Testing Guide](TESTING.md)** - How to run and write tests
- **[Bug Tracker](docs/bugs.md)** - Known issues and fixes

## Recent Updates (2025-12-06)

### Major Bug Fixes
- ✅ Fixed `setFlag()` bitwise negation bug (was using `!mask` instead of `~mask`)
- ✅ Fixed DECA opcode (was incrementing instead of decrementing)
- ✅ Fixed all branch operations (PC offset calculation)
- ✅ Implemented missing opcodes: CLRA, CLRB, DEXS, ABA
- ✅ Fixed ROT stack operation
- ✅ Fixed runMode assignment

### Test Infrastructure
- Created comprehensive test suite with 57 tests
- Tests cover flags, opcodes, branches, and stack operations
- On-device testing via PlatformIO
- All tests documented in [TESTING.md](TESTING.md)

### Example Programs
- Added Fibonacci tone generator (`dog-code/fibtone.dog`)
- Complete tutorial for loading programs
- Improved upload script with auto-detection
- New self-test (`dog-code/selftest.ass` / `dog-code/selftest.hex`) that sanity-checks core opcodes and plays a short confirmation tune

## Hardware

- **Board**: Arduino Nano (ATmega328P)
- **Display/Input**: TM1638 (8 LEDs, 8 seven-segment displays, 8 buttons)
- **Pins**: STB=7, CLK=10, DIO=11, Speaker=9
- **Serial**: 9600 baud

## Architecture

- **16-bit addressing** with 8-bit instructions and data
- **Two 8-bit accumulators** (A & B)
- **Custom instruction set** (~100+ opcodes)
- **Stack-oriented programming** support
- **Five addressing modes**: Immediate, Absolute, Indexed, Accumulator, Relative

Memory:
- 128 bytes program memory
- 64-byte PC/subroutine stack
- 64-byte auxiliary stack for Forth-style operations
- EEPROM for persistent storage

## Example Program

```assembly
LDAi F6      ; Load 0xF6 into accumulator A
STAa 99 01   ; Store A at address 0x0199
LDBa 99 01   ; Load B from address 0x0199
CAB          ; Compare A with B
BZS 01       ; Branch forward 1 if zero set
ERR          ; Display error
OK           ; Display OK
HALT         ; Stop execution
```

## Historical Notes

**2025-04-20**: Rebooted project for PlatformIO. Code compiles but hardware needs verification (7-year-old display may be faulty).

**2019-01-05**: Resumed after long break. Fixed Arduino library compatibility with ArduinoSTL. Known issues: flow control timing, long program loading, branch opcode 2's complement issues.

**2018-07-16**: Fixed compilation typo. TONE demo had upload issues. Mode switching buggy - suspected timing errors.

**2018-04-24**: Added TONE opcode. Reworked serial interface (64-byte buffer issue). Created [DOG-1 Bachs!](https://youtu.be/eEgXBOtdvvg) video.

**2018-04-16**: Fixed TM1638 library mess and flag bugs (`!x` vs `~x` for bitwise negation - finally fixed in 2025!).

**2018-03-31**: ~40 opcodes implemented. Created minimal Python assembler.

**2018-03-30**: Implemented dedicated 8-bit stack for stack-oriented programming experiments.

**2018-03-29**: First program running! Made [intro video](https://youtu.be/qjk-y1qbj7w).

**2018-03-28**: Refactored TM1638 interface into separate library.

**2018-03-27**: UI and program input implemented.

## Contributing

See the comprehensive test suite in `test/` directory. Run tests before submitting changes:

```bash
~/.platformio/penv/bin/pio test -e test
```

## License

See LICENSE file for details.
