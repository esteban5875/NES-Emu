#include "../include/CPU_DATA.h"
#include "../include/ERROR.h"

bool read_mem_accessible(uint16_t addr, CPU_STATUS* status, uint8_t* out_value) {
      if (addr >= RAM_SIZE) {
          panic("Read_mem_accessible: address 0x%04X out of bounds\n", addr);
      }
      *out_value = status->memory[addr];
      return true;
}

void write_mem_accessible(uint16_t addr, uint8_t value, CPU_STATUS* status){ //Write into one memory slot
    if (addr >= RAM_SIZE) { //Check if address is in bounds
        panic("Write_mem: address 0x%04X out of bounds\n", addr);
    }
    
    status->memory[addr] = value;
}

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

bool read_mem_u16_accessible(uint16_t addr, CPU_STATUS* status, uint16_t* out_value){ //Read one 16 bit word from memory, bounds-checked
    //NES is little-endian meaning least significant come before most significant
    uint8_t out_low;
    uint8_t out_high;

    if (!read_mem_accessible(addr, status, &out_low)) {
        panic("Read_mem_u16_accessible: address 0x%04X out of bounds\n", addr);
    }
    if (!read_mem_accessible(addr + 1, status, &out_high)) {
        panic("Read_mem_u16_accessible: address 0x%04X out of bounds\n", addr + 1);
    }

    *out_value = (uint16_t)((out_high << 8) | out_low);
    return true;
}

void write_mem_u16_accessible(uint16_t value, uint16_t addr, CPU_STATUS* status) { //Write one 16 bit word to memory, bounds-checked
    uint8_t low  = (uint8_t)(value & 0x00FF);       //Mask off the low byte
    uint8_t high = (uint8_t)((value >> 8) & 0x00FF); //Shift right 8 bits to get the high byte, mask just in case
    write_mem_accessible(addr, low, status);       //Store low byte at addr
    write_mem_accessible(addr + 1, high, status);  //Store high byte at addr + 1
}