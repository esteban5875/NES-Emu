#include "../include/CPU.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void update_zero_and_negative_flags(CPU_STATUS* cpu, uint8_t result) {
    if (result == 0) { // Zero flag set if A is 0
        cpu->status = cpu->status | 0b00000010; //Sets the zero flag
    } else {
        cpu->status = cpu->status & 0b11111101; //Clears the zero flag
    }

    if (result & 0b10000000) { // Negative flag set if result is negative (bit 7 is set)
        cpu->status = cpu->status | 0b10000000; //Sets the negative flag
    } else {
        cpu->status = cpu->status & 0b01111111; //Clears the negative flag
    }
}

void load_program(uint8_t* program, CPU_STATUS* status, size_t size) {
    if (size > RAM_SIZE) {
        size = RAM_SIZE; // avoid overflowing memory[]
    }
    
    memcpy(status->memory, program, size);
}

void clear_memory(CPU_STATUS* status) {
    memset(status->memory, 0, RAM_SIZE);
}

void execute_prog(uint8_t* program, CPU_STATUS* status) {
    status->program_counter = 0;
    while (status->program_counter < RAM_SIZE && program[status->program_counter] != 0x00) {
        uint8_t opcode = program[(size_t) status->program_counter];
        status->program_counter++;
        switch (opcode) {
            case 0xA9: // Load immediate value into A
                uint8_t operand = program[(size_t) status->program_counter];
                status->program_counter++; // Go to the operand of A9
                status->register_a = operand;
                update_zero_and_negative_flags(status, status->register_a);
                break;
            case 0xAA: // TAX - Transfer A to X
                status->register_x = status->register_a;
                update_zero_and_negative_flags(status, status->register_x);
                break;
            
            case 0xE8: // INX - Increment X
                status->register_x++;
                update_zero_and_negative_flags(status, status->register_x);
                break;
        }
    }
}


uint8_t* read_prog(CPU_STATUS* status) { //Function to retrieve a program from memory
    uint16_t program_counter = 0; //Set local program counter to avoid collisions with CPU counter
    while (program_counter < 0x7FF && status->memory[program_counter] != 0x00) { //Continue until memory limit or BRK
        program_counter++; //Increase program counter
    }

    uint8_t* program = malloc((size_t) program_counter); //Allocate an array of program counter size
    if (program == NULL) return NULL;

    for (size_t i = 0; i < program_counter; i++) {
        program[i] = status->memory[i]; //Store program in alloc
    }

    return program;
}

uint8_t read_mem(uint16_t addr, CPU_STATUS* status){ //Read one memory slot
    if (addr >= RAM_SIZE) { //Check if address is in bounds
        fprintf(stderr, "read_mem: address 0x%04X out of bounds\n", addr);
        return 0;
    }
    return status->memory[addr]; 
}

void write_mem(uint16_t addr, uint8_t value, CPU_STATUS* status){ //Write into one memory slot
    if (addr >= RAM_SIZE) { //Check if address is in bounds
        fprintf(stderr, "write_mem: address 0x%04X out of bounds\n", addr);
        return;
    }
    
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



