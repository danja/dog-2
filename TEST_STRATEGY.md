# DOG-2 Testing Strategy

## Overview

This document outlines the testing approach for DOG-2, focusing on on-device testing with PlatformIO and the Arduino Nano connected over USB.

## Testing Approaches

### 1. On-Device Testing (Primary Method)

Tests run on the Arduino itself and report results over serial. This is ideal for:
- Testing actual opcode execution
- Verifying flag operations
- Testing hardware interactions (TM1638, EEPROM)
- Integration testing of the full VM

**Pros**: Tests real hardware behavior, catches timing issues
**Cons**: Slower than native tests, requires device connection

### 2. Native Testing (Secondary Method)

Tests run on your development machine. Good for:
- Pure logic functions (hexCharToValue, readAbsoluteAddr)
- Flag manipulation algorithms
- Fast iteration during development

**Pros**: Very fast, no hardware needed
**Cons**: Can't test Arduino-specific code, hardware, or EEPROM

## Setup Instructions

### Step 1: Update platformio.ini

Add test configuration to your `platformio.ini`:

```ini
[env:nanoatmega328]
platform = atmelavr
board = nanoatmega328
framework = arduino
lib_deps =
    https://github.com/danja/dog.git
    danja/TM1638lite@^1.1
    ArduinoSTL
test_build_src = yes  ; Include src/ files in test builds
monitor_speed = 9600
test_speed = 9600

[env:native]
platform = native
test_build_src = yes
build_flags =
    -std=c++11
    -DUNIT_TEST
```

### Step 2: Create Test Directory Structure

```
test/
├── test_embedded/          # Tests that run on device
│   ├── test_flags.cpp      # Status flag operations
│   ├── test_opcodes.cpp    # Individual opcode tests
│   ├── test_arithmetic.cpp # Arithmetic operations
│   ├── test_branches.cpp   # Branch/jump operations
│   └── test_stack.cpp      # Stack operations
├── test_native/            # Tests that run on PC
│   ├── test_helpers.cpp    # Pure functions
│   └── test_addressing.cpp # Address calculation
└── test_integration/       # Full program tests
    └── test_programs.cpp   # Complete assembly programs
```

### Step 3: Create Test Helper

Create `test/test_helpers.h` for shared test utilities:

```cpp
#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <unity.h>

// Test execution macros
#define RUN_OPCODE(opcode) \
    program[pc] = opcode; \
    doOperation(); \
    pc++;

#define SET_PROGRAM(...) \
    uint8_t prog[] = {__VA_ARGS__}; \
    for(int i = 0; i < sizeof(prog); i++) program[i] = prog[i];

// Assertion helpers
#define ASSERT_FLAG_SET(flag) \
    TEST_ASSERT_TRUE_MESSAGE(getFlag(flag), #flag " should be set")

#define ASSERT_FLAG_CLEAR(flag) \
    TEST_ASSERT_FALSE_MESSAGE(getFlag(flag), #flag " should be clear")

#define ASSERT_ACC_A(expected) \
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(expected, acc[0], "Accumulator A")

#define ASSERT_ACC_B(expected) \
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(expected, acc[1], "Accumulator B")

// Test isolation
inline void reset_test_state() {
    resetRegisters();
    for(int i = 0; i < MAX_PROG_SIZE; i++) program[i] = 0;
}

#endif
```

## Running Tests

### Run all tests on device:
```bash
~/.platformio/penv/bin/pio test -e nanoatmega328
```

### Run specific test file:
```bash
~/.platformio/penv/bin/pio test -e nanoatmega328 -f test_flags
```

### Run native tests (fast iteration):
```bash
~/.platformio/penv/bin/pio test -e native
```

### Run with verbose output:
```bash
~/.platformio/penv/bin/pio test -e nanoatmega328 -v
```

### Monitor serial output during tests:
```bash
~/.platformio/penv/bin/pio test -e nanoatmega328 --without-uploading && \
~/.platformio/penv/bin/pio device monitor
```

## Test Priority by Issue

### Critical Bugs (Fix First)
1. **setFlag() bitwise negation** - Test file: `test_flags.cpp`
2. **DECA opcode** - Test file: `test_opcodes.cpp`
3. **runMode assignment** - Test file: `test_mode.cpp`

### Known Issues (Fix Next)
4. **Branch operations with 2's complement** - Test file: `test_branches.cpp`
5. **Flow control timing** - Test file: `test_integration/test_flow.cpp`
6. **Long program loading** - Test file: `test_serial.cpp`

## Test-Driven Development Workflow

1. **Write failing test** for the bug
2. **Run test** - verify it fails
3. **Fix the bug** in src/
4. **Run test** - verify it passes
5. **Run all tests** - ensure no regressions
6. **Commit** with test and fix together

## Continuous Testing

Set up a git hook to run tests before commits:

```bash
# .git/hooks/pre-commit
#!/bin/bash
~/.platformio/penv/bin/pio test -e nanoatmega328
if [ $? -ne 0 ]; then
    echo "Tests failed. Commit aborted."
    exit 1
fi
```

## Serial Monitor for Manual Testing

When running manual tests, use the DUMP opcode (0xF8) to inspect state:

```bash
# Monitor serial output
~/.platformio/penv/bin/pio device monitor

# Upload and monitor in one command
~/.platformio/penv/bin/pio run -t upload && ~/.platformio/penv/bin/pio device monitor
```

The DUMP opcode outputs JSON with all register states, perfect for debugging.

## Test Coverage Goals

- **100% coverage** of all opcodes with basic functionality tests
- **Edge cases** for each addressing mode
- **Flag operations** for all arithmetic/logic operations
- **2's complement** branch offsets (positive and negative)
- **Stack overflow/underflow** conditions
- **EEPROM persistence** across resets

## Next Steps

1. Create initial test structure (directories and helper files)
2. Write tests for critical bugs first
3. Fix bugs one-by-one with TDD approach
4. Expand test coverage to known issues
5. Add regression tests as new bugs are discovered
