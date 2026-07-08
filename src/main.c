#include "./include/CPU.h"
//#include "./include/ByteToBin.h" <-- Uncomment to debug

#include <assert.h>
#include <stdio.h>

static CPU_STATUS status = {0};

void test_read_prog() {
    uint8_t test_prog[] = {0xA9, 0xC0, 0xAA, 0xE8, 0x00}; //Set A to 0xC0 and break

    execute_prog(test_prog, &status);

    //Print statements for debugging
    
    //printf("Register A: 0x%02X (expected 0xC0)\n", status.register_a);
    //printf("Status flags: 0b" BYTE_TO_BINARY_PATTERN " (expected zero flag=0, negative flag=1)\n",
           //BYTE_TO_BINARY(status.status));
    //printf("Program counter: %u (expected 2)\n", status.program_counter);

    assert(status.register_a == 0xC0);
    assert(status.register_x == 0xC1);
    assert((status.status & 0b00000010) == 0);      // zero flag should be clear
    assert((status.status & 0b10000000) == 0b10000000); // negative flag should be set
    assert(status.program_counter == 4);
    
    printf("test_read_prog passed!\n");
}

int main() {
    test_read_prog();
    printf("All tests passed!\n");
    return 0;
}
