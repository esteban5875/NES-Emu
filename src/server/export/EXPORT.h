#pragma once

#include "../../core/include/CPU_DATA.h"

void nes_init(void);
void nes_load_rom(const uint8_t* data, size_t len);
void nes_step_frame(void);
const uint8_t* nes_get_framebuffer(void); // devuelve puntero a 256*240*3 bytes
void nes_set_input(uint8_t controller_state);
void nes_reset(void);