#pragma once

#define MEM_SIZE 0x10000

#define RAM_START        0x0000
#define RAM_END          0x07FF
#define RAM_SIZE         (RAM_END - RAM_START + 1)   // 0x0800

#define RAM_MIRROR_START 0x0800
#define RAM_MIRROR_END   0x1FFF

#define STACK_BASE 0x0100   // page 1; real addr = STACK_BASE | S
#define STACK_RESET 0xFD    // S value after reset (top 0xFF minus 3 dummy pushes)


#define PPU_REG_START        0x2000
#define PPU_REG_END          0x2007
#define PPU_REG_SIZE         (PPU_REG_END - PPU_REG_START + 1) // 0x0008

#define PPU_REG_MIRROR_START 0x2008
#define PPU_REG_MIRROR_END   0x3FFF


#define APU_IO_START 0x4000
#define APU_IO_END   0x4017


#define APU_IO_TEST_START 0x4018
#define APU_IO_TEST_END   0x401F


#define EXPANSION_ROM_START 0x4020
#define EXPANSION_ROM_END   0x5FFF

#define SRAM_START 0x6000
#define SRAM_END   0x7FFF
#define SRAM_SIZE  (SRAM_END - SRAM_START + 1)

#define ROM_START 0x8000
#define ROM_END   0xFFFF
#define ROM_SIZE  (ROM_END - ROM_START + 1)   