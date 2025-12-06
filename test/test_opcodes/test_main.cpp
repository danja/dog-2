/**
 * Test Basic Opcode Operations
 *
 * Tests fundamental opcodes like load, store, arithmetic
 * Includes test for DECA bug (uses ++ instead of --)
 */

#include <unity.h>
#include <Arduino.h>
#include "test_helpers.h"

void setUp(void) {
    reset_test_state();
}

void tearDown(void) {
}

// ========== Load Immediate Tests ==========

void test_ldai_loads_value(void) {
    SET_PROGRAM(LDAi, 0x42);
    pc = 0;
    doOperation(); // LDAi
    ASSERT_ACC_A(0x42);
    ASSERT_PC(1); // Should advance to next instruction
}

void test_ldbi_loads_value(void) {
    SET_PROGRAM(LDBi, 0x99);
    pc = 0;
    doOperation(); // LDBi
    ASSERT_ACC_B(0x99);
}

void test_ldai_sets_zero_flag(void) {
    SET_PROGRAM(LDAi, 0x00);
    pc = 0;
    doOperation();
    ASSERT_ACC_A(0x00);
    ASSERT_FLAG_SET(ZERO);
}

void test_ldai_sets_negative_flag(void) {
    SET_PROGRAM(LDAi, 0x80); // MSB set = negative
    pc = 0;
    doOperation();
    ASSERT_FLAG_SET(NEGATIVE);
}

// ========== Store/Load Absolute Tests ==========

void test_staa_ldaa_roundtrip(void) {
    acc[0] = 0xF6;
    SET_PROGRAM(STAa, 0x10, 0x00); // Store A at address 0x0010
    pc = 0;
    doOperation();

    // Verify it was stored
    TEST_ASSERT_EQUAL_HEX8(0xF6, program[0x10]);

    // Load it back into B
    acc[0] = 0;
    SET_PROGRAM(LDAa, 0x10, 0x00);
    pc = 0;
    doOperation();
    ASSERT_ACC_A(0xF6);
}

// ========== Increment/Decrement Tests ==========

void test_inca_increments(void) {
    acc[0] = 0x10;
    program[0] = INCA;
    pc = 0;
    doOperation();
    ASSERT_ACC_A(0x11);
}

void test_inca_sets_zero_flag(void) {
    acc[0] = 0xFF;
    program[0] = INCA;
    pc = 0;
    doOperation();
    ASSERT_ACC_A(0x00);
    ASSERT_FLAG_SET(ZERO);
}

void test_inca_sets_overflow_on_rollover(void) {
    acc[0] = 0xFF;
    program[0] = INCA;
    pc = 0;
    doOperation();
    ASSERT_FLAG_SET(OVERFLOW);
}

// CRITICAL BUG TEST: DECA uses ++ instead of --
void test_deca_decrements(void) {
    acc[0] = 0x10;
    program[0] = DECA;
    pc = 0;
    doOperation();
    ASSERT_ACC_A(0x0F); // This will FAIL - currently does ++
}

void test_incb_increments(void) {
    acc[1] = 0x20;
    program[0] = INCB;
    pc = 0;
    doOperation();
    ASSERT_ACC_B(0x21);
}

// ========== Swap Test ==========

void test_swap_exchanges_accumulators(void) {
    acc[0] = 0xAA;
    acc[1] = 0xBB;
    program[0] = SWAP;
    pc = 0;
    doOperation();
    ASSERT_ACC_A(0xBB);
    ASSERT_ACC_B(0xAA);
}

// ========== Clear Tests ==========

void test_clra_clears_accumulator(void) {
    acc[0] = 0xFF;
    program[0] = CLRA;
    pc = 0;
    doOperation();
    ASSERT_ACC_A(0x00);
}

void test_clrb_clears_accumulator(void) {
    acc[1] = 0xFF;
    program[0] = CLRB;
    pc = 0;
    doOperation();
    ASSERT_ACC_B(0x00);
}

// ========== Logical Operations ==========

void test_and_operation(void) {
    acc[0] = 0b11110000;
    acc[1] = 0b10101010;
    program[0] = AND;
    pc = 0;
    doOperation();
    ASSERT_ACC_A(0b10100000);
}

void test_or_operation(void) {
    acc[0] = 0b11110000;
    acc[1] = 0b10101010;
    program[0] = OR;
    pc = 0;
    doOperation();
    ASSERT_ACC_A(0b11111010);
}

void test_xor_operation(void) {
    acc[0] = 0b11110000;
    acc[1] = 0b10101010;
    program[0] = XOR;
    pc = 0;
    doOperation();
    ASSERT_ACC_A(0b01011010);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();

    RUN_TEST(test_ldai_loads_value);
    RUN_TEST(test_ldbi_loads_value);
    RUN_TEST(test_ldai_sets_zero_flag);
    RUN_TEST(test_ldai_sets_negative_flag);
    RUN_TEST(test_staa_ldaa_roundtrip);
    RUN_TEST(test_inca_increments);
    RUN_TEST(test_inca_sets_zero_flag);
    RUN_TEST(test_inca_sets_overflow_on_rollover);
    RUN_TEST(test_deca_decrements);
    RUN_TEST(test_incb_increments);
    RUN_TEST(test_swap_exchanges_accumulators);
    RUN_TEST(test_clra_clears_accumulator);
    RUN_TEST(test_clrb_clears_accumulator);
    RUN_TEST(test_and_operation);
    RUN_TEST(test_or_operation);
    RUN_TEST(test_xor_operation);

    UNITY_END();
}

void loop() {
}
