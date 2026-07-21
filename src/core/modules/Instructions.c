#include "../include/CPU.h"
#include "../include/ERROR.h"

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

void validate_mode(const char* instr_name, const AddressingModes mode,
                    const AddressingModes* valid_modes, size_t count) {
    if (!mode_is_valid(mode, valid_modes, count)) {
        panic("%s: invalid addressing mode %d", instr_name, mode);
    }
}

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

INSTRUCTION_DECL_WITH_ADDR(AND){
    VALIDATE_MODE("AND", mode,
        Immediate, ZeroPage, ZeroPage_X,
        Absolute, Absolute_X, Absolute_Y,
        Indirect_X, Indirect_Y
    );

    UPDATE_REG(status, register_a, status->register_a & OP_VALUE);
}

INSTRUCTION_DECL_WITH_ADDR(ASL){
    VALIDATE_MODE("ASL", mode,
        NoneAddressing, ZeroPage,
        ZeroPage_X, Absolute,
        Absolute_X
    );

    uint8_t operand;
    uint16_t addr;

    if (mode == NoneAddressing) {
        operand = status->register_a;
    } else {
        addr = OP_ADDRESS;                // captured once — RMW reuses it
        operand = read_mem(addr, status);
    }

    uint8_t carry_out = (operand & 0x80) ? 1 : 0;
    uint8_t result = (uint8_t)(operand << 1);

    UPDATE_FLAG(status, FLAG_CARRY, carry_out);
    update_zero_and_negative_flags(status, result);

    if (mode == NoneAddressing) {
        status->register_a = result;
    } else {
        write_mem(addr, result, status);
    }
}

INSTRUCTION_DECL_WITH_ADDR(JMP){
    VALIDATE_MODE("JMP", mode, Absolute, Indirect);

    status->program_counter = OP_ADDRESS;
}

INSTRUCTION_DECL_WITH_ADDR(BIT){
    VALIDATE_MODE("BIT", mode, ZeroPage, Absolute);

    uint8_t operand = OP_VALUE;

    UPDATE_FLAG(status, FLAG_ZERO, (operand & status->register_a) == 0);
    UPDATE_FLAG(status, FLAG_NEGATIVE, operand & FLAG_NEGATIVE);
    UPDATE_FLAG(status, FLAG_OVERFLOW, operand & FLAG_OVERFLOW);
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

INSTRUCTION_DECL_WITH_ADDR(DEC){
    VALIDATE_MODE("DEC", mode, ZeroPage, ZeroPage_X, Absolute, Absolute_X);

    uint16_t addr = OP_ADDRESS;                       // capture once
    uint8_t result = (uint8_t)(read_mem(addr, status) - 1);
    write_mem(addr, result, status);
    update_zero_and_negative_flags(status, result);
}

INSTRUCTION_DECL_WITH_ADDR(EOR){
    VALIDATE_MODE("EOR", mode,
        Immediate, ZeroPage, ZeroPage_X,
        Absolute, Absolute_X, Absolute_Y,
        Indirect_X, Indirect_Y
    );

    UPDATE_REG(status, register_a, status->register_a ^ OP_VALUE);
}

INSTRUCTION_DECL_WITH_ADDR(INC){
    VALIDATE_MODE("INC", mode, ZeroPage, ZeroPage_X, Absolute, Absolute_X);

    uint16_t addr = OP_ADDRESS;                       // capture once
    uint8_t result = (uint8_t)(read_mem(addr, status) + 1);
    write_mem(addr, result, status);
    update_zero_and_negative_flags(status, result);
}

INSTRUCTION_DECL_WITH_ADDR(JSR){
    VALIDATE_MODE("JSR", mode, Absolute);

    uint16_t target = OP_ADDRESS;                    
    push_u16(status, status->program_counter - 1);   
    status->program_counter = target;
}