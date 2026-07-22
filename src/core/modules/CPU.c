#include "../include/ERROR.h"
#include "../include/CPU.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void reset_pc(CPU_STATUS* status){
    status->program_counter = ROM_START;
}

void update_all_flags(CPU_STATUS* cpu, uint8_t operand, uint16_t result) {
    uint8_t result8 = (uint8_t) result;
    uint8_t old_a    = cpu->register_a; // must be captured before register_a is overwritten

    UPDATE_FLAG(cpu, FLAG_CARRY, result > 0xFF);

    UPDATE_FLAG(cpu, FLAG_OVERFLOW,
        (~(old_a ^ operand) & (old_a ^ result8) & 0x80));

    UPDATE_FLAG(cpu, FLAG_ZERO, result8 == 0);
    UPDATE_FLAG(cpu, FLAG_NEGATIVE, result8 & 0x80);
}


void update_zero_and_negative_flags(CPU_STATUS* cpu, uint8_t result) {
    UPDATE_FLAG(cpu, FLAG_ZERO, result == 0);
    UPDATE_FLAG(cpu, FLAG_NEGATIVE, result & 0x80);
}

void load_program(uint8_t* program, CPU_STATUS* status, size_t size) {
    if (size > ROM_SIZE) {
        size = ROM_SIZE; // avoid overflowing memory[]
    }

    memcpy(&status->memory[ROM_START], program, size);
}

void clear_program(CPU_STATUS* status){
    memset(&status->memory[ROM_START], 0, ROM_SIZE);
}

void clear_ram(CPU_STATUS* status) {
    memset(status->memory, 0, RAM_SIZE);
}

void clear_memory(CPU_STATUS* status){
    memset(status->memory, 0, sizeof(status->memory));
}

// Takes ownership of `program` and frees it internally. Caller must not
// use or free `program` after calling this function
void execute_prog(CPU_STATUS* status, uint8_t* program) {
    reset_pc(status);
    while (status->program_counter <= ROM_END &&
           status->memory[status->program_counter] != HALT) {

        uint8_t opcode = status->memory[status->program_counter];
        status->program_counter++;          // past opcode; operands self-advance now

        switch (opcode) {
        case 0xA9: // LDA Immediate
            LDA(status, Immediate);
            break;

        case 0xAA: // TAX (implied — no operand, no advance beyond opcode)
            status->register_x = status->register_a;
            update_zero_and_negative_flags(status, status->register_x);
            break;

        case 0xE8: // INX (implied)
            status->register_x++;
            update_zero_and_negative_flags(status, status->register_x);
            break;

        default:
            panic("execute_prog: unknown opcode 0x%02X at PC 0x%04X",
                  opcode, status->program_counter - 1);
        }
    }

    free(program);
}

uint8_t* read_prog(CPU_STATUS* status) {
    uint16_t program_counter = ROM_START;
    while (program_counter <= ROM_END && status->memory[program_counter] != HALT) {
        program_counter++;
    }

    size_t len = (size_t)(program_counter - ROM_START);
    
    uint8_t* program = malloc(len + 1);
    
    if (program == NULL) return NULL;

    for (size_t i = 0; i < len; i++) {
        program[i] = status->memory[ROM_START + i];
    }

    program[len] = 0x00;
    return program;
}

void reset_cpu(CPU_STATUS *status) { //Reset the CPU state and program counter
    status->status = 0;
    status->register_a = 0;
    status->register_x = 0;
    status->register_y = 0;
    status->program_counter = ROM_START;
}

uint8_t wrapping_add(uint8_t pos, uint8_t reg) { //Helper function for wrapping add
    return (uint8_t)(pos + reg);
}

uint16_t get_operand_address(const AddressingModes mode, CPU_STATUS* status) {
    switch (mode) {
        case Immediate: {
            uint16_t addr = status->program_counter;
            status->program_counter += 1;
            return addr;
        }
        case ZeroPage: {
            uint16_t addr = (uint16_t) read_mem(status->program_counter, status);
            status->program_counter += 1;
            return addr;
        }
        case Absolute: {
            uint16_t addr = read_mem_u16(status->program_counter, status);
            status->program_counter += 2;
            return addr;
        }
        case ZeroPage_X: {
            uint8_t pos = read_mem(status->program_counter, status);
            status->program_counter += 1;
            return (uint16_t) wrapping_add(pos, status->register_x);
        }
        case ZeroPage_Y: {
            uint8_t pos = read_mem(status->program_counter, status);
            status->program_counter += 1;
            return (uint16_t) wrapping_add(pos, status->register_y);
        }
        case Absolute_X: {
            uint16_t pos = read_mem_u16(status->program_counter, status);
            status->program_counter += 2;
            return pos + status->register_x;
        }
        case Absolute_Y: {
            uint16_t pos = read_mem_u16(status->program_counter, status);
            status->program_counter += 2;
            return pos + status->register_y;
        }
        case Indirect_X: {
            uint8_t base = read_mem(status->program_counter, status);
            status->program_counter += 1;
            uint8_t ptr = wrapping_add(base, status->register_x);
            uint8_t lo = read_mem(ptr, status);
            uint8_t hi = read_mem((uint8_t)(ptr + 1), status);
            return (uint16_t)((hi << 8) | lo);
        }
        case Indirect_Y: {
            uint8_t base = read_mem(status->program_counter, status);
            status->program_counter += 1;
            uint8_t lo = read_mem(base, status);
            uint8_t hi = read_mem((uint8_t)(base + 1), status);
            uint16_t deref_base = (uint16_t)((hi << 8) | lo);
            return deref_base + status->register_y;
        }
        case Indirect: {
            uint16_t ptr = read_mem_u16(status->program_counter, status);
            status->program_counter += 2;
            uint16_t lo = read_mem(ptr, status);
            uint16_t hi;
            if ((ptr & 0x00FF) == 0x00FF) {
                hi = read_mem(ptr & 0xFF00, status);   // page-boundary bug preserved
            } else {
                hi = read_mem(ptr + 1, status);
            }
            return (uint16_t)((hi << 8) | lo);
        }

        case NoneAddressing:{
            panic("get_operand: unimplemented addressing mode %d", mode);
            return 0;
        }

        case Relative: {
            uint16_t addr = status->program_counter;
            status->program_counter += 1;
            return addr;
        }

        default: {
            panic("get_operand: unhandled mode %d", mode);   // silences the non-void warning
            return 0;
        }
    }
}