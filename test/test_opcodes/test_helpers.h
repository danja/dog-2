#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <unity.h>
#include <Arduino.h>
#include "core.h"

// Test execution macros
#define RUN_OPCODE(opcode) \
    program[pc] = opcode; \
    doOperation(); \
    pc++;

#define SET_PROGRAM(...) do { \
    uint8_t _temp_prog[] = {__VA_ARGS__}; \
    for(size_t i = 0; i < sizeof(_temp_prog); i++) program[i] = _temp_prog[i]; \
} while(0)

// Assertion helpers
#define ASSERT_FLAG_SET(flag) \
    TEST_ASSERT_TRUE_MESSAGE(getFlag(flag), #flag " should be set")

#define ASSERT_FLAG_CLEAR(flag) \
    TEST_ASSERT_FALSE_MESSAGE(getFlag(flag), #flag " should be clear")

#define ASSERT_ACC_A(expected) \
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(expected, acc[0], "Accumulator A")

#define ASSERT_ACC_B(expected) \
    TEST_ASSERT_EQUAL_HEX8_MESSAGE(expected, acc[1], "Accumulator B")

#define ASSERT_PC(expected) \
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(expected, pc, "Program Counter")

#define ASSERT_XREG(expected) \
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(expected, xReg, "Index Register")

// Test isolation - reset state before each test
inline void reset_test_state() {
    resetRegisters();
    for(int i = 0; i < MAX_PROG_SIZE; i++) program[i] = 0;
    pc = 0;
}

#endif
