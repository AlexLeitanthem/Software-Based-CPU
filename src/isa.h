#ifndef ISA_H
#define ISA_H

#include <stdint.h>
#include <stdbool.h>

// CPU Configuration
#define MEMORY_SIZE (64 * 1024)  // 64 KiB
#define RAM_START 0x0000
#define RAM_END 0x7FFF
#define MMIO_START 0x8000
#define MMIO_END 0xFEFF
#define VECTOR_START 0xFF00
#define VECTOR_END 0xFFFF

// Register definitions
typedef enum {
    REG_A = 0,    // 8-bit accumulator
    REG_B = 1,    // 8-bit general purpose
    REG_C = 2,    // 8-bit general purpose
    REG_D = 3,    // 8-bit general purpose
    REG_X = 4,    // 16-bit index register
    REG_Y = 5,    // 16-bit index register
    REG_SP = 6,   // 16-bit stack pointer
    REG_PC = 7,   // 16-bit program counter
    REG_FLAGS = 8 // 8-bit flags register
} register_t;

// Flag bits
#define FLAG_ZERO     (1 << 0)  // Zero flag
#define FLAG_NEGATIVE (1 << 1)  // Negative flag
#define FLAG_CARRY    (1 << 2)  // Carry flag
#define FLAG_OVERFLOW (1 << 3)  // Overflow flag
#define FLAG_INTERRUPT (1 << 4) // Interrupt enable flag

// Addressing modes
typedef enum {
    ADDR_IMMEDIATE = 0,    // #value
    ADDR_REGISTER = 1,     // register
    ADDR_ABSOLUTE = 2,     // [address]
    ADDR_X_INDEXED = 3,    // [X+offset]
    ADDR_Y_INDEXED = 4,    // [Y+offset]
    ADDR_SP_INDEXED = 5,   // [SP+offset]
    ADDR_RELATIVE = 6      // PC+offset (for branches)
} addressing_mode_t;

// Instruction opcodes
typedef enum {
    // Load/Store
    OP_LDI = 0x00,    // Load immediate
    OP_LDA = 0x01,    // Load from memory
    OP_STA = 0x02,    // Store to memory
    OP_MOV = 0x03,    // Move register to register
    
    // Arithmetic
    OP_ADD = 0x10,    // Add
    OP_SUB = 0x11,    // Subtract
    OP_ADC = 0x12,    // Add with carry
    OP_SBC = 0x13,    // Subtract with carry
    OP_CMP = 0x14,    // Compare
    OP_INC = 0x15,    // Increment
    OP_DEC = 0x16,    // Decrement
    
    // Logical
    OP_AND = 0x20,    // Logical AND
    OP_OR  = 0x21,    // Logical OR
    OP_XOR = 0x22,    // Logical XOR
    
    // Shift/Rotate
    OP_SHL = 0x30,    // Shift left
    OP_SHR = 0x31,    // Shift right
    OP_ROL = 0x32,    // Rotate left
    OP_ROR = 0x33,    // Rotate right
    
    // Jump/Call
    OP_JMP = 0x40,    // Jump absolute
    OP_JSR = 0x41,    // Jump to subroutine
    OP_RTS = 0x42,    // Return from subroutine
    
    // Branch
    OP_BEQ = 0x50,    // Branch if equal (Z=1)
    OP_BNE = 0x51,    // Branch if not equal (Z=0)
    OP_BCS = 0x52,    // Branch if carry set (C=1)
    OP_BCC = 0x53,    // Branch if carry clear (C=0)
    OP_BMI = 0x54,    // Branch if minus (N=1)
    OP_BPL = 0x55,    // Branch if plus (N=0)
    OP_BVS = 0x56,    // Branch if overflow set (V=1)
    OP_BVC = 0x57,    // Branch if overflow clear (V=0)
    
    // Stack
    OP_PHA = 0x60,    // Push accumulator
    OP_PLA = 0x61,    // Pull accumulator
    OP_PHP = 0x62,    // Push processor status
    OP_PLP = 0x63,    // Pull processor status
    OP_PUSH = 0x64,   // Push register
    OP_POP = 0x65,    // Pull register
    
    // System
    OP_SEI = 0x70,    // Set interrupt disable
    OP_CLI = 0x71,    // Clear interrupt disable
    OP_NOP = 0x72,    // No operation
    OP_HLT = 0x73     // Halt
} opcode_t;

// Instruction structure
typedef struct {
    opcode_t opcode;
    addressing_mode_t addr_mode;
    uint8_t operand1;
    uint8_t operand2;
    uint8_t cycles;
    const char* mnemonic;
} instruction_t;

// CPU state structure
typedef struct {
    // Registers
    uint8_t regs[8];      // A, B, C, D (8-bit), X, Y, SP, PC (16-bit)
    uint8_t flags;        // Status flags
    
    // Memory
    uint8_t* memory;
    
    // Control
    bool running;
    bool irq_pending;
    bool nmi_pending;
    uint64_t cycle_count;
    uint32_t instruction_count;
    
    // Debug
    bool trace_enabled;
    bool breakpoint_hit;
    uint16_t breakpoint_addr;
    uint16_t watch_addr;
    bool watch_hit;
    
    // Clock control
    uint32_t frequency_hz;
    uint64_t last_tick_time;
    uint32_t cycles_per_second;
} cpu_state_t;

// Function declarations
void isa_init(void);
const instruction_t* isa_get_instruction(opcode_t opcode);
const char* isa_get_mnemonic(opcode_t opcode);
uint8_t isa_get_cycles(opcode_t opcode);
bool isa_is_valid_opcode(uint8_t opcode);
void isa_print_instruction_table(void);

// Instruction execution
bool isa_execute_instruction(cpu_state_t* cpu);
uint8_t isa_fetch_byte(cpu_state_t* cpu);
uint16_t isa_fetch_word(cpu_state_t* cpu);
uint16_t isa_get_address(cpu_state_t* cpu, addressing_mode_t mode, uint8_t operand1, uint8_t operand2);
uint8_t isa_read_memory(cpu_state_t* cpu, uint16_t address);
void isa_write_memory(cpu_state_t* cpu, uint16_t address, uint8_t value);

// Flag operations
void isa_set_flag(cpu_state_t* cpu, uint8_t flag);
void isa_clear_flag(cpu_state_t* cpu, uint8_t flag);
bool isa_get_flag(cpu_state_t* cpu, uint8_t flag);
void isa_update_flags(cpu_state_t* cpu, uint8_t result, bool carry, bool overflow);

// Register operations
uint8_t isa_get_register(cpu_state_t* cpu, register_t reg);
void isa_set_register(cpu_state_t* cpu, register_t reg, uint8_t value);
uint16_t isa_get_register16(cpu_state_t* cpu, register_t reg);
void isa_set_register16(cpu_state_t* cpu, register_t reg, uint16_t value);

// Stack operations
void isa_push(cpu_state_t* cpu, uint8_t value);
uint8_t isa_pop(cpu_state_t* cpu);
void isa_push16(cpu_state_t* cpu, uint16_t value);
uint16_t isa_pop16(cpu_state_t* cpu);

#endif // ISA_H
