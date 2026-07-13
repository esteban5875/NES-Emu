#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//Consts

#define MEM_SIZE 0x10000

#define RAM_START 0x0000
#define RAM_END   0x07FF
#define RAM_SIZE (RAM_END + 1)


//CPU Data

typedef enum AddressingModes { //Enum for CPU Addressing modes
    Immediate,
    ZeroPage,
    ZeroPage_X,
    ZeroPage_Y,
    Absolute,
    Absolute_X,
    Absolute_Y,
    Indirect_X,
    Indirect_Y,
    NoneAddressing
} AddressingModes;

typedef struct CPU_STATUS {
    uint8_t register_a;
    uint8_t register_x;
    uint8_t register_y;
    uint8_t status;
    uint16_t program_counter;
    uint8_t memory[MEM_SIZE];
} CPU_STATUS;

//CPU Logic

extern void load_program(const uint8_t* program, CPU_STATUS* status, const size_t size);
extern void execute_prog(uint8_t* program, CPU_STATUS* status);
extern uint16_t get_operand_address(const AddressingModes mode, CPU_STATUS* status);
extern void reset_cpu(CPU_STATUS* status);
extern uint8_t* read_prog(CPU_STATUS* status);

//CPU Mem Logic

extern bool read_mem_accessible(uint16_t addr, CPU_STATUS* status, uint8_t* out_value);
extern void write_mem_accessible(uint16_t addr, uint8_t value, CPU_STATUS* status);
extern uint16_t read_mem_u16(uint16_t addr, CPU_STATUS* status);
extern void write_mem_u16(uint16_t value, uint16_t addr, CPU_STATUS* status);
extern uint8_t read_mem(uint16_t addr, CPU_STATUS* status);
extern void write_mem(uint16_t addr, uint8_t value, CPU_STATUS* status);
extern void clear_memory(CPU_STATUS* status);


