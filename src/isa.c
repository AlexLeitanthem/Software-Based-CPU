#include "isa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Instruction table with opcodes, cycles, and mnemonics
static const instruction_t instruction_table[] = {
    // Load/Store instructions
    {OP_LDI, ADDR_IMMEDIATE, 0, 0, 2, "LDI"},
    {OP_LDA, ADDR_ABSOLUTE, 0, 0, 3, "LDA"},
    {OP_LDA, ADDR_X_INDEXED, 0, 0, 4, "LDA"},
    {OP_LDA, ADDR_Y_INDEXED, 0, 0, 4, "LDA"},
    {OP_STA, ADDR_ABSOLUTE, 0, 0, 3, "STA"},
    {OP_STA, ADDR_X_INDEXED, 0, 0, 4, "STA"},
    {OP_STA, ADDR_Y_INDEXED, 0, 0, 4, "STA"},
    {OP_MOV, ADDR_REGISTER, 0, 0, 1, "MOV"},
    
    // Arithmetic instructions
    {OP_ADD, ADDR_IMMEDIATE, 0, 0, 2, "ADD"},
    {OP_ADD, ADDR_ABSOLUTE, 0, 0, 3, "ADD"},
    {OP_ADD, ADDR_REGISTER, 0, 0, 1, "ADD"},
    {OP_SUB, ADDR_IMMEDIATE, 0, 0, 2, "SUB"},
    {OP_SUB, ADDR_ABSOLUTE, 0, 0, 3, "SUB"},
    {OP_SUB, ADDR_REGISTER, 0, 0, 1, "SUB"},
    {OP_ADC, ADDR_IMMEDIATE, 0, 0, 2, "ADC"},
    {OP_ADC, ADDR_ABSOLUTE, 0, 0, 3, "ADC"},
    {OP_ADC, ADDR_REGISTER, 0, 0, 1, "ADC"},
    {OP_SBC, ADDR_IMMEDIATE, 0, 0, 2, "SBC"},
    {OP_SBC, ADDR_ABSOLUTE, 0, 0, 3, "SBC"},
    {OP_SBC, ADDR_REGISTER, 0, 0, 1, "SBC"},
    {OP_CMP, ADDR_IMMEDIATE, 0, 0, 2, "CMP"},
    {OP_CMP, ADDR_ABSOLUTE, 0, 0, 3, "CMP"},
    {OP_CMP, ADDR_REGISTER, 0, 0, 1, "CMP"},
    {OP_INC, ADDR_REGISTER, 0, 0, 1, "INC"},
    {OP_INC, ADDR_ABSOLUTE, 0, 0, 4, "INC"},
    {OP_DEC, ADDR_REGISTER, 0, 0, 1, "DEC"},
    {OP_DEC, ADDR_ABSOLUTE, 0, 0, 4, "DEC"},
    
    // Logical instructions
    {OP_AND, ADDR_IMMEDIATE, 0, 0, 2, "AND"},
    {OP_AND, ADDR_ABSOLUTE, 0, 0, 3, "AND"},
    {OP_AND, ADDR_REGISTER, 0, 0, 1, "AND"},
    {OP_OR, ADDR_IMMEDIATE, 0, 0, 2, "OR"},
    {OP_OR, ADDR_ABSOLUTE, 0, 0, 3, "OR"},
    {OP_OR, ADDR_REGISTER, 0, 0, 1, "OR"},
    {OP_XOR, ADDR_IMMEDIATE, 0, 0, 2, "XOR"},
    {OP_XOR, ADDR_ABSOLUTE, 0, 0, 3, "XOR"},
    {OP_XOR, ADDR_REGISTER, 0, 0, 1, "XOR"},
    
    // Shift/Rotate instructions
    {OP_SHL, ADDR_REGISTER, 0, 0, 1, "SHL"},
    {OP_SHL, ADDR_ABSOLUTE, 0, 0, 4, "SHL"},
    {OP_SHR, ADDR_REGISTER, 0, 0, 1, "SHR"},
    {OP_SHR, ADDR_ABSOLUTE, 0, 0, 4, "SHR"},
    {OP_ROL, ADDR_REGISTER, 0, 0, 1, "ROL"},
    {OP_ROL, ADDR_ABSOLUTE, 0, 0, 4, "ROL"},
    {OP_ROR, ADDR_REGISTER, 0, 0, 1, "ROR"},
    {OP_ROR, ADDR_ABSOLUTE, 0, 0, 4, "ROR"},
    
    // Jump/Call instructions
    {OP_JMP, ADDR_ABSOLUTE, 0, 0, 3, "JMP"},
    {OP_JSR, ADDR_ABSOLUTE, 0, 0, 6, "JSR"},
    {OP_RTS, ADDR_IMMEDIATE, 0, 0, 6, "RTS"},
    
    // Branch instructions
    {OP_BEQ, ADDR_RELATIVE, 0, 0, 2, "BEQ"},
    {OP_BNE, ADDR_RELATIVE, 0, 0, 2, "BNE"},
    {OP_BCS, ADDR_RELATIVE, 0, 0, 2, "BCS"},
    {OP_BCC, ADDR_RELATIVE, 0, 0, 2, "BCC"},
    {OP_BMI, ADDR_RELATIVE, 0, 0, 2, "BMI"},
    {OP_BPL, ADDR_RELATIVE, 0, 0, 2, "BPL"},
    {OP_BVS, ADDR_RELATIVE, 0, 0, 2, "BVS"},
    {OP_BVC, ADDR_RELATIVE, 0, 0, 2, "BVC"},
    
    // Stack instructions
    {OP_PHA, ADDR_IMMEDIATE, 0, 0, 3, "PHA"},
    {OP_PLA, ADDR_IMMEDIATE, 0, 0, 4, "PLA"},
    {OP_PHP, ADDR_IMMEDIATE, 0, 0, 3, "PHP"},
    {OP_PLP, ADDR_IMMEDIATE, 0, 0, 4, "PLP"},
    {OP_PUSH, ADDR_REGISTER, 0, 0, 3, "PUSH"},
    {OP_POP, ADDR_REGISTER, 0, 0, 4, "POP"},
    
    // System instructions
    {OP_SEI, ADDR_IMMEDIATE, 0, 0, 2, "SEI"},
    {OP_CLI, ADDR_IMMEDIATE, 0, 0, 2, "CLI"},
    {OP_NOP, ADDR_IMMEDIATE, 0, 0, 1, "NOP"},
    {OP_HLT, ADDR_IMMEDIATE, 0, 0, 1, "HLT"}
};

#define INSTRUCTION_COUNT (sizeof(instruction_table) / sizeof(instruction_table[0]))

void isa_init(void) {
    // Initialize instruction table if needed
}

const instruction_t* isa_get_instruction(opcode_t opcode) {
    for (size_t i = 0; i < INSTRUCTION_COUNT; i++) {
        if (instruction_table[i].opcode == opcode) {
            return &instruction_table[i];
        }
    }
    return NULL;
}

const char* isa_get_mnemonic(opcode_t opcode) {
    const instruction_t* inst = isa_get_instruction(opcode);
    return inst ? inst->mnemonic : "???";
}

uint8_t isa_get_cycles(opcode_t opcode) {
    const instruction_t* inst = isa_get_instruction(opcode);
    return inst ? inst->cycles : 0;
}

bool isa_is_valid_opcode(uint8_t opcode) {
    return isa_get_instruction((opcode_t)opcode) != NULL;
}

void isa_print_instruction_table(void) {
    printf("Instruction Set Architecture Reference\n");
    printf("=====================================\n\n");
    printf("%-6s %-8s %-12s %-8s %s\n", "Opcode", "Mnemonic", "Addressing", "Cycles", "Description");
    printf("%-6s %-8s %-12s %-8s %s\n", "------", "--------", "----------", "------", "-----------");
    
    for (size_t i = 0; i < INSTRUCTION_COUNT; i++) {
        const instruction_t* inst = &instruction_table[i];
        const char* addr_mode_str = "Unknown";
        
        switch (inst->addr_mode) {
            case ADDR_IMMEDIATE: addr_mode_str = "Immediate"; break;
            case ADDR_REGISTER: addr_mode_str = "Register"; break;
            case ADDR_ABSOLUTE: addr_mode_str = "Absolute"; break;
            case ADDR_X_INDEXED: addr_mode_str = "X-Indexed"; break;
            case ADDR_Y_INDEXED: addr_mode_str = "Y-Indexed"; break;
            case ADDR_SP_INDEXED: addr_mode_str = "SP-Indexed"; break;
            case ADDR_RELATIVE: addr_mode_str = "Relative"; break;
        }
        
        printf("0x%02X   %-8s %-12s %-8d %s\n", 
               inst->opcode, inst->mnemonic, addr_mode_str, inst->cycles, "Instruction");
    }
}

// Memory operations
uint8_t isa_fetch_byte(cpu_state_t* cpu) {
    uint16_t pc = isa_get_register16(cpu, REG_PC);
    uint8_t value = isa_read_memory(cpu, pc);
    isa_set_register16(cpu, REG_PC, pc + 1);
    return value;
}

uint16_t isa_fetch_word(cpu_state_t* cpu) {
    uint8_t low = isa_fetch_byte(cpu);
    uint8_t high = isa_fetch_byte(cpu);
    return low | (high << 8);
}

uint16_t isa_get_address(cpu_state_t* cpu, addressing_mode_t mode, uint8_t operand1, uint8_t operand2) {
    switch (mode) {
        case ADDR_IMMEDIATE:
            return 0; // Not used for immediate mode
        
        case ADDR_REGISTER:
            return operand1; // Register number
        
        case ADDR_ABSOLUTE:
            return operand1 | (operand2 << 8);
        
        case ADDR_X_INDEXED:
            return (operand1 | (operand2 << 8)) + isa_get_register16(cpu, REG_X);
        
        case ADDR_Y_INDEXED:
            return (operand1 | (operand2 << 8)) + isa_get_register16(cpu, REG_Y);
        
        case ADDR_SP_INDEXED:
            return isa_get_register16(cpu, REG_SP) + (int8_t)operand1;
        
        case ADDR_RELATIVE:
            return isa_get_register16(cpu, REG_PC) + (int8_t)operand1;
        
        default:
            return 0;
    }
}

uint8_t isa_read_memory(cpu_state_t* cpu, uint16_t address) {
    // address is a uint16_t and memory buffer covers full 16-bit space; direct access
    return cpu->memory[address];
}

void isa_write_memory(cpu_state_t* cpu, uint16_t address, uint8_t value) {
    // address is a uint16_t and memory buffer covers full 16-bit space; direct write
    cpu->memory[address] = value;
}

// Flag operations
void isa_set_flag(cpu_state_t* cpu, uint8_t flag) {
    cpu->flags |= flag;
}

void isa_clear_flag(cpu_state_t* cpu, uint8_t flag) {
    cpu->flags &= ~flag;
}

bool isa_get_flag(cpu_state_t* cpu, uint8_t flag) {
    return (cpu->flags & flag) != 0;
}

void isa_update_flags(cpu_state_t* cpu, uint8_t result, bool carry, bool overflow) {
    // Zero flag
    if (result == 0) {
        isa_set_flag(cpu, FLAG_ZERO);
    } else {
        isa_clear_flag(cpu, FLAG_ZERO);
    }
    
    // Negative flag
    if (result & 0x80) {
        isa_set_flag(cpu, FLAG_NEGATIVE);
    } else {
        isa_clear_flag(cpu, FLAG_NEGATIVE);
    }
    
    // Carry flag
    if (carry) {
        isa_set_flag(cpu, FLAG_CARRY);
    } else {
        isa_clear_flag(cpu, FLAG_CARRY);
    }
    
    // Overflow flag
    if (overflow) {
        isa_set_flag(cpu, FLAG_OVERFLOW);
    } else {
        isa_clear_flag(cpu, FLAG_OVERFLOW);
    }
}

// Register operations
uint8_t isa_get_register(cpu_state_t* cpu, register_t reg) {
    if (reg < 4) {
        return cpu->regs[reg]; // 8-bit registers
    } else if (reg == REG_FLAGS) {
        return cpu->flags;
    }
    return 0;
}

void isa_set_register(cpu_state_t* cpu, register_t reg, uint8_t value) {
    if (reg < 4) {
        cpu->regs[reg] = value; // 8-bit registers
    } else if (reg == REG_FLAGS) {
        cpu->flags = value;
    }
}

uint16_t isa_get_register16(cpu_state_t* cpu, register_t reg) {
    if (reg >= REG_X && reg <= REG_PC) {
        int index = reg - REG_X;
        return cpu->regs[index * 2] | (cpu->regs[index * 2 + 1] << 8);
    }
    return 0;
}

void isa_set_register16(cpu_state_t* cpu, register_t reg, uint16_t value) {
    if (reg >= REG_X && reg <= REG_PC) {
        int index = reg - REG_X;
        cpu->regs[index * 2] = value & 0xFF;
        cpu->regs[index * 2 + 1] = (value >> 8) & 0xFF;
    }
}

// Stack operations
void isa_push(cpu_state_t* cpu, uint8_t value) {
    uint16_t sp = isa_get_register16(cpu, REG_SP);
    isa_write_memory(cpu, sp, value);
    isa_set_register16(cpu, REG_SP, sp - 1);
}

uint8_t isa_pop(cpu_state_t* cpu) {
    uint16_t sp = isa_get_register16(cpu, REG_SP) + 1;
    isa_set_register16(cpu, REG_SP, sp);
    return isa_read_memory(cpu, sp);
}

void isa_push16(cpu_state_t* cpu, uint16_t value) {
    isa_push(cpu, (value >> 8) & 0xFF);
    isa_push(cpu, value & 0xFF);
}

uint16_t isa_pop16(cpu_state_t* cpu) {
    uint8_t low = isa_pop(cpu);
    uint8_t high = isa_pop(cpu);
    return low | (high << 8);
}

// Main instruction execution
bool isa_execute_instruction(cpu_state_t* cpu) {
    // Execute a single instruction regardless of the cpu->running flag.
    // This allows unit tests to call isa_execute_instruction/cpu_step
    // directly after a reset without enabling the run loop.
    
    // Fetch opcode
    uint8_t opcode = isa_fetch_byte(cpu);
    
    if (!isa_is_valid_opcode(opcode)) {
        printf("Invalid opcode: 0x%02X at PC=0x%04X\n", opcode, isa_get_register16(cpu, REG_PC) - 1);
        cpu->running = false;
        return false;
    }
    
    const instruction_t* inst = isa_get_instruction((opcode_t)opcode);
    if (!inst) {
        return false;
    }
    
    // Fetch operands based on addressing mode
    uint8_t operand1 = 0, operand2 = 0;
    
    switch (inst->addr_mode) {
        case ADDR_IMMEDIATE:
            operand1 = isa_fetch_byte(cpu);
            break;
        case ADDR_REGISTER:
            operand1 = isa_fetch_byte(cpu);
            break;
        case ADDR_ABSOLUTE:
            operand1 = isa_fetch_byte(cpu);
            operand2 = isa_fetch_byte(cpu);
            break;
        case ADDR_X_INDEXED:
        case ADDR_Y_INDEXED:
            operand1 = isa_fetch_byte(cpu);
            operand2 = isa_fetch_byte(cpu);
            break;
        case ADDR_SP_INDEXED:
            operand1 = isa_fetch_byte(cpu);
            break;
        case ADDR_RELATIVE:
            operand1 = isa_fetch_byte(cpu);
            break;
    }
    
    // Execute instruction
    bool result = true;
    
    switch (inst->opcode) {
        case OP_LDI:
            isa_set_register(cpu, REG_A, operand1);
            break;
            
        case OP_LDA: {
            uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
            uint8_t value = isa_read_memory(cpu, addr);
            isa_set_register(cpu, REG_A, value);
            break;
        }
        
        case OP_STA: {
            uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
            uint8_t value = isa_get_register(cpu, REG_A);
            isa_write_memory(cpu, addr, value);
            break;
        }
        
        case OP_MOV: {
            uint8_t src_reg = operand1;
            uint8_t value = isa_get_register(cpu, (register_t)src_reg);
            isa_set_register(cpu, REG_A, value);
            break;
        }
        
        case OP_ADD: {
            uint8_t value;
            if (inst->addr_mode == ADDR_IMMEDIATE) {
                value = operand1;
            } else {
                uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
                value = isa_read_memory(cpu, addr);
            }
            
            uint8_t a = isa_get_register(cpu, REG_A);
            uint16_t result = a + value;
            bool carry = (result > 0xFF);
            bool overflow = ((a ^ result) & (value ^ result) & 0x80) != 0;
            
            isa_set_register(cpu, REG_A, result & 0xFF);
            isa_update_flags(cpu, result & 0xFF, carry, overflow);
            break;
        }
        
        case OP_SUB: {
            uint8_t value;
            if (inst->addr_mode == ADDR_IMMEDIATE) {
                value = operand1;
            } else {
                uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
                value = isa_read_memory(cpu, addr);
            }
            
            uint8_t a = isa_get_register(cpu, REG_A);
            uint16_t result = a - value;
            // For subtraction, carry (borrow) occurs when a < value
            bool carry = (a < value);
            bool overflow = ((a ^ result) & (value ^ result) & 0x80) != 0;
            
            isa_set_register(cpu, REG_A, result & 0xFF);
            isa_update_flags(cpu, result & 0xFF, carry, overflow);
            break;
        }
        
        case OP_CMP: {
            uint8_t value;
            if (inst->addr_mode == ADDR_IMMEDIATE) {
                value = operand1;
            } else {
                uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
                value = isa_read_memory(cpu, addr);
            }
            
            uint8_t a = isa_get_register(cpu, REG_A);
            uint16_t result = a - value;
            // For comparison, set carry (borrow) based on unsigned comparison
            bool carry = (a < value);
            bool overflow = ((a ^ result) & (value ^ result) & 0x80) != 0;
            
            isa_update_flags(cpu, result & 0xFF, carry, overflow);
            break;
        }
        
        case OP_INC: {
            if (inst->addr_mode == ADDR_REGISTER) {
                uint8_t value = isa_get_register(cpu, (register_t)operand1);
                value++;
                isa_set_register(cpu, (register_t)operand1, value);
                isa_update_flags(cpu, value, false, false);
            } else {
                uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
                uint8_t value = isa_read_memory(cpu, addr);
                value++;
                isa_write_memory(cpu, addr, value);
                isa_update_flags(cpu, value, false, false);
            }
            break;
        }
        
        case OP_DEC: {
            if (inst->addr_mode == ADDR_REGISTER) {
                uint8_t value = isa_get_register(cpu, (register_t)operand1);
                value--;
                isa_set_register(cpu, (register_t)operand1, value);
                isa_update_flags(cpu, value, false, false);
            } else {
                uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
                uint8_t value = isa_read_memory(cpu, addr);
                value--;
                isa_write_memory(cpu, addr, value);
                isa_update_flags(cpu, value, false, false);
            }
            break;
        }
        
        case OP_AND: {
            uint8_t value;
            if (inst->addr_mode == ADDR_IMMEDIATE) {
                value = operand1;
            } else {
                uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
                value = isa_read_memory(cpu, addr);
            }
            
            uint8_t a = isa_get_register(cpu, REG_A);
            uint8_t result = a & value;
            isa_set_register(cpu, REG_A, result);
            isa_update_flags(cpu, result, false, false);
            break;
        }
        
        case OP_OR: {
            uint8_t value;
            if (inst->addr_mode == ADDR_IMMEDIATE) {
                value = operand1;
            } else {
                uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
                value = isa_read_memory(cpu, addr);
            }
            
            uint8_t a = isa_get_register(cpu, REG_A);
            uint8_t result = a | value;
            isa_set_register(cpu, REG_A, result);
            isa_update_flags(cpu, result, false, false);
            break;
        }
        
        case OP_XOR: {
            uint8_t value;
            if (inst->addr_mode == ADDR_IMMEDIATE) {
                value = operand1;
            } else {
                uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
                value = isa_read_memory(cpu, addr);
            }
            
            uint8_t a = isa_get_register(cpu, REG_A);
            uint8_t result = a ^ value;
            isa_set_register(cpu, REG_A, result);
            isa_update_flags(cpu, result, false, false);
            break;
        }
        
        case OP_JMP: {
            uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
            isa_set_register16(cpu, REG_PC, addr);
            break;
        }
        
        case OP_JSR: {
            uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
            uint16_t pc = isa_get_register16(cpu, REG_PC);
            isa_push16(cpu, pc);
            isa_set_register16(cpu, REG_PC, addr);
            break;
        }
        
        case OP_RTS: {
            uint16_t addr = isa_pop16(cpu);
            isa_set_register16(cpu, REG_PC, addr);
            break;
        }
        
        case OP_BEQ:
            if (isa_get_flag(cpu, FLAG_ZERO)) {
                uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
                isa_set_register16(cpu, REG_PC, addr);
            }
            break;
            
        case OP_BNE:
            if (!isa_get_flag(cpu, FLAG_ZERO)) {
                uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
                isa_set_register16(cpu, REG_PC, addr);
            }
            break;
            
        case OP_BCS:
            if (isa_get_flag(cpu, FLAG_CARRY)) {
                uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
                isa_set_register16(cpu, REG_PC, addr);
            }
            break;
            
        case OP_BCC:
            if (!isa_get_flag(cpu, FLAG_CARRY)) {
                uint16_t addr = isa_get_address(cpu, inst->addr_mode, operand1, operand2);
                isa_set_register16(cpu, REG_PC, addr);
            }
            break;
            
        case OP_PHA:
            isa_push(cpu, isa_get_register(cpu, REG_A));
            break;
            
        case OP_PLA:
            isa_set_register(cpu, REG_A, isa_pop(cpu));
            break;
            
        case OP_PHP:
            isa_push(cpu, cpu->flags);
            break;
            
        case OP_PLP:
            cpu->flags = isa_pop(cpu);
            break;
            
        case OP_PUSH:
            isa_push(cpu, isa_get_register(cpu, (register_t)operand1));
            break;
            
        case OP_POP:
            isa_set_register(cpu, (register_t)operand1, isa_pop(cpu));
            break;
            
        case OP_SEI:
            isa_set_flag(cpu, FLAG_INTERRUPT);
            break;
            
        case OP_CLI:
            isa_clear_flag(cpu, FLAG_INTERRUPT);
            break;
            
        case OP_NOP:
            // No operation
            break;
            
        case OP_HLT:
            cpu->running = false;
            break;
            
        default:
            printf("Unimplemented instruction: 0x%02X\n", opcode);
            result = false;
            break;
    }
    
    // Update cycle count
    cpu->cycle_count += inst->cycles;
    cpu->instruction_count++;
    
    return result;
}

