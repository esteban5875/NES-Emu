#pragma once

#include "CPU.h"

extern bool read_mem_accessible(uint16_t addr, CPU_STATUS* status, uint8_t* out_value);
extern void write_mem_accessible(uint16_t addr, uint8_t value, CPU_STATUS* status);
extern uint16_t read_mem_u16(uint16_t addr, CPU_STATUS* status);
extern void write_mem_u16(uint16_t value, uint16_t addr, CPU_STATUS* status);
extern uint8_t read_mem(uint16_t addr, CPU_STATUS* status);
extern void write_mem(uint16_t addr, uint8_t value, CPU_STATUS* status);
extern void clear_memory(CPU_STATUS* status);