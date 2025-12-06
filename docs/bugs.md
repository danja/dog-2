# DOG-2 Bug Tracker

This document tracks bugs found through testing and their fixes.

## Status Legend
- [ ] Not fixed
- [x] Fixed
- [~] Partially fixed / needs verification

## Test Results After Fixes

**Date**: 2025-12-06 (After fixes)
**Total Tests**: 57
**Passed**: 56 (98.2%)
**Failed**: 1 (1.8%)

### By Test Suite:
- ✅ **test_flags**: 11/11 (100%)
- ✅ **test_opcodes**: 16/16 (100%)
- ✅ **test_stack**: 15/15 (100%)
- ✅ **test_branches**: 14/15 (93%) - 1 integration test issue

## Critical Bugs (Must Fix)

### [x] Bug #1: DECA uses increment instead of decrement
**Severity**: Critical
**Location**: `src/core.cpp:602`
**Test**: `test_opcodes.cpp::test_deca_decrements`
**Symptom**: DECA increments accumulator instead of decrementing
**Expected**: `DECA` with acc[0]=0x10 should result in 0x0F
**Actual**: Results in 0x11

**Fix Required**:
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

### [x] Bug #2: setFlag() using logical NOT instead of bitwise NOT
**Severity**: Critical
**Location**: `src/core.cpp:1151-1154`
**Tests**:
- `test_opcodes.cpp::test_ldai_sets_zero_flag`
- `test_opcodes.cpp::test_inca_sets_zero_flag`
- `test_stack.cpp::test_inxs_sets_zero_flag`

**Symptom**: Loading or incrementing to zero doesn't set ZERO flag
**Expected**: Operations resulting in 0 should set ZERO flag
**Actual**: ZERO flag not set

**Status**: ✅ FIXED - This was the root cause of flags not being set correctly

**Fix Applied**: Changed line 367 from `status = status & !mask` to `status = status & ~mask`

### [x] Bug #3: doFlags() not setting OVERFLOW flag
**Severity**: Critical
**Location**: `src/core.cpp:1151-1154`
**Test**: `test_opcodes.cpp::test_inca_sets_overflow_on_rollover`
**Symptom**: Incrementing 0xFF to 0x00 doesn't set OVERFLOW
**Expected**: OVERFLOW flag should be set on rollover
**Actual**: OVERFLOW flag not set

**Status**: ✅ FIXED - Fixed by Bug #2 (setFlag fix)

### [x] Bug #4: All branch operations off by 1
**Severity**: Critical
**Location**: `src/core.cpp:440-472` (BRA, BZS, BZC, BCS, BCC)
**Tests**: All 15 tests in `test_branches.cpp` fail
**Symptom**: PC after branch is always 1 less than expected
**Expected**: `BRA +5` from PC=0 should land at PC=7
**Actual**: Lands at PC=6

**Root Cause**: Branch operations increment PC twice (once to read offset, once more when they shouldn't)

**Analysis**:
```cpp
case BRA:
    pc++;           // Move to offset byte (pc=1)
    pc += (int8_t)program[pc];  // Add offset (pc=1+5=6)
    return;         // Should be at 7, but we're at 6
```

**Status**: ✅ FIXED

**Fix Applied**: Updated all branch operations (BRA, BZS, BZC, BCS, BCC) to:
1. Increment PC to offset byte
2. Increment PC to next instruction position
3. Add signed offset from previous position
4. For conditional branches, also increment PC if not branching

### [x] Bug #5: CLRA opcode not implemented
**Severity**: High
**Location**: `src/core.cpp` (missing from switch statement)
**Test**: `test_opcodes.cpp::test_clra_clears_accumulator`
**Symptom**: CLRA opcode (0xC1) has no implementation
**Expected**: CLRA should set acc[0] to 0
**Actual**: Accumulator unchanged (0xFF remains 0xFF)

**Fix Required**: Add case to doOperation() switch statement

### [ ] Bug #6: CLRB opcode not implemented
**Severity**: High
**Location**: `src/core.cpp` (missing from switch statement)
**Test**: `test_opcodes.cpp::test_clrb_clears_accumulator`
**Symptom**: CLRB opcode (0xC2) has no implementation
**Expected**: CLRB should set acc[1] to 0
**Actual**: Accumulator unchanged (0xFF remains 0xFF)

**Fix Required**: Add case to doOperation() switch statement

### [ ] Bug #7: DEXS opcode not implemented
**Severity**: Medium
**Location**: `src/core.cpp` (missing from switch statement)
**Test**: `test_stack.cpp::test_dexs_decrements_stack_pointer`
**Symptom**: DEXS opcode (0xDC) has no implementation
**Expected**: DEXS should decrement xStackP
**Actual**: Stack pointer unchanged

**Fix Required**: Add case to doOperation() switch statement (mirrors INXS)

### [ ] Bug #8: ROT stack operation incorrect
**Severity**: Medium
**Location**: `src/core.cpp:711-717`
**Test**: `test_stack.cpp::test_rot_rotates_top_three`
**Symptom**: ROT doesn't rotate stack elements correctly
**Expected**: Stack `a b c` should become `c a b` (third to top)
**Actual**: Stack becomes `a c b` or similar

**Investigation**: Current implementation at line 711-717 needs review

### [ ] Bug #9: ABA opcode not working correctly
**Severity**: Medium
**Location**: `src/core.cpp` (likely not implemented or buggy)
**Test**: `test_stack.cpp::test_rpn_calculation`
**Symptom**: RPN calculation test fails - ABA doesn't add accumulators
**Expected**: With A=3, B=5, ABA should result in A=8
**Actual**: Results in A=3

**Investigation**: Check if ABA opcode is implemented

## Previously Known Bugs (from README.md)

### [ ] Bug #10: runMode assignment uses == instead of =
**Severity**: Low
**Location**: `src/core.cpp:226`
**Symptom**: Compiler warning: "statement has no effect"
**Fix Required**:
```cpp
// Line 226 - BEFORE:
runMode == RUN; // compiler says it has no effect TODO

// AFTER:
runMode = RUN;
```

### [ ] Bug #11: Flow control timing issues
**Severity**: Medium
**Source**: README.md, docs/todo.md
**Symptom**: May require button fumbling to operate
**Status**: Needs investigation and testing

### [ ] Bug #12: Loading long programs fails
**Severity**: High
**Source**: README.md, docs/todo.md
**Symptom**: Programs longer than 32 instructions fail to upload via serial
**Status**: Needs investigation - may be serial buffer issue

### [ ] Bug #13: Branch opcodes 2's complement issues
**Severity**: Critical
**Source**: README.md
**Symptom**: Branch operations with negative offsets don't work correctly
**Status**: **CONFIRMED by tests** - all branch tests fail
**Related**: Bug #4

## Test Results Summary

**Date**: 2025-12-06
**Total Tests**: 57
**Passed**: 32 (56%)
**Failed**: 25 (44%)

### By Test Suite:
- ✅ **test_flags**: 11/11 (100%) - All flag operations working!
- ⚠️ **test_opcodes**: 10/16 (62.5%) - 6 bugs found
- ❌ **test_branches**: 0/15 (0%) - All branch operations broken
- ⚠️ **test_stack**: 11/15 (73%) - 4 bugs found

## Fix Priority

### High Priority (Blocking core functionality)
1. Bug #4 - Branch operations (all broken)
2. Bug #2, #3 - Flag setting (breaks many operations)
3. Bug #1 - DECA opcode
4. Bug #5, #6 - CLRA/CLRB opcodes

### Medium Priority (Missing features)
5. Bug #7 - DEXS opcode
6. Bug #8 - ROT stack operation
7. Bug #9 - ABA opcode

### Low Priority (Cleanup)
8. Bug #10 - runMode assignment

### Needs Investigation
9. Bug #11 - Flow control timing
10. Bug #12 - Long program loading

## Notes

- Flag operations (setFlag/getFlag) appear to be working correctly based on test_flags results
- The original suspected bug in setFlag() (using !mask instead of ~mask) may have already been fixed or wasn't the issue
- Branch operations are systematically off by 1, suggesting a fundamental issue with PC handling
- Several opcodes defined in core.h are not implemented in the switch statement
