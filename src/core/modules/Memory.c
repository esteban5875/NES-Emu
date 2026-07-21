#include "../include/CPU_DATA.h"


uint8_t read_mem(uint16_t addr, CPU_STATUS *status){ //Unrestricted read, no bounds check
    return status->memory[addr];
}

void write_mem(uint16_t addr, uint8_t value, CPU_STATUS *status){ //Unrestricted write, no bounds check
    status->memory[addr] = value;
}

uint16_t read_mem_u16(uint16_t addr, CPU_STATUS* status){ //Read one 16 bit word from memory
    //NES is little-endian meaning least significant come before most significant
    uint16_t low  = read_mem(addr, status);
    uint16_t high = read_mem(addr + 1, status);

    return (uint16_t)((high << 8) | low); //Cast to uint16_t just in case
    //Shift most significant 8 bits left and or with low to add least significant 
}

void write_mem_u16(uint16_t value, uint16_t addr, CPU_STATUS* status) { //Write one 16 bit word to memory
    uint8_t low  = (uint8_t)(value & 0x00FF);       //Mask off the low byte
    uint8_t high = (uint8_t)((value >> 8) & 0x00FF); //Shift right 8 bits to get the high byte, mask just in case

    write_mem(addr, low, status);       //Store low byte at addr
    write_mem(addr + 1, high, status);  //Store high byte at addr + 1
}

void push(CPU_STATUS *status, uint8_t val) {
    write_mem(STACK_BASE | status->stack_pointer, val, status);
    status->stack_pointer--;         
}

uint8_t pull(CPU_STATUS *status) {
    status->stack_pointer++;                
    return read_mem(STACK_BASE | status->stack_pointer, status);
}

void push_u16(CPU_STATUS *status, uint16_t val) {
    push(status, (uint8_t)(val >> 8));   // high byte first
    push(status, (uint8_t)(val & 0xFF)); // then low
}

uint16_t pull_u16(CPU_STATUS *status) {
    uint8_t lo = pull(status);           // low byte comes back first
    uint8_t hi = pull(status);
    return (uint16_t)(hi << 8) | lo;
}