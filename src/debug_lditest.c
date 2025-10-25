#include "cpu.h"
#include "isa.h"
#include <stdio.h>

int main(void) {
    cpu_state_t* cpu = cpu_create();
    if (!cpu) {
        printf("cpu_create failed\n");
        return 1;
    }
    uint8_t program[] = {0x00, 0x42}; // LDI #0x42
    if (!cpu_load_program(cpu, program, sizeof(program), 0x0200)) {
        printf("cpu_load_program failed\n");
        return 1;
    }
    cpu_reset_to_address(cpu, 0x0200);
    printf("[DEBUG] PC=0x%04X byte0=0x%02X byte1=0x%02X\n", isa_get_register16(cpu, REG_PC), cpu->memory[0x0200], cpu->memory[0x0201]);
    bool stepped = cpu_step(cpu);
    printf("[DEBUG] cpu_step returned: %s\n", stepped ? "true" : "false");
    printf("[DEBUG] A after step: 0x%02X\n", isa_get_register(cpu, REG_A));
    return 0;
}
