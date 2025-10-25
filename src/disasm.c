#include "isa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

// Disassembler state
typedef struct {
    uint8_t* memory;
    uint16_t start_address;
    uint16_t end_address;
    bool verbose;
    bool show_addresses;
    bool show_hex;
} disasm_state_t;

// Function prototypes
void print_usage(const char* program_name);
void print_help(void);
bool parse_cli_options(int argc, char* argv[], disasm_state_t* state);
void disassemble_memory(disasm_state_t* state);
void disassemble_instruction(disasm_state_t* state, uint16_t address, int* bytes_consumed);
const char* get_addressing_mode_string(addressing_mode_t mode);
const char* get_register_name(register_t reg);

int main(int argc, char* argv[]) {
    disasm_state_t state = {0};
    
    // Parse command line options
    if (!parse_cli_options(argc, argv, &state)) {
        return 1;
    }
    
    if (state.memory == NULL) {
        fprintf(stderr, "No memory data provided\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // Disassemble memory
    disassemble_memory(&state);
    
    return 0;
}

void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] [MEMORY_DATA]\n", program_name);
    printf("\nOptions:\n");
    printf("  -s, --start ADDRESS     Start address (default: 0x0200)\n");
    printf("  -e, --end ADDRESS       End address (default: 0x0300)\n");
    printf("  -v, --verbose          Verbose output\n");
    printf("  -a, --addresses        Show addresses\n");
    printf("  -x, --hex              Show hex dump\n");
    printf("  -h, --help             Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s -s 0x0200 -e 0x0300\n", program_name);
    printf("  %s -s 0x0200 -e 0x0300 -a -x\n", program_name);
    printf("  %s --help\n", program_name);
}

void print_help(void) {
    print_usage("disasm");
    printf("\nDisassembler Features:\n");
    printf("  - Disassembles all supported instructions\n");
    printf("  - Shows addressing modes\n");
    printf("  - Shows register names\n");
    printf("  - Shows hex dump\n");
    printf("  - Shows addresses\n");
    printf("\nSupported Instructions:\n");
    printf("  Load/Store: LDI, LDA, STA, MOV\n");
    printf("  Arithmetic: ADD, SUB, CMP, INC, DEC\n");
    printf("  Logical: AND, OR, XOR\n");
    printf("  Jump/Call: JMP, JSR, RTS\n");
    printf("  Branch: BEQ, BNE, BCS, BCC, BMI, BPL, BVS, BVC\n");
    printf("  Stack: PHA, PLA, PHP, PLP, PUSH, POP\n");
    printf("  System: SEI, CLI, NOP, HLT\n");
}

bool parse_cli_options(int argc, char* argv[], disasm_state_t* state) {
    static struct option long_options[] = {
        {"start", required_argument, 0, 's'},
        {"end", required_argument, 0, 'e'},
        {"verbose", no_argument, 0, 'v'},
        {"addresses", no_argument, 0, 'a'},
        {"hex", no_argument, 0, 'x'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    // Set defaults
    state->memory = NULL;
    state->start_address = 0x0200;
    state->end_address = 0x0300;
    state->verbose = false;
    state->show_addresses = false;
    state->show_hex = false;
    
    while ((c = getopt_long(argc, argv, "s:e:vaxh", long_options, &option_index)) != -1) {
        switch (c) {
            case 's':
                state->start_address = strtol(optarg, NULL, 0);
                break;
            case 'e':
                state->end_address = strtol(optarg, NULL, 0);
                break;
            case 'v':
                state->verbose = true;
                break;
            case 'a':
                state->show_addresses = true;
                break;
            case 'x':
                state->show_hex = true;
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
    
    // For now, create a simple memory buffer
    // In a real implementation, this would load from a file
    state->memory = malloc(65536);
    if (state->memory) {
        memset(state->memory, 0, 65536);
        
        // Add some sample instructions for demonstration
        state->memory[0x0200] = 0x00; // LDI
        state->memory[0x0201] = 0x42; // Immediate value
        state->memory[0x0202] = 0x01; // LDA
        state->memory[0x0203] = 0x00; // Low address
        state->memory[0x0204] = 0x03; // High address
        state->memory[0x0205] = 0x40; // JMP
        state->memory[0x0206] = 0x00; // Low address
        state->memory[0x0207] = 0x02; // High address
        state->memory[0x0208] = 0x73; // HLT
    }
    
    return true;
}

void disassemble_memory(disasm_state_t* state) {
    printf("Disassembly from 0x%04X to 0x%04X:\n", 
           state->start_address, state->end_address);
    printf("=====================================\n\n");
    
    uint16_t address = state->start_address;
    
    while (address <= state->end_address) {
        int bytes_consumed = 0;
        
        if (state->show_addresses) {
            printf("0x%04X: ", address);
        }
        
        if (state->show_hex) {
            printf("%02X ", state->memory[address]);
        }
        
        disassemble_instruction(state, address, &bytes_consumed);
        
        if (bytes_consumed == 0) {
            // Unknown instruction, skip one byte
            printf("???");
            bytes_consumed = 1;
        }
        
        printf("\n");
        
        address += bytes_consumed;
    }
}

void disassemble_instruction(disasm_state_t* state, uint16_t address, int* bytes_consumed) {
    uint8_t opcode = state->memory[address];
    *bytes_consumed = 1;
    
    // Get instruction info
    const instruction_t* inst = isa_get_instruction((opcode_t)opcode);
    if (!inst) {
        printf("???");
        return;
    }
    
    printf("%s", inst->mnemonic);
    
    // Parse operands based on instruction
    switch (inst->opcode) {
        case OP_LDI:
            if (address + 1 <= state->end_address) {
                uint8_t value = state->memory[address + 1];
                printf(" #$%02X", value);
                *bytes_consumed = 2;
            }
            break;
            
        case OP_LDA:
        case OP_STA:
            if (address + 2 <= state->end_address) {
                uint16_t addr = state->memory[address + 1] | 
                               (state->memory[address + 2] << 8);
                printf(" [$%04X]", addr);
                *bytes_consumed = 3;
            }
            break;
            
        case OP_MOV:
            if (address + 1 <= state->end_address) {
                uint8_t reg = state->memory[address + 1];
                printf(" %s", get_register_name((register_t)reg));
                *bytes_consumed = 2;
            }
            break;
            
        case OP_ADD:
        case OP_SUB:
        case OP_CMP:
            if (address + 1 <= state->end_address) {
                uint8_t operand = state->memory[address + 1];
                printf(" #$%02X", operand);
                *bytes_consumed = 2;
            }
            break;
            
        case OP_JMP:
        case OP_JSR:
            if (address + 2 <= state->end_address) {
                uint16_t addr = state->memory[address + 1] | 
                               (state->memory[address + 2] << 8);
                printf(" $%04X", addr);
                *bytes_consumed = 3;
            }
            break;
            
        case OP_BEQ:
        case OP_BNE:
        case OP_BCS:
        case OP_BCC:
        case OP_BMI:
        case OP_BPL:
        case OP_BVS:
        case OP_BVC:
            if (address + 1 <= state->end_address) {
                int8_t offset = (int8_t)state->memory[address + 1];
                uint16_t target = address + 2 + offset;
                printf(" $%04X", target);
                *bytes_consumed = 2;
            }
            break;
            
        case OP_PUSH:
        case OP_POP:
            if (address + 1 <= state->end_address) {
                uint8_t reg = state->memory[address + 1];
                printf(" %s", get_register_name((register_t)reg));
                *bytes_consumed = 2;
            }
            break;
            
        case OP_NOP:
        case OP_HLT:
        case OP_RTS:
        case OP_PHA:
        case OP_PLA:
        case OP_PHP:
        case OP_PLP:
        case OP_SEI:
        case OP_CLI:
            // No operands
            break;
            
        default:
            printf("???");
            break;
    }
}

const char* get_addressing_mode_string(addressing_mode_t mode) {
    switch (mode) {
        case ADDR_IMMEDIATE: return "immediate";
        case ADDR_REGISTER: return "register";
        case ADDR_ABSOLUTE: return "absolute";
        case ADDR_X_INDEXED: return "x-indexed";
        case ADDR_Y_INDEXED: return "y-indexed";
        case ADDR_SP_INDEXED: return "sp-indexed";
        case ADDR_RELATIVE: return "relative";
        default: return "unknown";
    }
}

const char* get_register_name(register_t reg) {
    switch (reg) {
        case REG_A: return "A";
        case REG_B: return "B";
        case REG_C: return "C";
        case REG_D: return "D";
        case REG_X: return "X";
        case REG_Y: return "Y";
        case REG_SP: return "SP";
        case REG_PC: return "PC";
        default: return "?";
    }
}

