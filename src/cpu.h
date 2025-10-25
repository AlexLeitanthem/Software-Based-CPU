#ifndef CPU_H
#define CPU_H

#include "isa.h"
#include <stdbool.h>
#include <stdint.h>

// CPU configuration
#define CPU_FREQUENCY_HZ 1000000  // 1 MHz default
#define MAX_CYCLES_PER_TICK 1000

// CPU state structure is defined in isa.h
// Additional CPU-specific fields are added in cpu.c

// CPU functions
cpu_state_t* cpu_create(void);
void cpu_destroy(cpu_state_t* cpu);
void cpu_reset(cpu_state_t* cpu);
void cpu_reset_to_address(cpu_state_t* cpu, uint16_t address);

// Execution control
bool cpu_step(cpu_state_t* cpu);
bool cpu_run(cpu_state_t* cpu, uint64_t max_cycles);
void cpu_stop(cpu_state_t* cpu);

// Interrupt handling
void cpu_irq(cpu_state_t* cpu);
void cpu_nmi(cpu_state_t* cpu);
void cpu_handle_interrupts(cpu_state_t* cpu);

// Clock control
void cpu_set_frequency(cpu_state_t* cpu, uint32_t hz);
void cpu_throttle(cpu_state_t* cpu);

// Debug functions
void cpu_set_breakpoint(cpu_state_t* cpu, uint16_t address);
void cpu_clear_breakpoint(cpu_state_t* cpu);
void cpu_set_watchpoint(cpu_state_t* cpu, uint16_t address);
void cpu_clear_watchpoint(cpu_state_t* cpu);
void cpu_enable_trace(cpu_state_t* cpu, bool enable);

// Status functions
void cpu_print_registers(cpu_state_t* cpu);
void cpu_print_flags(cpu_state_t* cpu);
void cpu_print_status(cpu_state_t* cpu);
const char* cpu_get_status_string(cpu_state_t* cpu);

// Memory operations
bool cpu_load_program(cpu_state_t* cpu, const uint8_t* program, size_t size, uint16_t address);
bool cpu_load_file(cpu_state_t* cpu, const char* filename, uint16_t address);

// Utility functions
uint16_t cpu_get_pc(cpu_state_t* cpu);
uint16_t cpu_get_sp(cpu_state_t* cpu);
uint8_t cpu_get_a(cpu_state_t* cpu);
uint8_t cpu_get_flags(cpu_state_t* cpu);
bool cpu_is_running(cpu_state_t* cpu);
uint64_t cpu_get_cycle_count(cpu_state_t* cpu);
uint32_t cpu_get_instruction_count(cpu_state_t* cpu);

#endif // CPU_H
