# Quick Testing Reference

## Running Tests

All commands assume you're in the project root directory.

### Run all tests on the connected device:
```bash
~/.platformio/penv/bin/pio test -e nanoatmega328
```

### Run a specific test file:
```bash
# Run only flag tests
~/.platformio/penv/bin/pio test -e nanoatmega328 -f test_flags

# Run only opcode tests
~/.platformio/penv/bin/pio test -e nanoatmega328 -f test_opcodes

# Run only branch tests
~/.platformio/penv/bin/pio test -e nanoatmega328 -f test_branches
```

### Verbose output (see detailed test execution):
```bash
~/.platformio/penv/bin/pio test -e nanoatmega328 -v
```

### Create an alias for convenience:
```bash
# Add to your ~/.bashrc or ~/.zshrc
alias piotest='~/.platformio/penv/bin/pio test'

# Then you can just use:
piotest -e nanoatmega328
```

## Test Files Created

1. **test/test_embedded/test_flags.cpp** - Tests status flag operations
   - **Will expose the critical `setFlag()` bug** (uses `!mask` instead of `~mask`)
   - Tests all flag set/clear operations
   - Tests flag-related opcodes (CLRS, SETC, CLC, CLV)

2. **test/test_embedded/test_opcodes.cpp** - Tests basic opcodes
   - **Will expose the DECA bug** (uses `++` instead of `--`)
   - Tests load/store operations (LDAi, LDBi, STAa, etc.)
   - Tests increment/decrement operations
   - Tests logical operations (AND, OR, XOR)

3. **test/test_embedded/test_branches.cpp** - Tests branch operations
   - Tests unconditional branches (BRA)
   - Tests conditional branches (BZS, BZC, BCS, BCC)
   - **Tests 2's complement offsets** (positive and negative)
   - Tests edge cases (max positive/negative offsets)
   - Includes integration test with a simple loop

## Expected Test Results

### First Run (Before Fixes)
You should see **FAILURES** in:

1. **test_flags.cpp**:
   - `test_clear_zero_flag` - FAIL (setFlag bug)
   - `test_clear_carry_flag` - FAIL (setFlag bug)

2. **test_opcodes.cpp**:
   - `test_deca_decrements` - FAIL (DECA uses ++ instead of --)

All other tests should PASS (if the hardware is working).

### Fixing the Bugs

#### Bug 1: Fix setFlag() in core.cpp:367
```cpp
// BEFORE (WRONG):
status = status & !mask;

// AFTER (CORRECT):
status = status & ~mask;
```

#### Bug 2: Fix DECA in core.cpp:602
```cpp
// BEFORE (WRONG):
case DECA:
    if (acc[0] == 0xFF) setFlag(OVERFLOW, true);
    acc[0]++;  // WRONG!
    doFlags(acc[0]);
    return;

// AFTER (CORRECT):
case DECA:
    if (acc[0] == 0x00) setFlag(OVERFLOW, true);
    acc[0]--;  // CORRECT
    doFlags(acc[0]);
    return;
```

#### Bug 3: Fix runMode assignment in core.cpp:226
```cpp
// BEFORE (WRONG):
runMode == RUN; // compiler says it has no effect

// AFTER (CORRECT):
runMode = RUN;
```

## TDD Workflow

1. **Run tests** - See which ones fail
2. **Fix ONE bug** in src/core.cpp
3. **Run tests again** - Verify the fix worked
4. **Commit** the fix with the test
5. **Repeat** for next bug

## Interpreting Test Output

### Success:
```
test/test_embedded/test_flags.cpp:23:test_set_zero_flag [PASSED]
test/test_embedded/test_flags.cpp:28:test_set_carry_flag [PASSED]
```

### Failure:
```
test/test_embedded/test_flags.cpp:45:test_clear_zero_flag [FAILED]
Expected TRUE was FALSE
ZERO should be clear
```

### Summary:
```
-----------------------
11 Tests 2 Failures 0 Ignored
FAILED
```

## Adding New Tests

When you find a new bug or add a feature:

1. **Create a new test file** in `test/test_embedded/`
2. **Include headers**:
   ```cpp
   #include <unity.h>
   #include <Arduino.h>
   #include "../test_helpers.h"
   ```
3. **Write test functions** starting with `test_`
4. **Add setUp/tearDown** for test isolation
5. **Register tests in setup()**:
   ```cpp
   void setup() {
       delay(2000);
       UNITY_BEGIN();
       RUN_TEST(test_my_new_feature);
       UNITY_END();
   }
   ```

## Serial Monitor During Tests

If tests hang or you want to see debug output:

```bash
# In one terminal, upload the test
~/.platformio/penv/bin/pio test -e nanoatmega328 --without-uploading

# In another terminal, monitor serial
~/.platformio/penv/bin/pio device monitor
```

## Next Steps

1. **Run the tests** to see current failures
2. **Fix the setFlag() bug** first (most critical)
3. **Fix the DECA bug** second
4. **Run tests again** to verify
5. **Add more tests** for remaining known issues:
   - Flow control timing
   - Long program loading
   - Serial communication

See TEST_STRATEGY.md for comprehensive testing documentation.
