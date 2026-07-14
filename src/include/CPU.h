#pragma once

#include "CPU_DATA.h"

//HALT Macro

#define HALT 0x00

//Macros to avoid boilerplate when defining instructions

#define INSTRUCTION_DECL_WITH_ADDR(name) void name(CPU_STATUS *status, const AddressingModes mode)
#define INSTRUCTION_DECL_NO_ADDR(name) void name(CPU_STATUS *status)

//Helper Macros

#define VALIDATE_MODE(instr_name, mode, ...) \
    do { \
        AddressingModes _valid[] = { __VA_ARGS__ }; \
        validate_mode(instr_name, mode, _valid, sizeof(_valid) / sizeof(_valid[0])); \
    } while (0)

//Instructions

INSTRUCTION_DECL_WITH_ADDR(LDA); //Load to accumulator
INSTRUCTION_DECL_WITH_ADDR(ADC); //ADD with Carry
INSTRUCTION_DECL_WITH_ADDR(AND); //AND Operation
INSTRUCTION_DECL_WITH_ADDR(ASL); //Arithmetic Shift Left
INSTRUCTION_DECL_WITH_ADDR(BIT); //Bit Test
INSTRUCTION_DECL_WITH_ADDR(CMP); //Compare acc with value (A-M) and set flags as appropiate
INSTRUCTION_DECL_WITH_ADDR(CPX); //Compare X register with value (X-M) and set flags as appropiate
INSTRUCTION_DECL_WITH_ADDR(CPY); //Compare Y register with value (Y-M) and set flags as appropiate
INSTRUCTION_DECL_WITH_ADDR(DEC); //Decrement value by 1
INSTRUCTION_DECL_WITH_ADDR(EOR); //Exclusive OR between value and accumulator (A^M)
INSTRUCTION_DECL_WITH_ADDR(INC); //Increment value by 1
INSTRUCTION_DECL_WITH_ADDR(JMP); //Set the program counter to address
INSTRUCTION_DECL_WITH_ADDR(JSR); //Pushes (Address - 1) of return point to the stack and sets program counter to target


INSTRUCTION_DECL_NO_ADDR(BCC); //Branch if carry clear
INSTRUCTION_DECL_NO_ADDR(BCS); //Branch if carry set
INSTRUCTION_DECL_NO_ADDR(BEQ); //Branch if zero flag set
INSTRUCTION_DECL_NO_ADDR(BMI); //Branch if negative flag set
INSTRUCTION_DECL_NO_ADDR(BNE); //Branch if zero flag clear
INSTRUCTION_DECL_NO_ADDR(BPL); //Branch if negative flag clear
INSTRUCTION_DECL_NO_ADDR(BVC); //Branch if overflow flag clear
INSTRUCTION_DECL_NO_ADDR(BVS); //Branch if overflow flag set
INSTRUCTION_DECL_NO_ADDR(CLC); //Clear carry Flag
INSTRUCTION_DECL_NO_ADDR(CLD); //Clear decimal mode flag
INSTRUCTION_DECL_NO_ADDR(CLI); //Clear interrupt disable flag
INSTRUCTION_DECL_NO_ADDR(CLV); //Clear overflow flag
INSTRUCTION_DECL_NO_ADDR(DEX); //Decrement X register by 1
INSTRUCTION_DECL_NO_ADDR(DEY); //Decrement Y register by 1
INSTRUCTION_DECL_NO_ADDR(INX); //Increment X register by 1
INSTRUCTION_DECL_NO_ADDR(INY); //Increment Y register by 1

//Helpers

extern void update_zero_and_negative_flags(CPU_STATUS* cpu, uint8_t result);
extern void update_all_flags(CPU_STATUS* cpu, uint8_t operand, uint16_t result);