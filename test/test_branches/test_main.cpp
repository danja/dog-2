/**
 * Test Branch Operations
 *
 * Tests branch opcodes with 2's complement offsets
 * This addresses the known issue with branch operations
 */

#include <unity.h>
#include <Arduino.h>
#include "test_helpers.h"

void setUp(void) {
    reset_test_state();
}

void tearDown(void) {
}

// ========== Unconditional Branch Tests ==========

void test_bra_forward_jump(void) {
    // BRA +5 should jump forward 5 bytes from next instruction
    SET_PROGRAM(BRA, 0x05, NOP, NOP, NOP, NOP, NOP, HALT);
    pc = 0;
    doOperation(); // Execute BRA
    // PC should be at: 0 (BRA) -> 1 (offset) -> 2 (after offset) -> 2+5 = 7
    ASSERT_PC(7);
}

void test_bra_backward_jump(void) {
    // BRA -3 (0xFD in 2's complement) should jump back 3 bytes
    SET_PROGRAM(NOP, NOP, NOP, BRA, 0xFD);
    pc = 3; // Start at BRA
    doOperation(); // Execute BRA
    // PC should be at: 3 (BRA) -> 4 (offset) -> 5 (after offset) -> 5-3 = 2
    ASSERT_PC(2);
}

void test_bra_no_jump(void) {
    // BRA 0 should stay at next instruction
    SET_PROGRAM(BRA, 0x00, HALT);
    pc = 0;
    doOperation();
    ASSERT_PC(2);
}

// ========== Conditional Branch Tests - Zero Flag ==========

void test_bzs_branches_when_zero_set(void) {
    setFlag(ZERO, true);
    SET_PROGRAM(BZS, 0x03, NOP, NOP, NOP, HALT);
    pc = 0;
    doOperation();
    ASSERT_PC(5); // Should branch: 0->1->2->2+3=5
}

void test_bzs_no_branch_when_zero_clear(void) {
    setFlag(ZERO, false);
    SET_PROGRAM(BZS, 0x03, HALT);
    pc = 0;
    doOperation();
    ASSERT_PC(2); // Should not branch, just advance past offset
}

void test_bzc_branches_when_zero_clear(void) {
    setFlag(ZERO, false);
    SET_PROGRAM(BZC, 0x03, NOP, NOP, NOP, HALT);
    pc = 0;
    doOperation();
    ASSERT_PC(5);
}

void test_bzc_no_branch_when_zero_set(void) {
    setFlag(ZERO, true);
    SET_PROGRAM(BZC, 0x03, HALT);
    pc = 0;
    doOperation();
    ASSERT_PC(2);
}

// ========== Conditional Branch Tests - Carry Flag ==========

void test_bcs_branches_when_carry_set(void) {
    setFlag(CARRY, true);
    SET_PROGRAM(BCS, 0x02, NOP, NOP, HALT);
    pc = 0;
    doOperation();
    ASSERT_PC(4);
}

void test_bcs_no_branch_when_carry_clear(void) {
    setFlag(CARRY, false);
    SET_PROGRAM(BCS, 0x02, HALT);
    pc = 0;
    doOperation();
    ASSERT_PC(2);
}

void test_bcc_branches_when_carry_clear(void) {
    setFlag(CARRY, false);
    SET_PROGRAM(BCC, 0x02, NOP, NOP, HALT);
    pc = 0;
    doOperation();
    ASSERT_PC(4);
}

// ========== 2's Complement Offset Tests ==========

void test_branch_with_negative_offset_FE(void) {
    // 0xFE = -2 in 2's complement
    SET_PROGRAM(NOP, NOP, NOP, BZS, 0xFE);
    setFlag(ZERO, true);
    pc = 3; // At BZS
    doOperation();
    // PC: 3->4->5->5-2=3
    ASSERT_PC(3);
}

void test_branch_with_negative_offset_FB(void) {
    // 0xFB = -5 in 2's complement
    SET_PROGRAM(NOP, NOP, NOP, NOP, NOP, BZS, 0xFB);
    setFlag(ZERO, true);
    pc = 5; // At BZS
    doOperation();
    // PC: 5->6->7->7-5=2
    ASSERT_PC(2);
}

void test_branch_with_max_positive_offset(void) {
    // 0x7F = +127 (max positive for signed byte)
    program[0] = BRA;
    program[1] = 0x7F;
    pc = 0;
    doOperation();
    ASSERT_PC(129); // 0->1->2->2+127=129
}

void test_branch_with_max_negative_offset(void) {
    // 0x80 = -128 (max negative for signed byte)
    pc = 128; // Start far enough ahead
    program[128] = BRA;
    program[129] = 0x80;
    doOperation();
    ASSERT_PC(2); // 128->129->130->130-128=2
}

// ========== Integration Test: Loop ==========

void test_simple_loop_with_backward_branch(void) {
    // Simple program: increment A 3 times
    // 0: LDAi 00
    // 2: INCA
    // 3: CMPAi 03
    // 5: BZC FD (-3, back to INCA if not zero)
    SET_PROGRAM(LDAi, 0x00, INCA, CMPAi, 0x03, BZC, 0xFD);

    pc = 0;
    // Run until we hit the branch that doesn't take
    for(int i = 0; i < 20; i++) {
        if (program[pc] == 0) break; // Stop at NOP
        doOperation();
        pc++;
    }

    // After 3 iterations, A should be 3
    ASSERT_ACC_A(0x03);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();

    RUN_TEST(test_bra_forward_jump);
    RUN_TEST(test_bra_backward_jump);
    RUN_TEST(test_bra_no_jump);
    RUN_TEST(test_bzs_branches_when_zero_set);
    RUN_TEST(test_bzs_no_branch_when_zero_clear);
    RUN_TEST(test_bzc_branches_when_zero_clear);
    RUN_TEST(test_bzc_no_branch_when_zero_set);
    RUN_TEST(test_bcs_branches_when_carry_set);
    RUN_TEST(test_bcs_no_branch_when_carry_clear);
    RUN_TEST(test_bcc_branches_when_carry_clear);
    RUN_TEST(test_branch_with_negative_offset_FE);
    RUN_TEST(test_branch_with_negative_offset_FB);
    RUN_TEST(test_branch_with_max_positive_offset);
    RUN_TEST(test_branch_with_max_negative_offset);
    RUN_TEST(test_simple_loop_with_backward_branch);

    UNITY_END();
}

void loop() {
}
