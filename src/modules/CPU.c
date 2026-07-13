#include "../include/Error.h"
#include "../include/CPU.h"

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

void load_program(const uint8_t* program, CPU_STATUS* status, size_t size) {
    if (size > RAM_SIZE) {
        size = RAM_SIZE; // avoid overflowing memory[]
    }
    
    memcpy(status->memory, program, size);
}

void clear_memory(CPU_STATUS* status) {
    memset(status->memory, 0, RAM_SIZE);
}

// Takes ownership of `program` and frees it internally. Caller must not
// use or free `program` after calling this function
void execute_prog(uint8_t* program, CPU_STATUS* status) {
    status->program_counter = 0;
    while (status->program_counter < RAM_SIZE && program[status->program_counter] != HALT) {
        uint8_t opcode = program[(size_t) status->program_counter];
        status->program_counter++;
        switch (opcode) {
            case 0xA9: // Load immediate value into A
                LDA(status, Immediate);
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

    free(program);
}


uint8_t* read_prog(CPU_STATUS* status) { //Function to retrieve a program from memory
    uint16_t program_counter = 0; //Set local program counter to avoid collisions with CPU counter
    while (program_counter < 0x7FF && status->memory[program_counter] != HALT) { //Continue until memory limit or BRK
        program_counter++; //Increase program counter
    }

    uint8_t* program = malloc((size_t) program_counter + 1); //Allocate an array of program counter size
    if (program == NULL) return NULL;

    for (size_t i = 0; i < program_counter; i++) {
        program[i] = status->memory[i]; //Store program in alloc
    }

    program[program_counter] = 0x00; // explicit terminator
    
    return program;
}

void reset_cpu(CPU_STATUS *status) { //Reset the CPU state and program counter
    status->status = 0;
    status->register_a = 0;
    status->register_x = 0;
    status->register_y = 0;
    status->program_counter = read_mem_u16(0xFFFC, status);
}

uint8_t wrapping_add(uint8_t pos, uint8_t reg){ //Helper function for wrapping add
    return (uint8_t) pos + reg;
}

uint16_t get_operand_address(const AddressingModes mode, CPU_STATUS* status) {
    switch (mode) {
        case Immediate: return status->program_counter; //Immediate addressing mode, the operand is directly next to the operator 

        case ZeroPage: return (uint16_t) read_mem(status->program_counter, status); //Zero page addressing for operand (0-255)

        case Absolute: return read_mem_u16(status->program_counter, status); //Absolute addressing for operand (0-65535)

        case ZeroPage_X: { //Adds register X to zero page instruction address
            uint8_t pos = read_mem(status->program_counter, status);
            uint16_t addr = (uint16_t) wrapping_add(pos, status->register_x);
            return addr;
        }

        case ZeroPage_Y: { //Adds register Y to zero page instruction address
            uint8_t pos = read_mem(status->program_counter, status);
            uint16_t addr = (uint16_t) wrapping_add(pos, status->register_y);
            return addr;
        }
        
        case Absolute_X: { //Take 16 bit instruction address and add X
            uint16_t pos = read_mem_u16(status->program_counter, status);
            uint16_t addr = pos + status->register_x;
            return addr;
        }

        case Absolute_Y: { //Take 16 bit instruction address and add Y
            uint16_t pos = read_mem_u16(status->program_counter, status);
            uint16_t addr = pos +  status->register_y;
            return addr;
        }

        case Indirect_X: { //Get a ptr by adding X content to zero page instruction address and get the 16 bit address with it
            uint8_t base = read_mem(status->program_counter, status);
            uint8_t ptr = wrapping_add(base, status->register_x);
            uint8_t lo = read_mem(ptr, status);
            uint8_t hi = read_mem((uint8_t)ptr + 1, status);
            uint16_t addr = (hi << 8) | lo;
            return addr;
        }

        case Indirect_Y: { //Get significant two bytes from PC and add Y to get the address
            uint8_t base = read_mem(status->program_counter, status);
            uint8_t lo = read_mem(base, status);
            uint8_t hi = read_mem((uint8_t) (base + 1), status);
            uint16_t deref_base = (hi << 8) | lo;
            uint16_t addr = deref_base + status->register_y;
            return addr;
        }

        case NoneAddressing: {
            panic("get_operand: unimplemented addressing mode %d", mode);
            return 0;
        }
    }
}




