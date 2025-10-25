#include "cpu.h"
#include "memory.h"
#include "devices.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

// Command line options
typedef struct {
    char* program_file;
    uint16_t load_address;
    bool run_immediately;
    uint32_t frequency_hz;
    bool trace_enabled;
    uint16_t breakpoint_addr;
    uint16_t watch_addr;
    uint64_t max_cycles;
    char* until_condition;
    bool help_requested;
} cli_options_t;

// Function prototypes
void print_usage(const char* program_name);
void print_help(void);
bool parse_cli_options(int argc, char* argv[], cli_options_t* options);
void print_cpu_status(cpu_state_t* cpu);
void run_interactive_mode(cpu_state_t* cpu);
void run_batch_mode(cpu_state_t* cpu, cli_options_t* options);

int main(int argc, char* argv[]) {
    cli_options_t options = {0};
    
    // Parse command line options
    if (!parse_cli_options(argc, argv, &options)) {
        return 1;
    }
    
    if (options.help_requested) {
        print_help();
        return 0;
    }
    
    // Create CPU instance
    cpu_state_t* cpu = cpu_create();
    if (!cpu) {
        fprintf(stderr, "Failed to create CPU instance\n");
        return 1;
    }
    
    // Set CPU frequency
    if (options.frequency_hz > 0) {
        cpu_set_frequency(cpu, options.frequency_hz);
    }
    
    // Enable trace if requested
    if (options.trace_enabled) {
        cpu_enable_trace(cpu, true);
    }
    
    // Set breakpoint if specified
    if (options.breakpoint_addr != 0) {
        cpu_set_breakpoint(cpu, options.breakpoint_addr);
    }
    
    // Set watchpoint if specified
    if (options.watch_addr != 0) {
        cpu_set_watchpoint(cpu, options.watch_addr);
    }
    
    // Load program if specified
    if (options.program_file) {
        if (!cpu_load_file(cpu, options.program_file, options.load_address)) {
            fprintf(stderr, "Failed to load program from %s\n", options.program_file);
            cpu_destroy(cpu);
            return 1;
        }
        printf("Loaded program from %s at address 0x%04X\n", 
               options.program_file, options.load_address);
    }
    
    // Run in appropriate mode
    if (options.run_immediately) {
        run_batch_mode(cpu, &options);
    } else {
        run_interactive_mode(cpu);
    }
    
    // Cleanup
    cpu_destroy(cpu);
    return 0;
}

void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] [PROGRAM]\n", program_name);
    printf("\nOptions:\n");
    printf("  -a, --addr ADDRESS     Load program at ADDRESS (default: 0x0200)\n");
    printf("  -r, --run              Run program immediately\n");
    printf("  -f, --freq HZ           Set CPU frequency in Hz (default: 1000000)\n");
    printf("  -t, --trace            Enable instruction tracing\n");
    printf("  -b, --break ADDRESS    Set breakpoint at ADDRESS\n");
    printf("  -w, --watch ADDRESS    Set watchpoint at ADDRESS\n");
    printf("  -c, --cycles COUNT      Maximum cycles to execute\n");
    printf("  -u, --until CONDITION  Run until condition is met\n");
    printf("  -h, --help             Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s examples/hello.bin --addr 0x0200 --run\n", program_name);
    printf("  %s --trace --break 0x0300\n", program_name);
    printf("  %s examples/addloop.bin --freq 500000 --cycles 10000\n", program_name);
}

void print_help(void) {
    print_usage("cpu-sim");
    printf("\nInteractive Commands:\n");
    printf("  step, s                Execute single instruction\n");
    printf("  run, r                 Run program\n");
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
    printf("  load FILE ADDRESS      Load program from file\n");
    printf("  save FILE ADDRESS SIZE Save memory to file\n");
    printf("  quit, q                Exit simulator\n");
}

bool parse_cli_options(int argc, char* argv[], cli_options_t* options) {
    static struct option long_options[] = {
        {"addr", required_argument, 0, 'a'},
        {"run", no_argument, 0, 'r'},
        {"freq", required_argument, 0, 'f'},
        {"trace", no_argument, 0, 't'},
        {"break", required_argument, 0, 'b'},
        {"watch", required_argument, 0, 'w'},
        {"cycles", required_argument, 0, 'c'},
        {"until", required_argument, 0, 'u'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    // Set defaults
    options->load_address = 0x0200;
    options->run_immediately = false;
    options->frequency_hz = 0;
    options->trace_enabled = false;
    options->breakpoint_addr = 0;
    options->watch_addr = 0;
    options->max_cycles = 0;
    options->until_condition = NULL;
    options->help_requested = false;
    
    while ((c = getopt_long(argc, argv, "a:rf:tb:w:c:u:h", long_options, &option_index)) != -1) {
        switch (c) {
            case 'a':
                options->load_address = strtol(optarg, NULL, 0);
                break;
            case 'r':
                options->run_immediately = true;
                break;
            case 'f':
                options->frequency_hz = strtol(optarg, NULL, 0);
                break;
            case 't':
                options->trace_enabled = true;
                break;
            case 'b':
                options->breakpoint_addr = strtol(optarg, NULL, 0);
                break;
            case 'w':
                options->watch_addr = strtol(optarg, NULL, 0);
                break;
            case 'c':
                options->max_cycles = strtoull(optarg, NULL, 0);
                break;
            case 'u':
                options->until_condition = optarg;
                break;
            case 'h':
                options->help_requested = true;
                break;
            case '?':
                return false;
            default:
                return false;
        }
    }
    
    // Get program file from remaining arguments
    if (optind < argc) {
        options->program_file = argv[optind];
    }
    
    return true;
}

void print_cpu_status(cpu_state_t* cpu) {
    printf("CPU Status: %s\n", cpu_get_status_string(cpu));
    cpu_print_registers(cpu);
    cpu_print_flags(cpu);
    printf("Cycles: %llu, Instructions: %u\n", 
           (unsigned long long)cpu_get_cycle_count(cpu), 
           cpu_get_instruction_count(cpu));
}

void run_interactive_mode(cpu_state_t* cpu) {
    char line[256];
    char command[64];
    char arg1[64];
    char arg2[64];
    char arg3[64];
    
    printf("Software CPU Simulator - Interactive Mode\n");
    printf("Type 'help' for available commands\n\n");
    
    while (true) {
        printf("cpu> ");
        fflush(stdout);
        
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        
        // Parse command
        int args = sscanf(line, "%63s %63s %63s %63s", command, arg1, arg2, arg3);
        
        if (strcmp(command, "quit") == 0 || strcmp(command, "q") == 0) {
            break;
        } else if (strcmp(command, "help") == 0) {
            print_help();
        } else if (strcmp(command, "step") == 0 || strcmp(command, "s") == 0) {
            if (cpu_step(cpu)) {
                print_cpu_status(cpu);
            } else {
                printf("Execution stopped\n");
            }
        } else if (strcmp(command, "run") == 0 || strcmp(command, "r") == 0) {
            uint64_t max_cycles = 0;
            if (args > 1) {
                max_cycles = strtoull(arg1, NULL, 0);
            }
            cpu_run(cpu, max_cycles);
            print_cpu_status(cpu);
        } else if (strcmp(command, "stop") == 0) {
            cpu_stop(cpu);
            printf("Execution stopped\n");
        } else if (strcmp(command, "reset") == 0) {
            cpu_reset(cpu);
            printf("CPU reset\n");
        } else if (strcmp(command, "regs") == 0) {
            cpu_print_registers(cpu);
        } else if (strcmp(command, "flags") == 0) {
            cpu_print_flags(cpu);
        } else if (strcmp(command, "status") == 0) {
            print_cpu_status(cpu);
        } else if (strcmp(command, "mem") == 0) {
            if (args > 1) {
                uint16_t addr = strtol(arg1, NULL, 0);
                uint16_t size = (args > 2) ? strtol(arg2, NULL, 0) : 16;
                memory_dump(cpu->memory, addr, addr + size - 1);
            } else {
                printf("Usage: mem ADDRESS [SIZE]\n");
            }
        } else if (strcmp(command, "disasm") == 0) {
            if (args > 1) {
                uint16_t addr = strtol(arg1, NULL, 0);
                uint16_t size = (args > 2) ? strtol(arg2, NULL, 0) : 16;
                memory_dump_disasm(cpu->memory, addr, addr + size - 1);
            } else {
                printf("Usage: disasm ADDRESS [SIZE]\n");
            }
        } else if (strcmp(command, "break") == 0) {
            if (args > 1) {
                uint16_t addr = strtol(arg1, NULL, 0);
                cpu_set_breakpoint(cpu, addr);
                printf("Breakpoint set at 0x%04X\n", addr);
            } else {
                printf("Usage: break ADDRESS\n");
            }
        } else if (strcmp(command, "watch") == 0) {
            if (args > 1) {
                uint16_t addr = strtol(arg1, NULL, 0);
                cpu_set_watchpoint(cpu, addr);
                printf("Watchpoint set at 0x%04X\n", addr);
            } else {
                printf("Usage: watch ADDRESS\n");
            }
        } else if (strcmp(command, "trace") == 0) {
            if (args > 1) {
                bool enable = (strcmp(arg1, "on") == 0);
                cpu_enable_trace(cpu, enable);
                printf("Tracing %s\n", enable ? "enabled" : "disabled");
            } else {
                printf("Tracing is %s\n", cpu->trace_enabled ? "enabled" : "disabled");
            }
        } else if (strcmp(command, "load") == 0) {
            if (args > 1) {
                uint16_t addr = (args > 2) ? strtol(arg2, NULL, 0) : 0x0200;
                if (cpu_load_file(cpu, arg1, addr)) {
                    printf("Loaded %s at 0x%04X\n", arg1, addr);
                } else {
                    printf("Failed to load %s\n", arg1);
                }
            } else {
                printf("Usage: load FILE [ADDRESS]\n");
            }
        } else if (strcmp(command, "save") == 0) {
            if (args > 2) {
                uint16_t addr = strtol(arg2, NULL, 0);
                uint16_t size = strtol(arg3, NULL, 0);
                FILE* file = fopen(arg1, "wb");
                if (file) {
                    fwrite(&cpu->memory[addr], 1, size, file);
                    fclose(file);
                    printf("Saved %d bytes from 0x%04X to %s\n", size, addr, arg1);
                } else {
                    printf("Failed to save to %s\n", arg1);
                }
            } else {
                printf("Usage: save FILE ADDRESS SIZE\n");
            }
        } else {
            printf("Unknown command: %s\n", command);
            printf("Type 'help' for available commands\n");
        }
    }
}

void run_batch_mode(cpu_state_t* cpu, cli_options_t* options) {
    printf("Running program in batch mode...\n");
    
    // Reset CPU to load address
    cpu_reset_to_address(cpu, options->load_address);
    
    // Run program
    uint64_t max_cycles = options->max_cycles;
    if (max_cycles == 0) {
        max_cycles = 1000000; // Default limit
    }
    
    cpu_run(cpu, max_cycles);
    
    // Print final status
    print_cpu_status(cpu);
    
    if (cpu_is_running(cpu)) {
        printf("Program completed successfully\n");
    } else {
        printf("Program stopped\n");
    }
}

