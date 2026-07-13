#include "../include/CPU.h"

void LDA(CPU_STATUS *status, const AddressingModes mode){
    uint16_t address = get_operand_address(mode, status);
    uint8_t value = read_mem(address, status);

    status->register_a = value;
    update_zero_and_negative_flags(status, value);
} 
