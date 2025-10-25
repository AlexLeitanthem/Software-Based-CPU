#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Simple CPU state
typedef struct {
    uint8_t regs[8];      // A, B, C, D, X, Y, SP, PC
    uint8_t flags;        // Status flags
    uint8_t* memory;      // 64KB memory
    bool running;
    uint64_t cycle_count;
} simple_cpu_t;

// Flag definitions
#define FLAG_ZERO     0x01
#define FLAG_NEGATIVE 0x02
#define FLAG_CARRY    0x04
#define FLAG_OVERFLOW 0x08

// Register definitions
#define REG_A  0
#define REG_B  1
#define REG_C  2
#define REG_D  3
#define REG_X  4
#define REG_Y  5
#define REG_SP 6
#define REG_PC 7

// Memory size
#define MEMORY_SIZE 65536

// Simple CPU functions
simple_cpu_t* simple_cpu_create(void) {
    simple_cpu_t* cpu = malloc(sizeof(simple_cpu_t));
    if (!cpu) return NULL;
    
    cpu->memory = malloc(MEMORY_SIZE);
    if (!cpu->memory) {
        free(cpu);
        return NULL;
    }
    
    // Initialize
    memset(cpu->regs, 0, sizeof(cpu->regs));
    memset(cpu->memory, 0, MEMORY_SIZE);
    cpu->flags = 0;
    cpu->running = false;
    cpu->cycle_count = 0;
    
    // Set initial PC and SP
    cpu->regs[REG_PC] = 0xFC;  // Low byte
    cpu->regs[REG_PC + 1] = 0xFF;  // High byte (0xFFFC)
    cpu->regs[REG_SP] = 0xFF;  // Low byte
    cpu->regs[REG_SP + 1] = 0x7F;  // High byte (0x7FFF)
    
    return cpu;
}

void simple_cpu_destroy(simple_cpu_t* cpu) {
    if (cpu) {
        if (cpu->memory) free(cpu->memory);
        free(cpu);
    }
}

void simple_cpu_reset(simple_cpu_t* cpu) {
    memset(cpu->regs, 0, sizeof(cpu->regs));
    cpu->flags = 0;
    cpu->running = false;
    cpu->cycle_count = 0;
    
    // Set initial PC and SP
    cpu->regs[REG_PC] = 0xFC;
    cpu->regs[REG_PC + 1] = 0xFF;
    cpu->regs[REG_SP] = 0xFF;
    cpu->regs[REG_SP + 1] = 0x7F;
}

uint8_t simple_cpu_get_register(simple_cpu_t* cpu, int reg) {
    if (reg < 4) return cpu->regs[reg];
    return 0;
}

uint16_t simple_cpu_get_register16(simple_cpu_t* cpu, int reg) {
    if (reg >= 4 && reg <= 7) {
        int index = (reg - 4) * 2;
        return cpu->regs[index] | (cpu->regs[index + 1] << 8);
    }
    return 0;
}

void simple_cpu_set_register(simple_cpu_t* cpu, int reg, uint8_t value) {
    if (reg < 4) cpu->regs[reg] = value;
}

void simple_cpu_set_register16(simple_cpu_t* cpu, int reg, uint16_t value) {
    if (reg >= 4 && reg <= 7) {
        int index = (reg - 4) * 2;
        cpu->regs[index] = value & 0xFF;
        cpu->regs[index + 1] = (value >> 8) & 0xFF;
    }
}

uint8_t simple_cpu_read_memory(simple_cpu_t* cpu, uint16_t address) {
    if (address < MEMORY_SIZE) {
        return cpu->memory[address];
    }
    return 0;
}

void simple_cpu_write_memory(simple_cpu_t* cpu, uint16_t address, uint8_t value) {
    if (address < MEMORY_SIZE) {
        cpu->memory[address] = value;
    }
}

void simple_cpu_print_status(simple_cpu_t* cpu) {
    printf("CPU Status:\n");
    printf("  A = 0x%02X  B = 0x%02X  C = 0x%02X  D = 0x%02X\n", 
           simple_cpu_get_register(cpu, REG_A),
           simple_cpu_get_register(cpu, REG_B),
           simple_cpu_get_register(cpu, REG_C),
           simple_cpu_get_register(cpu, REG_D));
    printf("  X = 0x%04X  Y = 0x%04X\n",
           simple_cpu_get_register16(cpu, REG_X),
           simple_cpu_get_register16(cpu, REG_Y));
    printf("  SP = 0x%04X  PC = 0x%04X\n",
           simple_cpu_get_register16(cpu, REG_SP),
           simple_cpu_get_register16(cpu, REG_PC));
    printf("  Flags = 0x%02X\n", cpu->flags);
    printf("  Cycles = %llu\n", (unsigned long long)cpu->cycle_count);
}

// Simple instruction execution
bool simple_cpu_step(simple_cpu_t* cpu) {
    if (!cpu->running) return false;
    
    uint16_t pc = simple_cpu_get_register16(cpu, REG_PC);
    uint8_t opcode = simple_cpu_read_memory(cpu, pc);
    
    printf("Executing opcode 0x%02X at PC=0x%04X\n", opcode, pc);
    
    // Simple instruction set
    switch (opcode) {
        case 0x00: // LDI (Load Immediate)
            {
                uint8_t value = simple_cpu_read_memory(cpu, pc + 1);
                simple_cpu_set_register(cpu, REG_A, value);
                simple_cpu_set_register16(cpu, REG_PC, pc + 2);
                printf("  LDI #0x%02X\n", value);
            }
            break;
            
        case 0x01: // LDA (Load from memory)
            {
                uint8_t addr_low = simple_cpu_read_memory(cpu, pc + 1);
                uint8_t addr_high = simple_cpu_read_memory(cpu, pc + 2);
                uint16_t addr = addr_low | (addr_high << 8);
                uint8_t value = simple_cpu_read_memory(cpu, addr);
                simple_cpu_set_register(cpu, REG_A, value);
                simple_cpu_set_register16(cpu, REG_PC, pc + 3);
                printf("  LDA [0x%04X] = 0x%02X\n", addr, value);
            }
            break;
            
        case 0x02: // STA (Store to memory)
            {
                uint8_t addr_low = simple_cpu_read_memory(cpu, pc + 1);
                uint8_t addr_high = simple_cpu_read_memory(cpu, pc + 2);
                uint16_t addr = addr_low | (addr_high << 8);
                uint8_t value = simple_cpu_get_register(cpu, REG_A);
                simple_cpu_write_memory(cpu, addr, value);
                simple_cpu_set_register16(cpu, REG_PC, pc + 3);
                printf("  STA [0x%04X] = 0x%02X\n", addr, value);
            }
            break;
            
        case 0x73: // HLT (Halt)
            cpu->running = false;
            simple_cpu_set_register16(cpu, REG_PC, pc + 1);
            printf("  HLT\n");
            break;
            
        default:
            printf("  Unknown opcode: 0x%02X\n", opcode);
            cpu->running = false;
            return false;
    }
    
    cpu->cycle_count++;
    return true;
}

void simple_cpu_run(simple_cpu_t* cpu, uint64_t max_cycles) {
    cpu->running = true;
    uint64_t start_cycles = cpu->cycle_count;
    
    while (cpu->running && (cpu->cycle_count - start_cycles) < max_cycles) {
        if (!simple_cpu_step(cpu)) {
            break;
        }
    }
}

// Load program into memory
bool simple_cpu_load_program(simple_cpu_t* cpu, const uint8_t* program, size_t size, uint16_t address) {
    if (address + size > MEMORY_SIZE) return false;
    
    for (size_t i = 0; i < size; i++) {
        simple_cpu_write_memory(cpu, address + i, program[i]);
    }
    return true;
}

// Interactive mode
void simple_cpu_interactive(simple_cpu_t* cpu) {
    char line[256];
    char command[64];
    char arg1[64];
    char arg2[64];
    
    printf("Simple CPU Simulator - Interactive Mode\n");
    printf("Type 'help' for available commands\n\n");
    
    while (true) {
        printf("cpu> ");
        fflush(stdout);
        
        if (!fgets(line, sizeof(line), stdin)) break;
        
        int args = sscanf(line, "%63s %63s %63s", command, arg1, arg2);
        
        if (strcmp(command, "quit") == 0 || strcmp(command, "q") == 0) {
            break;
        } else if (strcmp(command, "help") == 0) {
            printf("Available commands:\n");
            printf("  step, s        - Execute single instruction\n");
            printf("  run, r [cycles] - Run program\n");
            printf("  stop           - Stop execution\n");
            printf("  reset          - Reset CPU\n");
            printf("  status         - Show CPU status\n");
            printf("  mem addr [size] - Dump memory\n");
            printf("  load file addr - Load program from file\n");
            printf("  quit, q        - Exit simulator\n");
        } else if (strcmp(command, "step") == 0 || strcmp(command, "s") == 0) {
            if (simple_cpu_step(cpu)) {
                simple_cpu_print_status(cpu);
            } else {
                printf("Execution stopped\n");
            }
        } else if (strcmp(command, "run") == 0 || strcmp(command, "r") == 0) {
            uint64_t max_cycles = 1000;
            if (args > 1) {
                max_cycles = strtoull(arg1, NULL, 0);
            }
            simple_cpu_run(cpu, max_cycles);
            simple_cpu_print_status(cpu);
        } else if (strcmp(command, "stop") == 0) {
            cpu->running = false;
            printf("Execution stopped\n");
        } else if (strcmp(command, "reset") == 0) {
            simple_cpu_reset(cpu);
            printf("CPU reset\n");
        } else if (strcmp(command, "status") == 0) {
            simple_cpu_print_status(cpu);
        } else if (strcmp(command, "mem") == 0) {
            if (args > 1) {
                uint16_t addr = strtol(arg1, NULL, 0);
                uint16_t size = (args > 2) ? strtol(arg2, NULL, 0) : 16;
                printf("Memory dump from 0x%04X to 0x%04X:\n", addr, addr + size - 1);
                for (uint16_t i = 0; i < size; i++) {
                    if (i % 16 == 0) printf("0x%04X: ", addr + i);
                    printf("%02X ", simple_cpu_read_memory(cpu, addr + i));
                    if (i % 16 == 15) printf("\n");
                }
                if (size % 16 != 0) printf("\n");
            } else {
                printf("Usage: mem ADDRESS [SIZE]\n");
            }
        } else if (strcmp(command, "load") == 0) {
            if (args > 1) {
                uint16_t addr = (args > 2) ? strtol(arg2, NULL, 0) : 0x0200;
                FILE* file = fopen(arg1, "rb");
                if (file) {
                    fseek(file, 0, SEEK_END);
                    long size = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    
                    uint8_t* program = malloc(size);
                    if (program) {
                        fread(program, 1, size, file);
                        if (simple_cpu_load_program(cpu, program, size, addr)) {
                            printf("Loaded %ld bytes from %s at 0x%04X\n", size, arg1, addr);
                        } else {
                            printf("Failed to load program\n");
                        }
                        free(program);
                    }
                    fclose(file);
                } else {
                    printf("Cannot open file: %s\n", arg1);
                }
            } else {
                printf("Usage: load FILE [ADDRESS]\n");
            }
        } else {
            printf("Unknown command: %s\n", command);
            printf("Type 'help' for available commands\n");
        }
    }
}

int main(int argc, char* argv[]) {
    simple_cpu_t* cpu = simple_cpu_create();
    if (!cpu) {
        fprintf(stderr, "Failed to create CPU instance\n");
        return 1;
    }
    
    // Check for command line arguments
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("Simple CPU Simulator\n");
            printf("Usage: %s [program.bin] [address]\n", argv[0]);
            printf("\nIf no program is specified, starts in interactive mode.\n");
            printf("If a program is specified, loads it and starts execution.\n");
            simple_cpu_destroy(cpu);
            return 0;
        }
        
        // Load program from file
        uint16_t addr = (argc > 2) ? strtol(argv[2], NULL, 0) : 0x0200;
        FILE* file = fopen(argv[1], "rb");
        if (file) {
            fseek(file, 0, SEEK_END);
            long size = ftell(file);
            fseek(file, 0, SEEK_SET);
            
            uint8_t* program = malloc(size);
            if (program) {
                fread(program, 1, size, file);
                if (simple_cpu_load_program(cpu, program, size, addr)) {
                    printf("Loaded %ld bytes from %s at 0x%04X\n", size, argv[1], addr);
                    simple_cpu_set_register16(cpu, REG_PC, addr);
                    simple_cpu_run(cpu, 1000);
                } else {
                    printf("Failed to load program\n");
                }
                free(program);
            }
            fclose(file);
        } else {
            printf("Cannot open file: %s\n", argv[1]);
        }
    } else {
        // Interactive mode
        simple_cpu_interactive(cpu);
    }
    
    simple_cpu_destroy(cpu);
    return 0;
}

