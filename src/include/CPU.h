#pragma once

#include <stdint.h>
#include <stddef.h>

#define RAM_START 0x0000
#define RAM_END   0x07FF
#define RAM_SIZE (RAM_END + 1)

typedef struct CPU_STATUS {
    uint8_t register_a;
    uint16_t register_x;
    uint8_t status;
    uint16_t program_counter;
    uint8_t memory[0x10000];
} CPU_STATUS;

extern void load_program(uint8_t* program, CPU_STATUS* status, size_t size);
extern void execute_prog(uint8_t* program, CPU_STATUS* status);
extern uint8_t read_mem(uint16_t addr, CPU_STATUS* status);
extern void write_mem(uint16_t addr, uint8_t value, CPU_STATUS* status);
extern void clear_memory(CPU_STATUS* status);
extern uint16_t read_mem_u16(uint16_t addr, CPU_STATUS* status);
extern void write_mem_u16(uint16_t value, uint16_t addr, CPU_STATUS* status);