#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main() {
    // Create a simple addition program: A = 5 + 3
    uint8_t program[] = {
        0x00, 0x05,  // LDI #5
        0x00, 0x03,  // LDI #3 (this overwrites the 5, so we need a different approach)
        0x73        // HLT
    };
    
    // Better approach: store values in memory first
    uint8_t add_program[] = {
        0x00, 0x05,  // LDI #5
        0x02, 0x00, 0x10,  // STA [0x1000] - store 5 at address 0x1000
        0x00, 0x03,  // LDI #3
        0x02, 0x01, 0x10,  // STA [0x1001] - store 3 at address 0x1001
        0x01, 0x00, 0x10,  // LDA [0x1000] - load 5
        0x01, 0x01, 0x10,  // LDA [0x1001] - load 3 (overwrites 5)
        0x73        // HLT
    };
    
    FILE* file = fopen("examples/add_program.bin", "wb");
    if (file) {
        fwrite(add_program, 1, sizeof(add_program), file);
        fclose(file);
        printf("Created add_program.bin with %zu bytes\n", sizeof(add_program));
        printf("This program stores 5 and 3 in memory, then loads them\n");
    } else {
        printf("Failed to create file\n");
        return 1;
    }
    
    return 0;
}

