#include "./include/CPU_DATA.h"
#include "./include/BYTETOBIN.h" 

#include <inttypes.h>
#include <assert.h>
#include <stdio.h>

static CPU_STATUS status = {0};

uint8_t test_prog[] = {
    0xA9, 
    0xC0, 
    0xAA, 
    0xE8, 
    0x00
}; //Set A to 0xC0 and break

void log_sys(){
    printf("Register A: 0x%02X\n", status.register_a);
    printf("Status flags: 0b" BYTE_TO_BINARY_PATTERN "\n",
           BYTE_TO_BINARY(status.status));
    printf("Program counter: %u\n", status.program_counter);

    uint8_t opcode;
    int current_inst = 0;

    do {
        opcode = status.memory[current_inst];
        printf("Instruction %d : %02X\n", ((int) current_inst + 1), opcode);
        current_inst++;
    } while (opcode != 0x00);
}

void test_read_prog(uint8_t test_prog[], size_t size) {

    load_program(test_prog, &status, size);
    
    uint8_t* test_prog_from_mem = read_prog(&status);
    
    execute_prog(test_prog_from_mem, &status);

    //Print statements for debugging

    assert(status.program_counter == size - 1);
    assert(status.register_a == 0xC0);
    assert(status.register_x == 0xC1);
    assert((status.status & 0b00000010) == 0);      // zero flag should be clear
    assert((status.status & 0b10000000) == 0b10000000); // negative flag should be set

    log_sys();

    reset_cpu(&status);
}

int main() {
    test_read_prog(test_prog, (sizeof(test_prog)/sizeof(uint8_t)));
    printf("\nAll tests passed!\n");
    return 0;
}
