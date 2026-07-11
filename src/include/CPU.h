#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "Consts.h"

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

extern void load_program(const uint8_t* program, CPU_STATUS* status, const size_t size);
extern void execute_prog(uint8_t* program, CPU_STATUS* status);
extern uint16_t get_operand_address(const AddressingModes mode, CPU_STATUS* status);
extern void reset_cpu(CPU_STATUS* status);
extern void lda(CPU_STATUS* status, const AddressingModes mode);
extern uint8_t* read_prog(CPU_STATUS* status);

