#include "../include/CPU.h"
#include <stdint.h>

void LDA(CPU_STATUS *status, const AddressingModes mode){
    uint16_t address = get_operand_address(mode, status);
    uint8_t value = read_mem(address, status);

    status->register_a = value;
    update_zero_and_negative_flags(status, value);
} 

void LDX(CPU_STATUS *status, const AddressingModes mode){
    uint16_t address = get_operand_address(mode, status);
    uint8_t value = read_mem(address, status);

    status->register_x = value;
    update_zero_and_negative_flags(status, value);
}

void LDY(CPU_STATUS *status, const AddressingModes mode){
    uint16_t address = get_operand_address(mode, status);
    uint8_t value = read_mem(address, status);

    status->register_y = value;
    update_zero_and_negative_flags(status, value);
}

void ADC(CPU_STATUS *status, const AddressingModes mode){
    uint8_t carry_bit = (status->status & CARRY_FLAG) ? 1 : 0;
    uint8_t operand = read_mem(get_operand_address(mode, status), status);
    uint16_t result = (uint16_t)status->register_a + operand + carry_bit;
    uint8_t result8 = (uint8_t)result;

     // carry (bit 0)
    if (result > 0xFF) status->status |= 0x01;
    else status->status &= ~0x01;

    // overflow (bit 6)
    if (~(status->register_a ^ operand) & (status->register_a ^ result8) & 0x80) {
        status->status |= 0x40;
    }
    
    else {  status->status &= ~0x40;    }
    
    status->register_a = result8;
    
    update_zero_and_negative_flags(status, result8);
}

