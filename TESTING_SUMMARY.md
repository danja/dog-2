# Testing Setup Summary

## What Was Created

I've set up a comprehensive testing framework for DOG-2 using PlatformIO's built-in test infrastructure. Here's what's ready to use:

### Documentation
- **TEST_STRATEGY.md** - Comprehensive testing strategy and philosophy
- **TESTING.md** - Quick reference for running tests and TDD workflow
- **TESTING_SUMMARY.md** - This file

### Configuration
- **platformio.ini** - Updated with `test_build_src = yes` and test speed settings

### Test Infrastructure
- **test/test_helpers.h** - Shared test utilities and macros for cleaner tests

### Test Suites (4 files, ~50+ test cases)

1. **test/test_embedded/test_flags.cpp** (11 tests)
   - Status flag set/clear operations
   - Flag-related opcodes (CLRS, SETC, CLC, CLV)
   - **Will expose setFlag() bug** (uses `!mask` instead of `~mask`)

2. **test/test_embedded/test_opcodes.cpp** (16 tests)
   - Load/store immediate and absolute
   - Increment/decrement operations
   - **Will expose DECA bug** (uses `++` instead of `--`)
   - Logical operations (AND, OR, XOR)
   - Swap and clear operations

3. **test/test_embedded/test_branches.cpp** (15 tests)
   - Unconditional branches (BRA)
   - Conditional branches (BZS, BZC, BCS, BCC)
   - **2's complement offset testing** (positive and negative)
   - Edge cases (max positive/negative offsets)
   - Loop integration test

4. **test/test_embedded/test_stack.cpp** (15 tests)
   - Push/pop operations (PUSHXA, POPXA, PUSHXB, POPXB)
   - Forth-style stack manipulation (DUP, SWAP, OVER, ROT, DROP, TUCK)
   - Stack overflow/underflow detection
   - RPN calculator integration test

## Critical Bugs These Tests Will Expose

### Bug 1: setFlag() bitwise negation (CRITICAL)
**Location**: src/core.cpp:367
**Problem**: Uses `!mask` (logical NOT) instead of `~mask` (bitwise NOT)
**Impact**: Clearing flags doesn't work properly
**Tests that fail**: `test_clear_zero_flag`, `test_clear_carry_flag`

**Fix**:
```cpp
// Line 367 - BEFORE:
status = status & !mask;

// Line 367 - AFTER:
status = status & ~mask;
```

### Bug 2: DECA uses increment instead of decrement
**Location**: src/core.cpp:602
**Problem**: `acc[0]++` should be `acc[0]--`
**Impact**: DECA increments instead of decrements
**Tests that fail**: `test_deca_decrements`

**Fix**:
```cpp
// Line 602 - BEFORE:
case DECA:
    if (acc[0] == 0xFF) setFlag(OVERFLOW, true);
    acc[0]++;  // WRONG!
    doFlags(acc[0]);
    return;

// AFTER:
case DECA:
    if (acc[0] == 0x00) setFlag(OVERFLOW, true);
    acc[0]--;  // CORRECT
    doFlags(acc[0]);
    return;
```

### Bug 3: runMode assignment uses == instead of =
**Location**: src/core.cpp:226
**Problem**: `runMode == RUN` is a comparison, not assignment
**Impact**: Compiler warning, no effect
**Tests**: Not directly tested, but would prevent mode switching

**Fix**:
```cpp
// Line 226 - BEFORE:
runMode == RUN; // compiler says it has no effect TODO

// AFTER:
runMode = RUN;
```

## How to Run Tests

### Quick Start
```bash
# Run all tests on connected Arduino
~/.platformio/penv/bin/pio test -e nanoatmega328

# Create an alias for convenience
alias piotest='~/.platformio/penv/bin/pio test'
piotest -e nanoatmega328
```

### Run Specific Test Files
```bash
# Test flags only
~/.platformio/penv/bin/pio test -e nanoatmega328 -f test_flags

# Test opcodes only
~/.platformio/penv/bin/pio test -e nanoatmega328 -f test_opcodes

# Test branches only
~/.platformio/penv/bin/pio test -e nanoatmega328 -f test_branches

# Test stack operations only
~/.platformio/penv/bin/pio test -e nanoatmega328 -f test_stack
```

### Verbose Output
```bash
~/.platformio/penv/bin/pio test -e nanoatmega328 -v
```

## Recommended Workflow

### 1. First Test Run (See Current State)
```bash
~/.platformio/penv/bin/pio test -e nanoatmega328
```

**Expected Result**: Some tests will FAIL due to the 3 bugs above.

### 2. Fix Bug #1 (setFlag - Most Critical)
- Edit src/core.cpp:367
- Change `!mask` to `~mask`
- Save file

### 3. Verify Fix
```bash
~/.platformio/penv/bin/pio test -e nanoatmega328 -f test_flags
```

**Expected Result**: All flag tests should now PASS.

### 4. Fix Bug #2 (DECA)
- Edit src/core.cpp:602
- Change `acc[0]++` to `acc[0]--`
- Fix overflow check from `0xFF` to `0x00`

### 5. Verify Fix
```bash
~/.platformio/penv/bin/pio test -e nanoatmega328 -f test_opcodes
```

### 6. Fix Bug #3 (runMode)
- Edit src/core.cpp:226
- Change `runMode == RUN` to `runMode = RUN`

### 7. Run Full Test Suite
```bash
~/.platformio/penv/bin/pio test -e nanoatmega328
```

**Expected Result**: All ~57 tests should PASS (if hardware is working).

### 8. Commit Your Fixes
```bash
git add src/core.cpp test/
git commit -m "Fix critical bugs: setFlag bitwise NOT, DECA decrement, runMode assignment

- Fixed setFlag() to use ~mask instead of !mask for bitwise negation
- Fixed DECA to decrement instead of increment
- Fixed runMode assignment to use = instead of ==
- Added comprehensive test suite with 57 tests covering flags, opcodes, branches, and stack operations
- All tests now pass"
```

## Next Steps After Initial Fixes

1. **Test on Real Hardware** - Verify TM1638 display works
2. **Add Serial Tests** - Test program upload functionality
3. **Add Timing Tests** - Test flow control and button handling
4. **Add Integration Tests** - Test complete assembly programs
5. **Test EEPROM** - Test persistence across resets
6. **Address Remaining TODOs** - From docs/todo.md

## Test-Driven Development for New Features

When adding new features or fixing new bugs:

1. **Write the test first** (it should fail)
2. **Implement the feature**
3. **Run the test** (it should pass)
4. **Run all tests** (no regressions)
5. **Commit test and implementation together**

## Troubleshooting

### Tests hang or don't complete
- Check serial connection
- Try resetting the Arduino
- Check monitor_speed and test_speed match (9600)

### "No such file or directory" errors
- Run from project root directory
- Check that src/core.cpp and src/core.h exist

### Tests fail unexpectedly
- Check that hardware is connected
- Try running with `-v` flag for verbose output
- Check serial monitor for detailed error messages

### Can't find pio command
```bash
# Use full path
~/.platformio/penv/bin/pio test -e nanoatmega328

# Or add to PATH in ~/.bashrc:
export PATH="$HOME/.platformio/penv/bin:$PATH"
```

## Success Criteria

After following this testing setup:

✅ Test infrastructure is in place and working
✅ 3 critical bugs are identified and can be verified
✅ Tests can run on actual hardware over USB
✅ TDD workflow is established for future development
✅ Regression testing prevents reintroduction of bugs
✅ Code quality improves through systematic testing

## Files Created

```
test/
├── test_helpers.h              # Test utilities
├── test_embedded/
│   ├── test_flags.cpp         # Status flag tests (11 tests)
│   ├── test_opcodes.cpp       # Basic opcode tests (16 tests)
│   ├── test_branches.cpp      # Branch operation tests (15 tests)
│   └── test_stack.cpp         # Stack operation tests (15 tests)
platformio.ini                  # Updated with test configuration
TEST_STRATEGY.md                # Comprehensive testing strategy
TESTING.md                      # Quick reference guide
TESTING_SUMMARY.md              # This summary
```

Total: **57 test cases** ready to run on your Arduino Nano!
