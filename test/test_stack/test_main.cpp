/**
 * Test Stack Operations
 *
 * Tests the auxiliary stack operations (Forth-style stack manipulation)
 * and PC stack operations for subroutines
 */

#include <unity.h>
#include <Arduino.h>
#include "test_helpers.h"

void setUp(void) {
    reset_test_state();
}

void tearDown(void) {
}

// ========== Basic Stack Operations ==========

void test_pushxa_adds_to_stack(void) {
    acc[0] = 0x42;
    program[0] = PUSHXA;
    pc = 0;
    doOperation();

    TEST_ASSERT_EQUAL_HEX8(0x42, xStack[0]);
    TEST_ASSERT_EQUAL_UINT8(1, xStackP);
}

void test_popxa_removes_from_stack(void) {
    // Setup: push a value first
    xStack[0] = 0x99;
    xStackP = 1;
    acc[0] = 0x00;

    // Pop it
    program[0] = POPXA;
    pc = 0;
    doOperation();

    ASSERT_ACC_A(0x99);
    TEST_ASSERT_EQUAL_UINT8(0, xStackP);
}

void test_pushxb_and_popxb(void) {
    acc[1] = 0xAB;
    program[0] = PUSHXB;
    pc = 0;
    doOperation();

    TEST_ASSERT_EQUAL_HEX8(0xAB, xStack[0]);

    acc[1] = 0x00;
    program[1] = POPXB;
    pc = 1;
    doOperation();

    ASSERT_ACC_B(0xAB);
}

// ========== Stack Manipulation (Forth-style) ==========

void test_dup_duplicates_top(void) {
    // Setup: a b c on stack (xStackP = 2, top at index 2)
    xStack[0] = 0xAA;  // a (bottom)
    xStack[1] = 0xBB;  // b
    xStack[2] = 0xCC;  // c (top)
    xStackP = 2;

    program[0] = DUP;
    pc = 0;
    doOperation();

    // Should now be: a b c c
    TEST_ASSERT_EQUAL_HEX8(0xAA, xStack[0]);
    TEST_ASSERT_EQUAL_HEX8(0xBB, xStack[1]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, xStack[2]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, xStack[3]);  // Duplicated
    TEST_ASSERT_EQUAL_UINT8(3, xStackP);
}

void test_swaps_exchanges_top_two(void) {
    // Setup: a b c on stack
    xStack[0] = 0xAA;
    xStack[1] = 0xBB;
    xStack[2] = 0xCC;
    xStackP = 2;

    program[0] = SWAPS;
    pc = 0;
    doOperation();

    // Should now be: a c b
    TEST_ASSERT_EQUAL_HEX8(0xAA, xStack[0]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, xStack[1]);  // Swapped
    TEST_ASSERT_EQUAL_HEX8(0xBB, xStack[2]);  // Swapped
}

void test_over_copies_second_to_top(void) {
    // Setup: a b c on stack
    xStack[0] = 0xAA;
    xStack[1] = 0xBB;
    xStack[2] = 0xCC;
    xStackP = 2;

    program[0] = OVER;
    pc = 0;
    doOperation();

    // Should be: a b c b
    TEST_ASSERT_EQUAL_HEX8(0xAA, xStack[0]);
    TEST_ASSERT_EQUAL_HEX8(0xBB, xStack[1]);
    TEST_ASSERT_EQUAL_HEX8(0xCC, xStack[2]);
    TEST_ASSERT_EQUAL_HEX8(0xBB, xStack[3]);  // Copy of second
    TEST_ASSERT_EQUAL_UINT8(3, xStackP);
}

void test_rot_rotates_top_three(void) {
    // Setup: a b c on stack
    xStack[0] = 0xAA;
    xStack[1] = 0xBB;
    xStack[2] = 0xCC;
    xStackP = 2;

    program[0] = ROT;
    pc = 0;
    doOperation();

    // Should be: c a b (rotate third to top)
    TEST_ASSERT_EQUAL_HEX8(0xCC, xStack[0]);
    TEST_ASSERT_EQUAL_HEX8(0xAA, xStack[1]);
    TEST_ASSERT_EQUAL_HEX8(0xBB, xStack[2]);
}

void test_drop_removes_top(void) {
    // Setup: a b c on stack
    xStack[0] = 0xAA;
    xStack[1] = 0xBB;
    xStack[2] = 0xCC;
    xStackP = 2;

    program[0] = DROP;
    pc = 0;
    doOperation();

    // Should be: a b
    TEST_ASSERT_EQUAL_HEX8(0xAA, xStack[0]);
    TEST_ASSERT_EQUAL_HEX8(0xBB, xStack[1]);
    TEST_ASSERT_EQUAL_UINT8(1, xStackP);
}

void test_tuck_inserts_copy_below_second(void) {
    // Setup: a b c on stack
    xStack[0] = 0xAA;
    xStack[1] = 0xBB;
    xStack[2] = 0xCC;
    xStackP = 2;

    program[0] = TUCK;
    pc = 0;
    doOperation();

    // TUCK is SWAP then DUP, so: a b c -> a c b -> a c b b
    TEST_ASSERT_EQUAL_UINT8(3, xStackP);
}

// ========== Stack Overflow/Underflow Tests ==========

void test_pushxa_near_overflow(void) {
    // Fill stack to near capacity
    xStackP = X_STACK_SIZE - 2;
    acc[0] = 0x99;

    program[0] = PUSHXA;
    pc = 0;
    doOperation();

    TEST_ASSERT_EQUAL_UINT8(X_STACK_SIZE - 1, xStackP);
}

void test_popxa_underflow_detection(void) {
    // Attempting to pop from empty stack should show error
    xStackP = 0;

    program[0] = POPXA;
    pc = 0;
    doOperation();

    // Should set mode to PROG_MODE (error handling in popX)
    TEST_ASSERT_EQUAL(PROG_MODE, mode);
}

// ========== Stack Pointer Operations ==========

void test_inxs_increments_stack_pointer(void) {
    xStackP = 5;
    program[0] = INXS;
    pc = 0;
    doOperation();

    TEST_ASSERT_EQUAL_UINT8(6, xStackP);
    ASSERT_FLAG_CLEAR(ZERO);
}

void test_inxs_sets_zero_flag(void) {
    xStackP = 0xFF;  // Will overflow to 0
    program[0] = INXS;
    pc = 0;
    doOperation();

    TEST_ASSERT_EQUAL_UINT8(0, xStackP);
    ASSERT_FLAG_SET(ZERO);
}

void test_dexs_decrements_stack_pointer(void) {
    xStackP = 5;
    program[0] = DEXS;
    pc = 0;
    doOperation();

    TEST_ASSERT_EQUAL_UINT8(4, xStackP);
}

// ========== Integration Test: RPN Calculator ==========

void test_rpn_calculation(void) {
    // Test: 5 3 + (should give 8)
    // Push 5 to stack
    acc[0] = 5;
    program[0] = PUSHXA;
    pc = 0;
    doOperation();

    // Push 3 to stack
    acc[0] = 3;
    program[1] = PUSHXA;
    pc = 1;
    doOperation();

    // Pop to A and B, add them
    program[2] = POPXA;
    pc = 2;
    doOperation();  // A = 3

    program[3] = POPXB;
    pc = 3;
    doOperation();  // B = 5

    program[4] = ABA;
    pc = 4;
    doOperation();  // A = A + B = 8

    ASSERT_ACC_A(8);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();

    RUN_TEST(test_pushxa_adds_to_stack);
    RUN_TEST(test_popxa_removes_from_stack);
    RUN_TEST(test_pushxb_and_popxb);
    RUN_TEST(test_dup_duplicates_top);
    RUN_TEST(test_swaps_exchanges_top_two);
    RUN_TEST(test_over_copies_second_to_top);
    RUN_TEST(test_rot_rotates_top_three);
    RUN_TEST(test_drop_removes_top);
    RUN_TEST(test_tuck_inserts_copy_below_second);
    RUN_TEST(test_pushxa_near_overflow);
    RUN_TEST(test_popxa_underflow_detection);
    RUN_TEST(test_inxs_increments_stack_pointer);
    RUN_TEST(test_inxs_sets_zero_flag);
    RUN_TEST(test_dexs_decrements_stack_pointer);
    RUN_TEST(test_rpn_calculation);

    UNITY_END();
}

void loop() {
}
