#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "MEM_MAP.h"

//Flags

#define FLAG_CARRY      0x01  // bit 0
#define FLAG_ZERO       0x02  // bit 1
#define FLAG_INTERRUPT  0x04  // bit 2
#define FLAG_DECIMAL    0x08  // bit 3
#define FLAG_BREAK      0x10  // bit 4
#define FLAG_UNUSED     0x20  // bit 5
#define FLAG_OVERFLOW   0x40  // bit 6
#define FLAG_NEGATIVE   0x80  // bit 7

//Generic set/clear/check helpers

#define SET_FLAG(cpu, flag)     ((cpu)->status |= (flag))
#define CLEAR_FLAG(cpu, flag)   ((cpu)->status &= ~(flag))
#define UPDATE_FLAG(cpu, flag, cond) \
    ((cond) ? SET_FLAG(cpu, flag) : CLEAR_FLAG(cpu, flag))
#define CHECK_FLAG(cpu, flag)   (((cpu)->status & (flag)) != 0)

//CPU Data Entities

typedef enum AddressingModes { //Enum for CPU Addressing modes
    Immediate,
    ZeroPage,
    ZeroPage_X,
    ZeroPage_Y,
    Absolute,
    Absolute_X,
    Absolute_Y,
    Indirect,
    Indirect_X,
    Indirect_Y,
    Relative,
    NoneAddressing
} AddressingModes;

typedef struct CPU_STATUS {
    uint8_t register_a;
    uint8_t register_x;
    uint8_t register_y;
    uint8_t status;
    uint16_t program_counter;
    uint8_t stack_pointer;
    uint8_t memory[MEM_SIZE];
} CPU_STATUS;

//CPU Logic

extern void load_program(uint8_t* program, CPU_STATUS* status, const size_t size);
extern void execute_prog(CPU_STATUS* status, uint8_t* program); //Takes program just for memory cleanup
extern uint16_t get_operand_address(const AddressingModes mode, CPU_STATUS* status);
extern void reset_cpu(CPU_STATUS* status);
extern uint8_t* read_prog(CPU_STATUS* status);

//Mem Logic

extern bool read_mem_accessible(uint16_t addr, CPU_STATUS* status, uint8_t* out_value);
extern void write_mem_accessible(uint16_t addr, uint8_t value, CPU_STATUS* status);
extern uint16_t read_mem_u16(uint16_t addr, CPU_STATUS* status);
extern void write_mem_u16(uint16_t value, uint16_t addr, CPU_STATUS* status);
extern uint8_t read_mem(uint16_t addr, CPU_STATUS* status);
extern void write_mem(uint16_t addr, uint8_t value, CPU_STATUS* status);
extern void clear_memory(CPU_STATUS* status);
extern void clear_ram(CPU_STATUS* status);


//Prog logic

extern void clear_program(CPU_STATUS* status);

//Stack logic

extern void push(CPU_STATUS *status, uint8_t val);
extern uint8_t pull(CPU_STATUS *status);

extern void push_u16(CPU_STATUS *status, uint16_t val);
extern uint16_t pull_u16(CPU_STATUS *status);