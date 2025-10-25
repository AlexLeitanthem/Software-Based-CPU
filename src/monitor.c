#include "cpu.h"
#include "memory.h"
#include "devices.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

// Monitor state
typedef struct {
    cpu_state_t* cpu;
    bool running;
    char* script_file;
    bool verbose;
} monitor_state_t;

// Function prototypes
void print_usage(const char* program_name);
void print_help(void);
bool parse_cli_options(int argc, char* argv[], monitor_state_t* state);
void run_interactive_monitor(monitor_state_t* state);
void run_script_monitor(monitor_state_t* state);
bool execute_command(monitor_state_t* state, const char* command);
void print_cpu_status(monitor_state_t* state);
void print_memory_dump(monitor_state_t* state, uint16_t address, uint16_t size);
void print_disassembly(monitor_state_t* state, uint16_t address, uint16_t size);
void print_help_text(void);

int main(int argc, char* argv[]) {
    monitor_state_t state = {0};
    
    // Parse command line options
    if (!parse_cli_options(argc, argv, &state)) {
        return 1;
    }
    
    // Create CPU instance
    state.cpu = cpu_create();
    if (!state.cpu) {
        fprintf(stderr, "Failed to create CPU instance\n");
        return 1;
    }
    
    // Run monitor
    if (state.script_file) {
        run_script_monitor(&state);
    } else {
        run_interactive_monitor(&state);
    }
    
    // Cleanup
    cpu_destroy(state.cpu);
    return 0;
}

void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("\nOptions:\n");
    printf("  -s, --script FILE       Run script from file\n");
    printf("  -v, --verbose          Verbose output\n");
    printf("  -h, --help             Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s\n", program_name);
    printf("  %s -s commands.txt\n", program_name);
    printf("  %s --help\n", program_name);
}

void print_help(void) {
    print_usage("monitor");
    printf("\nMonitor Commands:\n");
    printf("  load FILE ADDRESS      Load program from file\n");
    printf("  save FILE ADDRESS SIZE Save memory to file\n");
    printf("  step, s                Execute single instruction\n");
    printf("  run, r [CYCLES]        Run program\n");
    printf("  stop                   Stop execution\n");
    printf("  reset                  Reset CPU\n");
    printf("  regs                   Show registers\n");
    printf("  flags                  Show flags\n");
    printf("  status                 Show CPU status\n");
    printf("  mem ADDRESS [SIZE]     Dump memory\n");
    printf("  disasm ADDRESS [SIZE]  Disassemble memory\n");
    printf("  break ADDRESS          Set breakpoint\n");
    printf("  watch ADDRESS          Set watchpoint\n");
    printf("  trace [on|off]         Enable/disable tracing\n");
    printf("  freq HZ                Set CPU frequency\n");
    printf("  irq                    Trigger IRQ\n");
    printf("  nmi                    Trigger NMI\n");
    printf("  quit, q                Exit monitor\n");
    printf("  help                   Show this help\n");
}

bool parse_cli_options(int argc, char* argv[], monitor_state_t* state) {
    static struct option long_options[] = {
        {"script", required_argument, 0, 's'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    // Set defaults
    state->cpu = NULL;
    state->running = true;
    state->script_file = NULL;
    state->verbose = false;
    
    while ((c = getopt_long(argc, argv, "s:vh", long_options, &option_index)) != -1) {
        switch (c) {
            case 's':
                state->script_file = optarg;
                break;
            case 'v':
                state->verbose = true;
                break;
            case 'h':
                print_help();
                return false;
            case '?':
                return false;
            default:
                return false;
        }
    }
    
    return true;
}

void run_interactive_monitor(monitor_state_t* state) {
    char line[256];
    char command[64];
    char arg1[64];
    char arg2[64];
    char arg3[64];
    
    printf("Software CPU Monitor - Interactive Mode\n");
    printf("Type 'help' for available commands\n\n");
    
    while (state->running) {
        printf("monitor> ");
        fflush(stdout);
        
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        
        // Parse command
        int args = sscanf(line, "%63s %63s %63s %63s", command, arg1, arg2, arg3);
        
        if (!execute_command(state, line)) {
            printf("Unknown command: %s\n", command);
            printf("Type 'help' for available commands\n");
        }
    }
}

void run_script_monitor(monitor_state_t* state) {
    FILE* script = fopen(state->script_file, "r");
    if (!script) {
        fprintf(stderr, "Cannot open script file: %s\n", state->script_file);
        return;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), script)) {
        // Remove newline
        line[strcspn(line, "\n")] = '\0';
        
        if (state->verbose) {
            printf("Executing: %s\n", line);
        }
        
        if (!execute_command(state, line)) {
            printf("Error executing command: %s\n", line);
        }
    }
    
    fclose(script);
}

bool execute_command(monitor_state_t* state, const char* command) {
    char cmd[64];
    char arg1[64];
    char arg2[64];
    char arg3[64];
    
    int args = sscanf(command, "%63s %63s %63s %63s", cmd, arg1, arg2, arg3);
    
    if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "q") == 0) {
        state->running = false;
        return true;
    } else if (strcmp(cmd, "help") == 0) {
        print_help_text();
        return true;
    } else if (strcmp(cmd, "load") == 0) {
        if (args > 1) {
            uint16_t addr = (args > 2) ? strtol(arg2, NULL, 0) : 0x0200;
            if (cpu_load_file(state->cpu, arg1, addr)) {
                printf("Loaded %s at 0x%04X\n", arg1, addr);
            } else {
                printf("Failed to load %s\n", arg1);
            }
        } else {
            printf("Usage: load FILE [ADDRESS]\n");
        }
        return true;
    } else if (strcmp(cmd, "save") == 0) {
        if (args > 2) {
            uint16_t addr = strtol(arg2, NULL, 0);
            uint16_t size = strtol(arg3, NULL, 0);
            FILE* file = fopen(arg1, "wb");
            if (file) {
                fwrite(&state->cpu->memory[addr], 1, size, file);
                fclose(file);
                printf("Saved %d bytes from 0x%04X to %s\n", size, addr, arg1);
            } else {
                printf("Failed to save to %s\n", arg1);
            }
        } else {
            printf("Usage: save FILE ADDRESS SIZE\n");
        }
        return true;
    } else if (strcmp(cmd, "step") == 0 || strcmp(cmd, "s") == 0) {
        if (cpu_step(state->cpu)) {
            print_cpu_status(state);
        } else {
            printf("Execution stopped\n");
        }
        return true;
    } else if (strcmp(cmd, "run") == 0 || strcmp(cmd, "r") == 0) {
        uint64_t max_cycles = 0;
        if (args > 1) {
            max_cycles = strtoull(arg1, NULL, 0);
        }
        cpu_run(state->cpu, max_cycles);
        print_cpu_status(state);
        return true;
    } else if (strcmp(cmd, "stop") == 0) {
        cpu_stop(state->cpu);
        printf("Execution stopped\n");
        return true;
    } else if (strcmp(cmd, "reset") == 0) {
        cpu_reset(state->cpu);
        printf("CPU reset\n");
        return true;
    } else if (strcmp(cmd, "regs") == 0) {
        cpu_print_registers(state->cpu);
        return true;
    } else if (strcmp(cmd, "flags") == 0) {
        cpu_print_flags(state->cpu);
        return true;
    } else if (strcmp(cmd, "status") == 0) {
        print_cpu_status(state);
        return true;
    } else if (strcmp(cmd, "mem") == 0) {
        if (args > 1) {
            uint16_t addr = strtol(arg1, NULL, 0);
            uint16_t size = (args > 2) ? strtol(arg2, NULL, 0) : 16;
            print_memory_dump(state, addr, size);
        } else {
            printf("Usage: mem ADDRESS [SIZE]\n");
        }
        return true;
    } else if (strcmp(cmd, "disasm") == 0) {
        if (args > 1) {
            uint16_t addr = strtol(arg1, NULL, 0);
            uint16_t size = (args > 2) ? strtol(arg2, NULL, 0) : 16;
            print_disassembly(state, addr, size);
        } else {
            printf("Usage: disasm ADDRESS [SIZE]\n");
        }
        return true;
    } else if (strcmp(cmd, "break") == 0) {
        if (args > 1) {
            uint16_t addr = strtol(arg1, NULL, 0);
            cpu_set_breakpoint(state->cpu, addr);
            printf("Breakpoint set at 0x%04X\n", addr);
        } else {
            printf("Usage: break ADDRESS\n");
        }
        return true;
    } else if (strcmp(cmd, "watch") == 0) {
        if (args > 1) {
            uint16_t addr = strtol(arg1, NULL, 0);
            cpu_set_watchpoint(state->cpu, addr);
            printf("Watchpoint set at 0x%04X\n", addr);
        } else {
            printf("Usage: watch ADDRESS\n");
        }
        return true;
    } else if (strcmp(cmd, "trace") == 0) {
        if (args > 1) {
            bool enable = (strcmp(arg1, "on") == 0);
            cpu_enable_trace(state->cpu, enable);
            printf("Tracing %s\n", enable ? "enabled" : "disabled");
        } else {
            printf("Tracing is %s\n", state->cpu->trace_enabled ? "enabled" : "disabled");
        }
        return true;
    } else if (strcmp(cmd, "freq") == 0) {
        if (args > 1) {
            uint32_t hz = strtol(arg1, NULL, 0);
            cpu_set_frequency(state->cpu, hz);
            printf("CPU frequency set to %u Hz\n", hz);
        } else {
            printf("Usage: freq HZ\n");
        }
        return true;
    } else if (strcmp(cmd, "irq") == 0) {
        cpu_irq(state->cpu);
        printf("IRQ triggered\n");
        return true;
    } else if (strcmp(cmd, "nmi") == 0) {
        cpu_nmi(state->cpu);
        printf("NMI triggered\n");
        return true;
    }
    
    return false;
}

void print_cpu_status(monitor_state_t* state) {
    printf("CPU Status: %s\n", cpu_get_status_string(state->cpu));
    cpu_print_registers(state->cpu);
    cpu_print_flags(state->cpu);
    printf("Cycles: %llu, Instructions: %u\n", 
           (unsigned long long)cpu_get_cycle_count(state->cpu), 
           cpu_get_instruction_count(state->cpu));
}

void print_memory_dump(monitor_state_t* state, uint16_t address, uint16_t size) {
    printf("Memory dump from 0x%04X to 0x%04X:\n", address, address + size - 1);
    printf("Address  ");
    for (int i = 0; i < 16; i++) {
        printf("%02X ", i);
    }
    printf("\n");
    
    for (uint16_t addr = address; addr < address + size; addr += 16) {
        printf("0x%04X: ", addr);
        for (int i = 0; i < 16 && addr + i < address + size; i++) {
            printf("%02X ", state->cpu->memory[addr + i]);
        }
        printf("\n");
    }
}

void print_disassembly(monitor_state_t* state, uint16_t address, uint16_t size) {
    printf("Disassembly from 0x%04X to 0x%04X:\n", address, address + size - 1);
    printf("Address  Instruction\n");
    printf("-------- -----------\n");
    
    uint16_t addr = address;
    while (addr < address + size) {
        printf("0x%04X: ", addr);
        
        uint8_t opcode = state->cpu->memory[addr];
        const instruction_t* inst = isa_get_instruction((opcode_t)opcode);
        
        if (inst) {
            printf("%s", inst->mnemonic);
            
            // Parse operands (simplified)
            if (inst->opcode == OP_LDI && addr + 1 < address + size) {
                printf(" #$%02X", state->cpu->memory[addr + 1]);
                addr += 2;
            } else if (inst->opcode == OP_LDA && addr + 2 < address + size) {
                uint16_t target = state->cpu->memory[addr + 1] | 
                                 (state->cpu->memory[addr + 2] << 8);
                printf(" [$%04X]", target);
                addr += 3;
            } else {
                addr++;
            }
        } else {
            printf("???");
            addr++;
        }
        
        printf("\n");
    }
}

void print_help_text(void) {
    printf("Monitor Commands:\n");
    printf("  load FILE ADDRESS      Load program from file\n");
    printf("  save FILE ADDRESS SIZE Save memory to file\n");
    printf("  step, s                Execute single instruction\n");
    printf("  run, r [CYCLES]        Run program\n");
    printf("  stop                   Stop execution\n");
    printf("  reset                  Reset CPU\n");
    printf("  regs                   Show registers\n");
    printf("  flags                  Show flags\n");
    printf("  status                 Show CPU status\n");
    printf("  mem ADDRESS [SIZE]     Dump memory\n");
    printf("  disasm ADDRESS [SIZE]  Disassemble memory\n");
    printf("  break ADDRESS          Set breakpoint\n");
    printf("  watch ADDRESS          Set watchpoint\n");
    printf("  trace [on|off]         Enable/disable tracing\n");
    printf("  freq HZ                Set CPU frequency\n");
    printf("  irq                    Trigger IRQ\n");
    printf("  nmi                    Trigger NMI\n");
    printf("  quit, q                Exit monitor\n");
    printf("  help                   Show this help\n");
}

