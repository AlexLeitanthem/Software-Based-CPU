#include "cpu.h"
#include "memory.h"
#include "devices.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

// Create new CPU instance
cpu_state_t* cpu_create(void) {
    cpu_state_t* cpu = malloc(sizeof(cpu_state_t));
    if (!cpu) {
        return NULL;
    }
    
    // Allocate memory
    cpu->memory = malloc(MEMORY_SIZE);
    if (!cpu->memory) {
        free(cpu);
        return NULL;
    }
    
    // Initialize state
    cpu_reset(cpu);
    cpu->frequency_hz = CPU_FREQUENCY_HZ;
    cpu->cycles_per_second = 0;
    cpu->last_tick_time = 0;
    
    // Initialize memory system
    memory_init(cpu->memory);
    
    // Initialize devices
    devices_init();
    
    return cpu;
}

// Destroy CPU instance
void cpu_destroy(cpu_state_t* cpu) {
    if (cpu) {
        if (cpu->memory) {
            free(cpu->memory);
        }
        devices_cleanup();
        free(cpu);
    }
}

// Reset CPU to initial state
void cpu_reset(cpu_state_t* cpu) {
    // Clear all registers
    memset(cpu->regs, 0, sizeof(cpu->regs));
    
    // Set initial stack pointer (grows downward from 0x7FFF)
    isa_set_register16(cpu, REG_SP, 0x7FFF);
    
    // Set program counter to reset vector (0xFFFC)
    isa_set_register16(cpu, REG_PC, 0xFFFC);
    
    // Clear flags
    cpu->flags = 0;
    
    // Clear control flags
    cpu->running = false;
    cpu->irq_pending = false;
    cpu->nmi_pending = false;
    
    // Reset counters
    cpu->cycle_count = 0;
    cpu->instruction_count = 0;
    
    // Clear debug flags
    cpu->trace_enabled = false;
    cpu->breakpoint_hit = false;
    cpu->breakpoint_addr = 0;
    cpu->watch_addr = 0;
    cpu->watch_hit = false;
    
    // Clear memory
    memset(cpu->memory, 0, MEMORY_SIZE);
    
    // Initialize memory map
    memory_init(cpu->memory);
}

// Reset CPU to specific address
void cpu_reset_to_address(cpu_state_t* cpu, uint16_t address) {
    // Perform a lightweight reset that preserves memory contents but
    // initializes registers and control flags. Tests load a program into
    // memory and then call this function, so clearing memory here would
    // erase the loaded program.
    memset(cpu->regs, 0, sizeof(cpu->regs));
    isa_set_register16(cpu, REG_SP, 0x7FFF);
    isa_set_register16(cpu, REG_PC, address);
    cpu->flags = 0;
    cpu->running = false;
    cpu->irq_pending = false;
    cpu->nmi_pending = false;
    cpu->cycle_count = 0;
    cpu->instruction_count = 0;
    cpu->trace_enabled = false;
    cpu->breakpoint_hit = false;
    cpu->breakpoint_addr = 0;
    cpu->watch_addr = 0;
    cpu->watch_hit = false;
}

// Execute single instruction
bool cpu_step(cpu_state_t* cpu) {
    // Allow single-step even when the CPU is not in 'running' mode.
    // Tests call cpu_step() directly after reset without setting cpu->running.
    
    // Check for breakpoints
    uint16_t pc = isa_get_register16(cpu, REG_PC);
    if (cpu->breakpoint_addr != 0 && pc == cpu->breakpoint_addr) {
        cpu->breakpoint_hit = true;
        cpu->running = false;
        printf("Breakpoint hit at 0x%04X\n", pc);
        return false;
    }
    
    // Check for watchpoints
    if (cpu->watch_addr != 0) {
        // This would need to be implemented in memory system
        // For now, just a placeholder
    }
    
    // Handle interrupts
    cpu_handle_interrupts(cpu);
    
    // Execute instruction
    bool result = isa_execute_instruction(cpu);
    
    // Print trace if enabled
    if (cpu->trace_enabled) {
        cpu_print_status(cpu);
    }
    
    return result;
}

// Run CPU for specified number of cycles
bool cpu_run(cpu_state_t* cpu, uint64_t max_cycles) {
    cpu->running = true;
    uint64_t start_cycles = cpu->cycle_count;
    
    while (cpu->running && (cpu->cycle_count - start_cycles) < max_cycles) {
        if (!cpu_step(cpu)) {
            break;
        }
        
        // Throttle execution if needed
        cpu_throttle(cpu);
    }
    
    return cpu->running;
}

// Stop CPU execution
void cpu_stop(cpu_state_t* cpu) {
    cpu->running = false;
}

// Trigger IRQ
void cpu_irq(cpu_state_t* cpu) {
    cpu->irq_pending = true;
}

// Trigger NMI
void cpu_nmi(cpu_state_t* cpu) {
    cpu->nmi_pending = true;
}

// Handle interrupts
void cpu_handle_interrupts(cpu_state_t* cpu) {
    // NMI has higher priority than IRQ
    if (cpu->nmi_pending) {
        cpu->nmi_pending = false;
        
        // Save current state
        isa_push16(cpu, isa_get_register16(cpu, REG_PC));
        isa_push(cpu, cpu->flags);
        
        // Set interrupt disable flag
        isa_set_flag(cpu, FLAG_INTERRUPT);
        
        // Jump to NMI vector (0xFFFA)
        uint16_t nmi_vector = isa_read_memory(cpu, 0xFFFA) | (isa_read_memory(cpu, 0xFFFB) << 8);
        isa_set_register16(cpu, REG_PC, nmi_vector);
        
        return;
    }
    
    // Handle IRQ if interrupts are enabled
    if (cpu->irq_pending && !isa_get_flag(cpu, FLAG_INTERRUPT)) {
        cpu->irq_pending = false;
        
        // Save current state
        isa_push16(cpu, isa_get_register16(cpu, REG_PC));
        isa_push(cpu, cpu->flags);
        
        // Set interrupt disable flag
        isa_set_flag(cpu, FLAG_INTERRUPT);
        
        // Jump to IRQ vector (0xFFFE)
        uint16_t irq_vector = isa_read_memory(cpu, 0xFFFE) | (isa_read_memory(cpu, 0xFFFF) << 8);
        isa_set_register16(cpu, REG_PC, irq_vector);
    }
}

// Set CPU frequency
void cpu_set_frequency(cpu_state_t* cpu, uint32_t hz) {
    cpu->frequency_hz = hz;
    cpu->cycles_per_second = hz;
}

// Throttle CPU execution
void cpu_throttle(cpu_state_t* cpu) {
    if (cpu->frequency_hz == 0) {
        return; // No throttling
    }
    
    static uint64_t last_time = 0;
    uint64_t current_time = 0;
    
#ifdef _WIN32
    current_time = GetTickCount64();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    current_time = tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
    
    if (last_time == 0) {
        last_time = current_time;
        return;
    }
    
    uint64_t elapsed = current_time - last_time;
    uint64_t expected_cycles = (elapsed * cpu->frequency_hz) / 1000;
    
    if (cpu->cycle_count >= expected_cycles) {
        // Sleep to maintain timing
        uint64_t sleep_time = (cpu->cycle_count - expected_cycles) * 1000 / cpu->frequency_hz;
        if (sleep_time > 0) {
#ifdef _WIN32
            Sleep(sleep_time);
#else
            usleep(sleep_time * 1000);
#endif
        }
    }
    
    last_time = current_time;
}

// Set breakpoint
void cpu_set_breakpoint(cpu_state_t* cpu, uint16_t address) {
    cpu->breakpoint_addr = address;
}

// Clear breakpoint
void cpu_clear_breakpoint(cpu_state_t* cpu) {
    cpu->breakpoint_addr = 0;
    cpu->breakpoint_hit = false;
}

// Set watchpoint
void cpu_set_watchpoint(cpu_state_t* cpu, uint16_t address) {
    cpu->watch_addr = address;
}

// Clear watchpoint
void cpu_clear_watchpoint(cpu_state_t* cpu) {
    cpu->watch_addr = 0;
    cpu->watch_hit = false;
}

// Enable/disable trace
void cpu_enable_trace(cpu_state_t* cpu, bool enable) {
    cpu->trace_enabled = enable;
}

// Print register values
void cpu_print_registers(cpu_state_t* cpu) {
    printf("Registers:\n");
    printf("  A = 0x%02X (%3d)  B = 0x%02X (%3d)\n", 
           isa_get_register(cpu, REG_A), isa_get_register(cpu, REG_A),
           isa_get_register(cpu, REG_B), isa_get_register(cpu, REG_B));
    printf("  C = 0x%02X (%3d)  D = 0x%02X (%3d)\n", 
           isa_get_register(cpu, REG_C), isa_get_register(cpu, REG_C),
           isa_get_register(cpu, REG_D), isa_get_register(cpu, REG_D));
    printf("  X = 0x%04X (%5d)  Y = 0x%04X (%5d)\n", 
           isa_get_register16(cpu, REG_X), isa_get_register16(cpu, REG_X),
           isa_get_register16(cpu, REG_Y), isa_get_register16(cpu, REG_Y));
    printf("  SP= 0x%04X (%5d)  PC= 0x%04X (%5d)\n", 
           isa_get_register16(cpu, REG_SP), isa_get_register16(cpu, REG_SP),
           isa_get_register16(cpu, REG_PC), isa_get_register16(cpu, REG_PC));
}

// Print flag values
void cpu_print_flags(cpu_state_t* cpu) {
    printf("Flags: ");
    printf("%s", isa_get_flag(cpu, FLAG_ZERO) ? "Z" : "-");
    printf("%s", isa_get_flag(cpu, FLAG_NEGATIVE) ? "N" : "-");
    printf("%s", isa_get_flag(cpu, FLAG_CARRY) ? "C" : "-");
    printf("%s", isa_get_flag(cpu, FLAG_OVERFLOW) ? "V" : "-");
    printf("%s", isa_get_flag(cpu, FLAG_INTERRUPT) ? "I" : "-");
    printf(" (0x%02X)\n", cpu->flags);
}

// Print CPU status
void cpu_print_status(cpu_state_t* cpu) {
    cpu_print_registers(cpu);
    cpu_print_flags(cpu);
    printf("Cycles: %llu, Instructions: %u\n", 
           (unsigned long long)cpu->cycle_count, cpu->instruction_count);
    if (cpu->running) {
        printf("Status: RUNNING\n");
    } else {
        printf("Status: STOPPED\n");
    }
}

// Get status string
const char* cpu_get_status_string(cpu_state_t* cpu) {
    static char status[256];
    snprintf(status, sizeof(status), 
             "PC=0x%04X SP=0x%04X A=0x%02X Flags=0x%02X Cycles=%llu",
             isa_get_register16(cpu, REG_PC),
             isa_get_register16(cpu, REG_SP),
             isa_get_register(cpu, REG_A),
             cpu->flags,
             (unsigned long long)cpu->cycle_count);
    return status;
}

// Load program into memory
bool cpu_load_program(cpu_state_t* cpu, const uint8_t* program, size_t size, uint16_t address) {
    if (address + size > MEMORY_SIZE) {
        return false;
    }
    
    memcpy(&cpu->memory[address], program, size);
    return true;
}

// Load program from file
bool cpu_load_file(cpu_state_t* cpu, const char* filename, uint16_t address) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return false;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (address + size > MEMORY_SIZE) {
        fclose(file);
        return false;
    }
    
    // Read file into memory
    size_t bytes_read = fread(&cpu->memory[address], 1, size, file);
    fclose(file);
    
    return bytes_read == size;
}

// Utility functions
uint16_t cpu_get_pc(cpu_state_t* cpu) {
    return isa_get_register16(cpu, REG_PC);
}

uint16_t cpu_get_sp(cpu_state_t* cpu) {
    return isa_get_register16(cpu, REG_SP);
}

uint8_t cpu_get_a(cpu_state_t* cpu) {
    return isa_get_register(cpu, REG_A);
}

uint8_t cpu_get_flags(cpu_state_t* cpu) {
    return cpu->flags;
}

bool cpu_is_running(cpu_state_t* cpu) {
    return cpu->running;
}

uint64_t cpu_get_cycle_count(cpu_state_t* cpu) {
    return cpu->cycle_count;
}

uint32_t cpu_get_instruction_count(cpu_state_t* cpu) {
    return cpu->instruction_count;
}

