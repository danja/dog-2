/**
 * Test Status Flags Operations
 *
 * This tests the critical setFlag() and getFlag() functions.
 * CRITICAL BUG: setFlag() uses !mask instead of ~mask for bitwise negation
 */

#include <unity.h>
#include <Arduino.h>
#include "test_helpers.h"

void setUp(void) {
    reset_test_state();
}

void tearDown(void) {
    // Clean up after each test
}

// Test setting individual flags
void test_set_zero_flag(void) {
    setFlag(ZERO, true);
    ASSERT_FLAG_SET(ZERO);
}

void test_set_carry_flag(void) {
    setFlag(CARRY, true);
    ASSERT_FLAG_SET(CARRY);
}

void test_set_negative_flag(void) {
    setFlag(NEGATIVE, true);
    ASSERT_FLAG_SET(NEGATIVE);
}

void test_set_overflow_flag(void) {
    setFlag(OVERFLOW, true);
    ASSERT_FLAG_SET(OVERFLOW);
}

// Test clearing flags (THIS WILL FAIL due to !mask bug)
void test_clear_zero_flag(void) {
    setFlag(ZERO, true);   // Set it first
    setFlag(ZERO, false);  // Then clear it
    ASSERT_FLAG_CLEAR(ZERO);
}

void test_clear_carry_flag(void) {
    setFlag(CARRY, true);
    setFlag(CARRY, false);
    ASSERT_FLAG_CLEAR(CARRY);
}

// Test that setting one flag doesn't affect others
void test_flags_independent(void) {
    setFlag(ZERO, true);
    setFlag(CARRY, true);

    ASSERT_FLAG_SET(ZERO);
    ASSERT_FLAG_SET(CARRY);
    ASSERT_FLAG_CLEAR(NEGATIVE);
    ASSERT_FLAG_CLEAR(OVERFLOW);
}

// Test flag operations with CLRS opcode
void test_clrs_opcode(void) {
    // Set all flags
    setFlag(ZERO, true);
    setFlag(CARRY, true);
    setFlag(NEGATIVE, true);
    setFlag(OVERFLOW, true);

    // Run CLRS
    program[0] = CLRS;
    pc = 0;
    doOperation();

    // All flags should be clear
    TEST_ASSERT_EQUAL_HEX8(0, status);
}

// Test SETC and CLC opcodes
void test_setc_opcode(void) {
    status = 0;
    program[0] = SETC;
    pc = 0;
    doOperation();
    ASSERT_FLAG_SET(CARRY);
}

void test_clc_opcode(void) {
    setFlag(CARRY, true);
    program[0] = CLC;
    pc = 0;
    doOperation();
    ASSERT_FLAG_CLEAR(CARRY);
}

// Test CLV opcode
void test_clv_opcode(void) {
    setFlag(OVERFLOW, true);
    program[0] = CLV;
    pc = 0;
    doOperation();
    ASSERT_FLAG_CLEAR(OVERFLOW);
}

void setup() {
    delay(2000); // Wait for serial connection
    UNITY_BEGIN();

    RUN_TEST(test_set_zero_flag);
    RUN_TEST(test_set_carry_flag);
    RUN_TEST(test_set_negative_flag);
    RUN_TEST(test_set_overflow_flag);
    RUN_TEST(test_clear_zero_flag);
    RUN_TEST(test_clear_carry_flag);
    RUN_TEST(test_flags_independent);
    RUN_TEST(test_clrs_opcode);
    RUN_TEST(test_setc_opcode);
    RUN_TEST(test_clc_opcode);
    RUN_TEST(test_clv_opcode);

    UNITY_END();
}

void loop() {
    // Nothing to do here
}
