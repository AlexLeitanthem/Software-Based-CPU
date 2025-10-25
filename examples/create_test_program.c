#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main() {
    // Create a simple test program
    uint8_t program[] = {
        0x00, 0x48,  // LDI #'H' (0x48)
        0x02, 0x00, 0x80,  // STA [0x8000] (UART TX)
        0x00, 0x65,  // LDI #'e' (0x65)
        0x02, 0x00, 0x80,  // STA [0x8000] (UART TX)
        0x00, 0x6C,  // LDI #'l' (0x6C)
        0x02, 0x00, 0x80,  // STA [0x8000] (UART TX)
        0x00, 0x6C,  // LDI #'l' (0x6C)
        0x02, 0x00, 0x80,  // STA [0x8000] (UART TX)
        0x00, 0x6F,  // LDI #'o' (0x6F)
        0x02, 0x00, 0x80,  // STA [0x8000] (UART TX)
        0x00, 0x0A,  // LDI #'\n' (0x0A)
        0x02, 0x00, 0x80,  // STA [0x8000] (UART TX)
        0x73        // HLT
    };
    
    FILE* file = fopen("examples/simple_hello.bin", "wb");
    if (file) {
        fwrite(program, 1, sizeof(program), file);
        fclose(file);
        printf("Created simple_hello.bin with %zu bytes\n", sizeof(program));
    } else {
        printf("Failed to create file\n");
        return 1;
    }
    
    return 0;
}
