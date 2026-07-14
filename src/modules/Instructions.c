#include "../include/CPU.h"
#include "../include/ERROR.h"

static bool mode_is_valid(const AddressingModes mode, const AddressingModes* valid_modes, size_t count) {
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

void LDA(CPU_STATUS *status, const AddressingModes mode){
    VALIDATE_MODE("LDA", mode, 
        Immediate, ZeroPage, ZeroPage_X,
        Absolute, Absolute_X, Absolute_Y,
        Indirect_X, Indirect_Y
    );

    uint16_t address = get_operand_address(mode, status);
    uint8_t value = read_mem(address, status);

    status->register_a = value;
    update_zero_and_negative_flags(status, value);
} 

void LDX(CPU_STATUS *status, const AddressingModes mode){
    VALIDATE_MODE("LDX", mode, 
        Immediate, ZeroPage, ZeroPage_Y,
        Absolute, Absolute_Y
    );

    uint16_t address = get_operand_address(mode, status);
    uint8_t value = read_mem(address, status);

    status->register_x = value;
    update_zero_and_negative_flags(status, value);
}

void LDY(CPU_STATUS *status, const AddressingModes mode){
    VALIDATE_MODE("LDY", mode, 
        Immediate, ZeroPage, ZeroPage_X,
        Absolute, Absolute_X
    );

    uint16_t address = get_operand_address(mode, status);
    uint8_t value = read_mem(address, status);

    status->register_y = value;
    update_zero_and_negative_flags(status, value);
}

void ADC(CPU_STATUS *status, const AddressingModes mode){
    VALIDATE_MODE("ADC", mode, 
        Immediate, ZeroPage, ZeroPage_X,
        Absolute, Absolute_X, Absolute_Y,
        Indirect_X, Indirect_Y
    );

    uint8_t carry_bit = (status->status & FLAG_CARRY) ? 1 : 0;
    uint8_t operand = read_mem(get_operand_address(mode, status), status);
    uint16_t result = (uint16_t)status->register_a + operand + carry_bit;
    uint8_t result8 = (uint8_t)result;
    
    status->register_a = result8;
    
    update_all_flags(status, operand, result);
}

void AND(CPU_STATUS *status, const AddressingModes mode){
    VALIDATE_MODE("AND", mode, 
        Immediate, ZeroPage, ZeroPage_X, 
        Absolute, Absolute_X, Absolute_Y, 
        Indirect_X, Indirect_Y
    );

    uint8_t operand = read_mem(get_operand_address(mode, status), status);
    status->register_a &= operand;
    update_zero_and_negative_flags(status, status->register_a);
}

void ASL(CPU_STATUS *status, const AddressingModes mode) {
    VALIDATE_MODE("ASL", mode,
        NoneAddressing, ZeroPage, 
        ZeroPage_X, Absolute, 
        Absolute_X
    );

    uint8_t operand;
    uint16_t addr; // only meaningful if mode != Accumulator

    if (mode == NoneAddressing) {
        operand = status->register_a;
    } else {
        addr = get_operand_address(mode, status);
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

void JMP(CPU_STATUS *status, const AddressingModes mode) {
    VALIDATE_MODE("JMP", mode, Absolute, Indirect);

    uint16_t address = get_operand_address(mode, status);
    status->program_counter = address;
}