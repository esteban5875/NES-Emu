#include "../include/CPU.h"
#include "../include/ERROR.h"

#include <stdbool.h>

/* ============================================================
 *  SHARED HELPERS
 * ============================================================ */

static inline void branch_if(CPU_STATUS *status, const AddressingModes mode, bool condition) {
    int8_t offset = (int8_t)read_mem(get_operand_address(mode, status), status);
    if (condition) {
        status->program_counter = (uint16_t)(status->program_counter + offset);
    }
}

static inline void set_reg(CPU_STATUS *status, uint8_t *reg, uint8_t val) {
    *reg = val;
    update_zero_and_negative_flags(status, val);
}

static inline void compare(CPU_STATUS *status, uint8_t reg, uint8_t operand) {
    uint16_t diff = (uint16_t)reg - (uint16_t)operand;   // borrow lands in bit 8

    uint8_t flags = (diff < 0x100)               // C: set when reg >= operand  -> bit 0
                  | (((uint8_t)diff == 0) << 1)  // Z: set when equal           -> bit 1
                  | (diff & 0x80);               // N: bit 7 of result          -> bit 7

    status->status = (status->status & ~(FLAG_CARRY | FLAG_ZERO | FLAG_NEGATIVE)) | flags;
}

static inline bool mode_is_valid(const AddressingModes mode, const AddressingModes* valid_modes, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (valid_modes[i] == mode) return true;
    }
    return false;
}

static inline void validate_mode(const char* instr_name, const AddressingModes mode,
                    const AddressingModes* valid_modes, size_t count) {
    if (!mode_is_valid(mode, valid_modes, count)) {
        panic("%s: invalid addressing mode %d", instr_name, mode);
    }
}

// Read the operand for a shift/rotate: accumulator mode or memory.
// Writes the resolved address into *addr_out when mode != accumulator.
static inline uint8_t shift_operand(CPU_STATUS *status, const AddressingModes mode,
                                    uint16_t *addr_out) {
    if (mode == NoneAddressing) return status->register_a;
    *addr_out = OP_ADDRESS;                  // captured once — RMW reuses it
    return read_mem(*addr_out, status);
}

// Write back the result of a shift/rotate to A or memory.
static inline void shift_result(CPU_STATUS *status, const AddressingModes mode,
                                uint16_t addr, uint8_t result) {
    if (mode == NoneAddressing) status->register_a = result;
    else                        write_mem(addr, result, status);
    update_zero_and_negative_flags(status, result);
}

// BRK / IRQ / NMI share this. set_b distinguishes software BRK from hardware IRQ.
static void interrupt(CPU_STATUS *status, uint16_t vector, bool set_b) {
    push_u16(status, status->program_counter);
    push(status, set_b ? (status->status |  FLAG_BREAK | 0x20)
                       : ((status->status & ~FLAG_BREAK) | 0x20));
    status->status |= FLAG_INTERRUPT;
    status->program_counter = read_mem_u16(vector, status);
}


/* ============================================================
 *  LOAD / STORE
 * ============================================================ */

INSTRUCTION_DECL_WITH_ADDR(LDA){
    VALIDATE_MODE("LDA", mode,
        Immediate, ZeroPage, ZeroPage_X,
        Absolute, Absolute_X, Absolute_Y,
        Indirect_X, Indirect_Y
    );
    UPDATE_REG(status, register_a, OP_VALUE);
}

INSTRUCTION_DECL_WITH_ADDR(LDX){
    VALIDATE_MODE("LDX", mode,
        Immediate, ZeroPage, ZeroPage_Y,
        Absolute, Absolute_Y
    );
    UPDATE_REG(status, register_x, OP_VALUE);
}

INSTRUCTION_DECL_WITH_ADDR(LDY){
    VALIDATE_MODE("LDY", mode,
        Immediate, ZeroPage, ZeroPage_X,
        Absolute, Absolute_X
    );
    UPDATE_REG(status, register_y, OP_VALUE);
}

INSTRUCTION_DECL_WITH_ADDR(STA){
    VALIDATE_MODE("STA", mode,
        ZeroPage, ZeroPage_X, Absolute,
        Absolute_X, Absolute_Y, Indirect_X,
        Indirect_Y
    );
    write_mem(OP_ADDRESS, status->register_a, status);
}

INSTRUCTION_DECL_WITH_ADDR(STX){
    VALIDATE_MODE("STX", mode, ZeroPage, ZeroPage_Y, Absolute);
    write_mem(OP_ADDRESS, status->register_x, status);
}

INSTRUCTION_DECL_WITH_ADDR(STY){
    VALIDATE_MODE("STY", mode, ZeroPage, ZeroPage_X, Absolute);
    write_mem(OP_ADDRESS, status->register_y, status);
}


/* ============================================================
 *  TRANSFERS
 * ============================================================ */

INSTRUCTION_DECL_NO_ADDR(TAX){ UPDATE_REG(status, register_x, status->register_a); }
INSTRUCTION_DECL_NO_ADDR(TAY){ UPDATE_REG(status, register_y, status->register_a); }
INSTRUCTION_DECL_NO_ADDR(TXA){ UPDATE_REG(status, register_a, status->register_x); }
INSTRUCTION_DECL_NO_ADDR(TYA){ UPDATE_REG(status, register_a, status->register_y); }
INSTRUCTION_DECL_NO_ADDR(TSX){ UPDATE_REG(status, register_x, status->stack_pointer); }

INSTRUCTION_DECL_NO_ADDR(TXS){
    status->stack_pointer = status->register_x;   // sets NO flags — the odd one out
}


/* ============================================================
 *  STACK
 * ============================================================ */

INSTRUCTION_DECL_NO_ADDR(PHA){
    push(status, status->register_a);
}

INSTRUCTION_DECL_NO_ADDR(PHP){
    push(status, status->status | FLAG_BREAK | 0x20);   // pushed copy: B=1, bit5=1
}

INSTRUCTION_DECL_NO_ADDR(PLA){
    UPDATE_REG(status, register_a, pull(status));
}

INSTRUCTION_DECL_NO_ADDR(PLP){
    status->status = (pull(status) & ~FLAG_BREAK) | 0x20;  // B/bit5 not settable in P
}


/* ============================================================
 *  LOGIC
 * ============================================================ */

INSTRUCTION_DECL_WITH_ADDR(AND){
    VALIDATE_MODE("AND", mode,
        Immediate, ZeroPage, ZeroPage_X,
        Absolute, Absolute_X, Absolute_Y,
        Indirect_X, Indirect_Y
    );
    UPDATE_REG(status, register_a, status->register_a & OP_VALUE);
}

INSTRUCTION_DECL_WITH_ADDR(EOR){
    VALIDATE_MODE("EOR", mode,
        Immediate, ZeroPage, ZeroPage_X,
        Absolute, Absolute_X, Absolute_Y,
        Indirect_X, Indirect_Y
    );
    UPDATE_REG(status, register_a, status->register_a ^ OP_VALUE);
}

INSTRUCTION_DECL_WITH_ADDR(ORA){
    VALIDATE_MODE("ORA", mode,
        Immediate, ZeroPage, ZeroPage_X,
        Absolute, Absolute_X, Absolute_Y,
        Indirect_X, Indirect_Y
    );
    UPDATE_REG(status, register_a, status->register_a | OP_VALUE);
}

INSTRUCTION_DECL_WITH_ADDR(BIT){
    VALIDATE_MODE("BIT", mode, ZeroPage, Absolute);

    uint8_t operand = OP_VALUE;

    UPDATE_FLAG(status, FLAG_ZERO, (operand & status->register_a) == 0);
    UPDATE_FLAG(status, FLAG_NEGATIVE, operand & FLAG_NEGATIVE);
    UPDATE_FLAG(status, FLAG_OVERFLOW, operand & FLAG_OVERFLOW);
}


/* ============================================================
 *  ARITHMETIC / COMPARE
 * ============================================================ */

INSTRUCTION_DECL_WITH_ADDR(ADC){
    VALIDATE_MODE("ADC", mode,
        Immediate, ZeroPage, ZeroPage_X,
        Absolute, Absolute_X, Absolute_Y,
        Indirect_X, Indirect_Y
    );

    uint8_t carry_bit = (status->status & FLAG_CARRY) ? 1 : 0;
    uint8_t operand = OP_VALUE;
    uint16_t result = (uint16_t)status->register_a + operand + carry_bit;

    status->register_a = (uint8_t)result;
    update_all_flags(status, operand, result);
}

INSTRUCTION_DECL_WITH_ADDR(SBC){
    VALIDATE_MODE("SBC", mode,
        Immediate, ZeroPage, ZeroPage_X,
        Absolute, Absolute_X, Absolute_Y,
        Indirect_X, Indirect_Y
    );

    uint8_t carry_bit = (status->status & FLAG_CARRY) ? 1 : 0;
    uint8_t operand = (uint8_t)(OP_VALUE ^ 0xFF);        // SBC == ADC with inverted operand
    uint16_t result = (uint16_t)status->register_a + operand + carry_bit;

    status->register_a = (uint8_t)result;
    update_all_flags(status, operand, result);           // pass the INVERTED operand
}

INSTRUCTION_DECL_WITH_ADDR(CMP){
    VALIDATE_MODE("CMP", mode,
        Immediate, ZeroPage, ZeroPage_X,
        Absolute, Absolute_X, Absolute_Y,
        Indirect_X, Indirect_Y
    );
    compare(status, status->register_a, OP_VALUE);
}

INSTRUCTION_DECL_WITH_ADDR(CPX){
    VALIDATE_MODE("CPX", mode, Immediate, ZeroPage, Absolute);
    compare(status, status->register_x, OP_VALUE);
}

INSTRUCTION_DECL_WITH_ADDR(CPY){
    VALIDATE_MODE("CPY", mode, Immediate, ZeroPage, Absolute);
    compare(status, status->register_y, OP_VALUE);
}


/* ============================================================
 *  INCREMENT / DECREMENT
 * ============================================================ */

INSTRUCTION_DECL_WITH_ADDR(INC){
    VALIDATE_MODE("INC", mode, ZeroPage, ZeroPage_X, Absolute, Absolute_X);

    uint16_t addr = OP_ADDRESS;                       // capture once
    uint8_t result = (uint8_t)(read_mem(addr, status) + 1);
    write_mem(addr, result, status);
    update_zero_and_negative_flags(status, result);
}

INSTRUCTION_DECL_WITH_ADDR(DEC){
    VALIDATE_MODE("DEC", mode, ZeroPage, ZeroPage_X, Absolute, Absolute_X);

    uint16_t addr = OP_ADDRESS;                       // capture once
    uint8_t result = (uint8_t)(read_mem(addr, status) - 1);
    write_mem(addr, result, status);
    update_zero_and_negative_flags(status, result);
}

INSTRUCTION_DECL_NO_ADDR(INX){ UPDATE_REG(status, register_x, status->register_x + 1); }
INSTRUCTION_DECL_NO_ADDR(INY){ UPDATE_REG(status, register_y, status->register_y + 1); }
INSTRUCTION_DECL_NO_ADDR(DEX){ UPDATE_REG(status, register_x, status->register_x - 1); }
INSTRUCTION_DECL_NO_ADDR(DEY){ UPDATE_REG(status, register_y, status->register_y - 1); }


/* ============================================================
 *  SHIFTS / ROTATES
 * ============================================================ */

INSTRUCTION_DECL_WITH_ADDR(ASL){
    VALIDATE_MODE("ASL", mode,
        NoneAddressing, ZeroPage, ZeroPage_X, Absolute, Absolute_X);

    uint16_t addr = 0;
    uint8_t operand = shift_operand(status, mode, &addr);
    uint8_t result  = (uint8_t)(operand << 1);

    UPDATE_FLAG(status, FLAG_CARRY, operand & 0x80);
    shift_result(status, mode, addr, result);
}

INSTRUCTION_DECL_WITH_ADDR(LSR){
    VALIDATE_MODE("LSR", mode,
        NoneAddressing, ZeroPage, ZeroPage_X, Absolute, Absolute_X);

    uint16_t addr = 0;
    uint8_t operand = shift_operand(status, mode, &addr);
    uint8_t result  = (uint8_t)(operand >> 1);        // bit 7 becomes 0, so N always clears

    UPDATE_FLAG(status, FLAG_CARRY, operand & 0x01);
    shift_result(status, mode, addr, result);
}

INSTRUCTION_DECL_WITH_ADDR(ROL){
    VALIDATE_MODE("ROL", mode,
        NoneAddressing, ZeroPage, ZeroPage_X, Absolute, Absolute_X);

    uint16_t addr = 0;
    uint8_t operand  = shift_operand(status, mode, &addr);
    uint8_t carry_in = (status->status & FLAG_CARRY) ? 1 : 0;
    uint8_t result   = (uint8_t)((operand << 1) | carry_in);   // 9-bit rotate through C

    UPDATE_FLAG(status, FLAG_CARRY, operand & 0x80);
    shift_result(status, mode, addr, result);
}

INSTRUCTION_DECL_WITH_ADDR(ROR){
    VALIDATE_MODE("ROR", mode,
        NoneAddressing, ZeroPage, ZeroPage_X, Absolute, Absolute_X);

    uint16_t addr = 0;
    uint8_t operand  = shift_operand(status, mode, &addr);
    uint8_t carry_in = (status->status & FLAG_CARRY) ? 0x80 : 0x00;
    uint8_t result   = (uint8_t)((operand >> 1) | carry_in);   // 9-bit rotate through C

    UPDATE_FLAG(status, FLAG_CARRY, operand & 0x01);
    shift_result(status, mode, addr, result);
}


/* ============================================================
 *  JUMPS / CALLS
 * ============================================================ */

INSTRUCTION_DECL_WITH_ADDR(JMP){
    VALIDATE_MODE("JMP", mode, Absolute, Indirect);
    status->program_counter = OP_ADDRESS;
}

INSTRUCTION_DECL_WITH_ADDR(JSR){
    VALIDATE_MODE("JSR", mode, Absolute);

    uint16_t target = OP_ADDRESS;
    push_u16(status, status->program_counter - 1);   // addr of JSR's last byte
    status->program_counter = target;
}

INSTRUCTION_DECL_NO_ADDR(RTS){
    status->program_counter = pull_u16(status) + 1;  // +1: JSR pushed addr-of-last-byte
}


/* ============================================================
 *  BRANCHES  (all Relative)
 * ============================================================ */

INSTRUCTION_DECL_WITH_ADDR(BCC){
    VALIDATE_MODE("BCC", mode, Relative);
    branch_if(status, mode, (status->status & FLAG_CARRY) == 0);
}

INSTRUCTION_DECL_WITH_ADDR(BCS){
    VALIDATE_MODE("BCS", mode, Relative);
    branch_if(status, mode, (status->status & FLAG_CARRY) != 0);
}

INSTRUCTION_DECL_WITH_ADDR(BEQ){
    VALIDATE_MODE("BEQ", mode, Relative);
    branch_if(status, mode, (status->status & FLAG_ZERO) != 0);
}

INSTRUCTION_DECL_WITH_ADDR(BNE){
    VALIDATE_MODE("BNE", mode, Relative);
    branch_if(status, mode, (status->status & FLAG_ZERO) == 0);
}

INSTRUCTION_DECL_WITH_ADDR(BMI){
    VALIDATE_MODE("BMI", mode, Relative);
    branch_if(status, mode, (status->status & FLAG_NEGATIVE) != 0);
}

INSTRUCTION_DECL_WITH_ADDR(BPL){
    VALIDATE_MODE("BPL", mode, Relative);
    branch_if(status, mode, (status->status & FLAG_NEGATIVE) == 0);
}

INSTRUCTION_DECL_WITH_ADDR(BVS){
    VALIDATE_MODE("BVS", mode, Relative);
    branch_if(status, mode, (status->status & FLAG_OVERFLOW) != 0);
}

INSTRUCTION_DECL_WITH_ADDR(BVC){
    VALIDATE_MODE("BVC", mode, Relative);
    branch_if(status, mode, (status->status & FLAG_OVERFLOW) == 0);
}


/* ============================================================
 *  FLAG CONTROL
 * ============================================================ */

INSTRUCTION_DECL_NO_ADDR(CLC){ CLEAR_FLAG(status, FLAG_CARRY);     }
INSTRUCTION_DECL_NO_ADDR(CLD){ CLEAR_FLAG(status, FLAG_DECIMAL);   }
INSTRUCTION_DECL_NO_ADDR(CLI){ CLEAR_FLAG(status, FLAG_INTERRUPT); }
INSTRUCTION_DECL_NO_ADDR(CLV){ CLEAR_FLAG(status, FLAG_OVERFLOW);  }
INSTRUCTION_DECL_NO_ADDR(SEC){ SET_FLAG(status, FLAG_CARRY);       }
INSTRUCTION_DECL_NO_ADDR(SED){ SET_FLAG(status, FLAG_DECIMAL);     }
INSTRUCTION_DECL_NO_ADDR(SEI){ SET_FLAG(status, FLAG_INTERRUPT);   }


/* ============================================================
 *  SYSTEM
 * ============================================================ */

INSTRUCTION_DECL_NO_ADDR(NOP){
    // 2 cycles, touches nothing. Empty body is correct.
}

INSTRUCTION_DECL_NO_ADDR(BRK){
    status->program_counter++;              // skip the padding byte after opcode 0x00
    interrupt(status, 0xFFFE, true);
}

INSTRUCTION_DECL_NO_ADDR(RTI){
    status->status = (pull(status) & ~FLAG_BREAK) | 0x20;  // P first
    status->program_counter = pull_u16(status);            // then PC — NO +1
}