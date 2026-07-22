#pragma once

#include "CPU_DATA.h"

//HALT Macro

#define HALT 0x00

//Instruction Macros

#define INSTRUCTION_DECL_WITH_ADDR(name) void name(CPU_STATUS *status, const AddressingModes mode)
#define INSTRUCTION_DECL_NO_ADDR(name) void name(CPU_STATUS *status)

//Helper Macros

#define VALIDATE_MODE(instr_name, mode, ...) \
    do { \
        AddressingModes _valid[] = { __VA_ARGS__ }; \
        validate_mode(instr_name, mode, _valid, sizeof(_valid) / sizeof(_valid[0])); \
    } while (0)

#define UPDATE_REG(status, reg, val)      \
    do {                                  \
        (status)->reg = (val);            \
        update_zero_and_negative_flags((status), (status)->reg); \
    } while (0)

//Instructions

// ---- Load / Store (6) ----
INSTRUCTION_DECL_WITH_ADDR(LDA); //Load to accumulator
INSTRUCTION_DECL_WITH_ADDR(LDX); //Load to X register
INSTRUCTION_DECL_WITH_ADDR(LDY); //Load to Y register
INSTRUCTION_DECL_WITH_ADDR(STA); //Store accumulator into memory
INSTRUCTION_DECL_WITH_ADDR(STX); //Store X register into memory
INSTRUCTION_DECL_WITH_ADDR(STY); //Store Y register into memory

// ---- Transfers (6) ----
INSTRUCTION_DECL_NO_ADDR(TAX); //Transfer accumulator to X
INSTRUCTION_DECL_NO_ADDR(TAY); //Transfer accumulator to Y
INSTRUCTION_DECL_NO_ADDR(TXA); //Transfer X to accumulator
INSTRUCTION_DECL_NO_ADDR(TYA); //Transfer Y to accumulator
INSTRUCTION_DECL_NO_ADDR(TSX); //Transfer stack pointer to X, sets N/Z
INSTRUCTION_DECL_NO_ADDR(TXS); //Transfer X to stack pointer, sets NO flags

// ---- Stack (4) ----
INSTRUCTION_DECL_NO_ADDR(PHA); //Push accumulator
INSTRUCTION_DECL_NO_ADDR(PHP); //Push processor status (pushed copy has B=1, bit5=1)
INSTRUCTION_DECL_NO_ADDR(PLA); //Pull accumulator, sets N/Z
INSTRUCTION_DECL_NO_ADDR(PLP); //Pull processor status (B and bit5 unaffected in P)

// ---- Logic (4) ----
INSTRUCTION_DECL_WITH_ADDR(AND); //AND with accumulator (A&M)
INSTRUCTION_DECL_WITH_ADDR(EOR); //Exclusive OR with accumulator (A^M)
INSTRUCTION_DECL_WITH_ADDR(ORA); //Inclusive OR with accumulator (A|M)
INSTRUCTION_DECL_WITH_ADDR(BIT); //Bit test: Z from A&M, N from M.7, V from M.6

// ---- Arithmetic / Compare (5) ----
INSTRUCTION_DECL_WITH_ADDR(ADC); //Add with carry
INSTRUCTION_DECL_WITH_ADDR(SBC); //Subtract with carry (= ADC with operand^0xFF)
INSTRUCTION_DECL_WITH_ADDR(CMP); //Compare accumulator (A-M), C set if A>=M
INSTRUCTION_DECL_WITH_ADDR(CPX); //Compare X register (X-M)
INSTRUCTION_DECL_WITH_ADDR(CPY); //Compare Y register (Y-M)

// ---- Increment / Decrement (6) ----
INSTRUCTION_DECL_WITH_ADDR(INC); //Increment memory by 1
INSTRUCTION_DECL_WITH_ADDR(DEC); //Decrement memory by 1
INSTRUCTION_DECL_NO_ADDR(INX);   //Increment X register by 1
INSTRUCTION_DECL_NO_ADDR(INY);   //Increment Y register by 1
INSTRUCTION_DECL_NO_ADDR(DEX);   //Decrement X register by 1
INSTRUCTION_DECL_NO_ADDR(DEY);   //Decrement Y register by 1

// ---- Shifts / Rotates (4) ---- accumulator mode + memory modes
INSTRUCTION_DECL_WITH_ADDR(ASL); //Arithmetic shift left, C = old bit 7
INSTRUCTION_DECL_WITH_ADDR(LSR); //Logical shift right, C = old bit 0, N always cleared
INSTRUCTION_DECL_WITH_ADDR(ROL); //Rotate left through carry (9-bit)
INSTRUCTION_DECL_WITH_ADDR(ROR); //Rotate right through carry (9-bit)

// ---- Jumps / Calls (3) ----
INSTRUCTION_DECL_WITH_ADDR(JMP); //Set PC to address (Indirect has the page-boundary bug)
INSTRUCTION_DECL_WITH_ADDR(JSR); //Push (PC-1 in our model), then PC = target
INSTRUCTION_DECL_NO_ADDR(RTS);   //Return from subroutine, PC = pull_u16 + 1

// ---- Branches (8) ---- all Relative mode
INSTRUCTION_DECL_WITH_ADDR(BCC); //Branch if carry clear
INSTRUCTION_DECL_WITH_ADDR(BCS); //Branch if carry set
INSTRUCTION_DECL_WITH_ADDR(BEQ); //Branch if zero flag set
INSTRUCTION_DECL_WITH_ADDR(BNE); //Branch if zero flag clear
INSTRUCTION_DECL_WITH_ADDR(BMI); //Branch if negative flag set
INSTRUCTION_DECL_WITH_ADDR(BPL); //Branch if negative flag clear
INSTRUCTION_DECL_WITH_ADDR(BVC); //Branch if overflow flag clear
INSTRUCTION_DECL_WITH_ADDR(BVS); //Branch if overflow flag set

// ---- Status flag control (7) ---- no SEV exists on the 6502
INSTRUCTION_DECL_NO_ADDR(CLC); //Clear carry flag
INSTRUCTION_DECL_NO_ADDR(CLD); //Clear decimal mode flag (2A03 ignores D in ADC/SBC)
INSTRUCTION_DECL_NO_ADDR(CLI); //Clear interrupt disable flag
INSTRUCTION_DECL_NO_ADDR(CLV); //Clear overflow flag
INSTRUCTION_DECL_NO_ADDR(SEC); //Set carry flag
INSTRUCTION_DECL_NO_ADDR(SED); //Set decimal mode flag
INSTRUCTION_DECL_NO_ADDR(SEI); //Set interrupt disable flag

// ---- System (2) ----
INSTRUCTION_DECL_NO_ADDR(BRK); //Force interrupt: push PC+2 and P(B=1), I=1, PC=[FFFE]
INSTRUCTION_DECL_NO_ADDR(RTI); //Return from interrupt: pull P then PC, no +1

//Helpers

extern void update_zero_and_negative_flags(CPU_STATUS* cpu, uint8_t result);
extern void update_all_flags(CPU_STATUS* cpu, uint8_t operand, uint16_t result);

//OP Value Macros

#define OP_ADDRESS       get_operand_address(mode, status)
#define OP_VALUE         read_mem(OP_ADDRESS, status)