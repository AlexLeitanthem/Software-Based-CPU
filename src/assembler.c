#include "assembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

// Create assembler instance
assembler_t* assembler_create(void) {
    assembler_t* assembler = malloc(sizeof(assembler_t));
    if (!assembler) {
        return NULL;
    }
    
    // Initialize state
    memset(assembler, 0, sizeof(assembler_t));
    assembler->output = malloc(65536); // 64KB output buffer
    if (!assembler->output) {
        free(assembler);
        return NULL;
    }
    
    assembler->output_size = 0;
    assembler->current_address = 0;
    assembler->origin_address = 0;
    assembler->label_count = 0;
    assembler->symbol_count = 0;
    assembler->include_count = 0;
    assembler->error_occurred = false;
    
    return assembler;
}

// Destroy assembler instance
void assembler_destroy(assembler_t* assembler) {
    if (assembler) {
        if (assembler->output) {
            free(assembler->output);
        }
        if (assembler->file) {
            fclose(assembler->file);
        }
        free(assembler);
    }
}

// Assemble file
bool assembler_assemble_file(assembler_t* assembler, const char* filename) {
    assembler->filename = (char*)filename;
    assembler->file = fopen(filename, "r");
    if (!assembler->file) {
        assembler_error(assembler, "Cannot open file: %s", filename);
        return false;
    }
    
    assembler->line_number = 0;
    assembler->column_number = 0;
    
    // First pass: collect labels
    while (!feof(assembler->file)) {
        if (!assembler_parse_line(assembler)) {
            break;
        }
    }
    
    // Reset file for second pass
    fseek(assembler->file, 0, SEEK_SET);
    assembler->line_number = 0;
    assembler->column_number = 0;
    assembler->current_address = assembler->origin_address;
    
    // Second pass: generate code
    while (!feof(assembler->file)) {
        if (!assembler_parse_line(assembler)) {
            break;
        }
    }
    
    // Resolve labels
    if (!assembler_resolve_labels(assembler)) {
        return false;
    }
    
    fclose(assembler->file);
    assembler->file = NULL;
    
    return !assembler->error_occurred;
}

// Parse a single line
bool assembler_parse_line(assembler_t* assembler) {
    char line[MAX_LINE_LENGTH];
    if (!fgets(line, sizeof(line), assembler->file)) {
        return false;
    }
    
    assembler->line_number++;
    assembler->current_line = line;
    assembler->column_number = 0;
    
    // Skip whitespace
    assembler_skip_whitespace(assembler);
    
    // Skip empty lines
    if (assembler->current_line[assembler->column_number] == '\n' || 
        assembler->current_line[assembler->column_number] == '\0') {
        return true;
    }
    
    // Skip comments
    if (assembler->current_line[assembler->column_number] == ';') {
        return true;
    }
    
    // Parse label
    if (assembler_parse_label(assembler)) {
        return true;
    }
    
    // Parse directive
    if (assembler->current_line[assembler->column_number] == '.') {
        return assembler_parse_directive(assembler);
    }
    
    // Parse instruction
    return assembler_parse_instruction(assembler);
}

// Parse label
bool assembler_parse_label(assembler_t* assembler) {
    char label_name[MAX_LABEL_LENGTH];
    int i = 0;
    
    // Check if this is a label
    while (assembler_is_identifier_char(assembler->current_line[assembler->column_number])) {
        if (i >= MAX_LABEL_LENGTH - 1) {
            assembler_error(assembler, "Label name too long");
            return false;
        }
        label_name[i++] = assembler->current_line[assembler->column_number++];
    }
    
    if (i == 0) {
        return false; // Not a label
    }
    
    label_name[i] = '\0';
    
    // Check for colon
    assembler_skip_whitespace(assembler);
    if (assembler->current_line[assembler->column_number] == ':') {
        assembler->column_number++;
        assembler_add_label(assembler, label_name, assembler->current_address);
        return true;
    }
    
    // Not a label, reset position
    assembler->column_number -= i;
    return false;
}

// Parse directive
bool assembler_parse_directive(assembler_t* assembler) {
    assembler->column_number++; // Skip '.'
    
    char directive[32];
    int i = 0;
    while (assembler_is_identifier_char(assembler->current_line[assembler->column_number])) {
        if (i >= 31) {
            assembler_error(assembler, "Directive name too long");
            return false;
        }
        directive[i++] = assembler->current_line[assembler->column_number++];
    }
    directive[i] = '\0';
    
    assembler_skip_whitespace(assembler);
    
    if (strcmp(directive, "org") == 0) {
        uint16_t address = assembler_parse_expression(assembler);
        assembler->current_address = address;
        assembler->origin_address = address;
        return true;
    } else if (strcmp(directive, "byte") == 0) {
        uint16_t value = assembler_parse_expression(assembler);
        assembler_emit_byte(assembler, value & 0xFF);
        return true;
    } else if (strcmp(directive, "word") == 0) {
        uint16_t value = assembler_parse_expression(assembler);
        assembler_emit_word(assembler, value);
        return true;
    } else if (strcmp(directive, "string") == 0) {
        assembler_skip_whitespace(assembler);
        if (assembler->current_line[assembler->column_number] == '"') {
            assembler->column_number++;
            while (assembler->current_line[assembler->column_number] != '"' && 
                   assembler->current_line[assembler->column_number] != '\n') {
                assembler_emit_byte(assembler, assembler->current_line[assembler->column_number++]);
            }
            if (assembler->current_line[assembler->column_number] == '"') {
                assembler->column_number++;
            }
        }
        return true;
    } else if (strcmp(directive, "include") == 0) {
        assembler_skip_whitespace(assembler);
        if (assembler->current_line[assembler->column_number] == '"') {
            assembler->column_number++;
            char filename[256];
            int i = 0;
            while (assembler->current_line[assembler->column_number] != '"' && 
                   assembler->current_line[assembler->column_number] != '\n' && i < 255) {
                filename[i++] = assembler->current_line[assembler->column_number++];
            }
            filename[i] = '\0';
            if (assembler->current_line[assembler->column_number] == '"') {
                assembler->column_number++;
            }
            
            // TODO: Handle include files
            assembler_warning(assembler, "Include files not yet implemented");
        }
        return true;
    } else {
        assembler_error(assembler, "Unknown directive: .%s", directive);
        return false;
    }
}

// Parse instruction
bool assembler_parse_instruction(assembler_t* assembler) {
    char instruction[32];
    int i = 0;
    
    // Get instruction name
    while (assembler_is_identifier_char(assembler->current_line[assembler->column_number])) {
        if (i >= 31) {
            assembler_error(assembler, "Instruction name too long");
            return false;
        }
        instruction[i++] = assembler->current_line[assembler->column_number++];
    }
    instruction[i] = '\0';
    
    if (i == 0) {
        assembler_error(assembler, "Expected instruction");
        return false;
    }
    
    // Get opcode
    opcode_t opcode = assembler_get_opcode(instruction);
    if (opcode == 0 && strcmp(instruction, "NOP") != 0) {
        assembler_error(assembler, "Unknown instruction: %s", instruction);
        return false;
    }
    
    assembler_skip_whitespace(assembler);
    
    // Emit opcode
    assembler_emit_byte(assembler, opcode);
    
    // Parse operands based on instruction
    if (strcmp(instruction, "LDI") == 0) {
        uint16_t value = assembler_parse_expression(assembler);
        assembler_emit_byte(assembler, value & 0xFF);
    } else if (strcmp(instruction, "LDA") == 0 || strcmp(instruction, "STA") == 0) {
        // Parse addressing mode
        if (assembler->current_line[assembler->column_number] == '#') {
            // Immediate mode
            assembler->column_number++;
            uint16_t value = assembler_parse_expression(assembler);
            assembler_emit_byte(assembler, value & 0xFF);
        } else if (assembler->current_line[assembler->column_number] == '[') {
            // Absolute addressing
            assembler->column_number++;
            uint16_t address = assembler_parse_expression(assembler);
            if (assembler->current_line[assembler->column_number] == ']') {
                assembler->column_number++;
                assembler_emit_byte(assembler, address & 0xFF);
                assembler_emit_byte(assembler, (address >> 8) & 0xFF);
            } else {
                assembler_error(assembler, "Expected ']'");
                return false;
            }
        } else {
            assembler_error(assembler, "Invalid addressing mode");
            return false;
        }
    } else if (strcmp(instruction, "MOV") == 0) {
        // Parse register
        char reg_name[8];
        int i = 0;
        while (assembler_is_identifier_char(assembler->current_line[assembler->column_number])) {
            if (i >= 7) {
                assembler_error(assembler, "Register name too long");
                return false;
            }
            reg_name[i++] = assembler->current_line[assembler->column_number++];
        }
        reg_name[i] = '\0';
        
        register_t reg = assembler_get_register(reg_name);
        if (reg == 0 && strcmp(reg_name, "A") != 0) {
            assembler_error(assembler, "Unknown register: %s", reg_name);
            return false;
        }
        
        assembler_emit_byte(assembler, reg);
    } else if (strcmp(instruction, "JMP") == 0 || strcmp(instruction, "JSR") == 0) {
        uint16_t address = assembler_parse_expression(assembler);
        assembler_emit_byte(assembler, address & 0xFF);
        assembler_emit_byte(assembler, (address >> 8) & 0xFF);
    } else if (strcmp(instruction, "BEQ") == 0 || strcmp(instruction, "BNE") == 0 ||
               strcmp(instruction, "BCS") == 0 || strcmp(instruction, "BCC") == 0 ||
               strcmp(instruction, "BMI") == 0 || strcmp(instruction, "BPL") == 0 ||
               strcmp(instruction, "BVS") == 0 || strcmp(instruction, "BVC") == 0) {
        uint16_t address = assembler_parse_expression(assembler);
        int16_t offset = address - (assembler->current_address + 1);
        if (offset < -128 || offset > 127) {
            assembler_error(assembler, "Branch offset out of range: %d", offset);
            return false;
        }
        assembler_emit_byte(assembler, offset & 0xFF);
    }
    
    return true;
}

// Parse expression
uint16_t assembler_parse_expression(assembler_t* assembler) {
    return assembler_parse_term(assembler);
}

// Parse term
uint16_t assembler_parse_term(assembler_t* assembler) {
    uint16_t left = assembler_parse_factor(assembler);
    
    assembler_skip_whitespace(assembler);
    
    while (assembler->current_line[assembler->column_number] == '+' || 
           assembler->current_line[assembler->column_number] == '-') {
        char op = assembler->current_line[assembler->column_number++];
        uint16_t right = assembler_parse_factor(assembler);
        
        if (op == '+') {
            left += right;
        } else {
            left -= right;
        }
        
        assembler_skip_whitespace(assembler);
    }
    
    return left;
}

// Parse factor
uint16_t assembler_parse_factor(assembler_t* assembler) {
    assembler_skip_whitespace(assembler);
    
    if (assembler->current_line[assembler->column_number] == '(') {
        assembler->column_number++;
        uint16_t value = assembler_parse_expression(assembler);
        if (assembler->current_line[assembler->column_number] == ')') {
            assembler->column_number++;
        }
        return value;
    }
    
    if (assembler->current_line[assembler->column_number] == '#') {
        assembler->column_number++;
        return assembler_parse_number_literal(assembler);
    }
    
    if (isdigit(assembler->current_line[assembler->column_number]) || 
        assembler->current_line[assembler->column_number] == '$' ||
        assembler->current_line[assembler->column_number] == '%') {
        return assembler_parse_number_literal(assembler);
    }
    
    if (assembler_is_identifier_char(assembler->current_line[assembler->column_number])) {
        return assembler_parse_identifier(assembler);
    }
    
    assembler_error(assembler, "Expected number or identifier");
    return 0;
}

// Parse number literal
uint16_t assembler_parse_number_literal(assembler_t* assembler) {
    char number_str[32];
    int i = 0;
    
    if (assembler->current_line[assembler->column_number] == '$') {
        // Hexadecimal
        assembler->column_number++;
        while (isxdigit(assembler->current_line[assembler->column_number])) {
            if (i >= 31) {
                assembler_error(assembler, "Number too long");
                return 0;
            }
            number_str[i++] = assembler->current_line[assembler->column_number++];
        }
        number_str[i] = '\0';
        return strtol(number_str, NULL, 16);
    } else if (assembler->current_line[assembler->column_number] == '%') {
        // Binary
        assembler->column_number++;
        while (assembler->current_line[assembler->column_number] == '0' || 
               assembler->current_line[assembler->column_number] == '1') {
            if (i >= 31) {
                assembler_error(assembler, "Number too long");
                return 0;
            }
            number_str[i++] = assembler->current_line[assembler->column_number++];
        }
        number_str[i] = '\0';
        return strtol(number_str, NULL, 2);
    } else {
        // Decimal
        while (isdigit(assembler->current_line[assembler->column_number])) {
            if (i >= 31) {
                assembler_error(assembler, "Number too long");
                return 0;
            }
            number_str[i++] = assembler->current_line[assembler->column_number++];
        }
        number_str[i] = '\0';
        return strtol(number_str, NULL, 10);
    }
}

// Parse identifier
uint16_t assembler_parse_identifier(assembler_t* assembler) {
    char identifier[MAX_LABEL_LENGTH];
    int i = 0;
    
    while (assembler_is_identifier_char(assembler->current_line[assembler->column_number])) {
        if (i >= MAX_LABEL_LENGTH - 1) {
            assembler_error(assembler, "Identifier too long");
            return 0;
        }
        identifier[i++] = assembler->current_line[assembler->column_number++];
    }
    identifier[i] = '\0';
    
    // Check if it's a label
    label_t* label = assembler_find_label(assembler, identifier);
    if (label) {
        return label->address;
    }
    
    // Check if it's a symbol
    symbol_t* symbol = assembler_find_symbol(assembler, identifier);
    if (symbol) {
        return symbol->value;
    }
    
    assembler_error(assembler, "Undefined label or symbol: %s", identifier);
    return 0;
}

// Add label
bool assembler_add_label(assembler_t* assembler, const char* name, uint16_t address) {
    if (assembler->label_count >= MAX_LABELS) {
        assembler_error(assembler, "Too many labels");
        return false;
    }
    
    label_t* label = &assembler->labels[assembler->label_count++];
    strncpy(label->name, name, MAX_LABEL_LENGTH - 1);
    label->name[MAX_LABEL_LENGTH - 1] = '\0';
    label->address = address;
    label->defined = true;
    label->line = assembler->line_number;
    
    return true;
}

// Find label
label_t* assembler_find_label(assembler_t* assembler, const char* name) {
    for (int i = 0; i < assembler->label_count; i++) {
        if (strcmp(assembler->labels[i].name, name) == 0) {
            return &assembler->labels[i];
        }
    }
    return NULL;
}

// Find symbol
symbol_t* assembler_find_symbol(assembler_t* assembler, const char* name) {
    for (int i = 0; i < assembler->symbol_count; i++) {
        if (strcmp(assembler->symbols[i].name, name) == 0) {
            return &assembler->symbols[i];
        }
    }
    return NULL;
}

// Resolve labels
bool assembler_resolve_labels(assembler_t* assembler) {
    // TODO: Implement label resolution
    return true;
}

// Emit byte
void assembler_emit_byte(assembler_t* assembler, uint8_t value) {
    if (assembler->output_size < 65536) {
        assembler->output[assembler->output_size++] = value;
        assembler->current_address++;
    }
}

// Emit word
void assembler_emit_word(assembler_t* assembler, uint16_t value) {
    assembler_emit_byte(assembler, value & 0xFF);
    assembler_emit_byte(assembler, (value >> 8) & 0xFF);
}

// Emit string
void assembler_emit_string(assembler_t* assembler, const char* str) {
    while (*str) {
        assembler_emit_byte(assembler, *str++);
    }
}

// Skip whitespace
void assembler_skip_whitespace(assembler_t* assembler) {
    while (assembler->current_line[assembler->column_number] == ' ' || 
           assembler->current_line[assembler->column_number] == '\t') {
        assembler->column_number++;
    }
}

// Check if character is identifier character
bool assembler_is_identifier_char(char c) {
    return isalnum(c) || c == '_';
}

// Check if character is number character
bool assembler_is_number_char(char c) {
    return isdigit(c) || c == '$' || c == '%';
}

// Parse number
uint16_t assembler_parse_number(const char* str) {
    if (str[0] == '$') {
        return strtol(str + 1, NULL, 16);
    } else if (str[0] == '%') {
        return strtol(str + 1, NULL, 2);
    } else {
        return strtol(str, NULL, 10);
    }
}

// Check if string is register name
bool assembler_is_register_name(const char* name) {
    return strcmp(name, "A") == 0 || strcmp(name, "B") == 0 || 
           strcmp(name, "C") == 0 || strcmp(name, "D") == 0 ||
           strcmp(name, "X") == 0 || strcmp(name, "Y") == 0 ||
           strcmp(name, "SP") == 0 || strcmp(name, "PC") == 0;
}

// Get register number
register_t assembler_get_register(const char* name) {
    if (strcmp(name, "A") == 0) return REG_A;
    if (strcmp(name, "B") == 0) return REG_B;
    if (strcmp(name, "C") == 0) return REG_C;
    if (strcmp(name, "D") == 0) return REG_D;
    if (strcmp(name, "X") == 0) return REG_X;
    if (strcmp(name, "Y") == 0) return REG_Y;
    if (strcmp(name, "SP") == 0) return REG_SP;
    if (strcmp(name, "PC") == 0) return REG_PC;
    return 0;
}

// Check if string is instruction name
bool assembler_is_instruction_name(const char* name) {
    return strcmp(name, "LDI") == 0 || strcmp(name, "LDA") == 0 || 
           strcmp(name, "STA") == 0 || strcmp(name, "MOV") == 0 ||
           strcmp(name, "ADD") == 0 || strcmp(name, "SUB") == 0 ||
           strcmp(name, "JMP") == 0 || strcmp(name, "JSR") == 0 ||
           strcmp(name, "RTS") == 0 || strcmp(name, "BEQ") == 0 ||
           strcmp(name, "BNE") == 0 || strcmp(name, "NOP") == 0 ||
           strcmp(name, "HLT") == 0;
}

// Get opcode
opcode_t assembler_get_opcode(const char* name) {
    if (strcmp(name, "LDI") == 0) return OP_LDI;
    if (strcmp(name, "LDA") == 0) return OP_LDA;
    if (strcmp(name, "STA") == 0) return OP_STA;
    if (strcmp(name, "MOV") == 0) return OP_MOV;
    if (strcmp(name, "ADD") == 0) return OP_ADD;
    if (strcmp(name, "SUB") == 0) return OP_SUB;
    if (strcmp(name, "JMP") == 0) return OP_JMP;
    if (strcmp(name, "JSR") == 0) return OP_JSR;
    if (strcmp(name, "RTS") == 0) return OP_RTS;
    if (strcmp(name, "BEQ") == 0) return OP_BEQ;
    if (strcmp(name, "BNE") == 0) return OP_BNE;
    if (strcmp(name, "NOP") == 0) return OP_NOP;
    if (strcmp(name, "HLT") == 0) return OP_HLT;
    return 0;
}

// Get addressing mode
addressing_mode_t assembler_get_addressing_mode(const char* name) {
    if (strcmp(name, "immediate") == 0) return ADDR_IMMEDIATE;
    if (strcmp(name, "register") == 0) return ADDR_REGISTER;
    if (strcmp(name, "absolute") == 0) return ADDR_ABSOLUTE;
    if (strcmp(name, "x_indexed") == 0) return ADDR_X_INDEXED;
    if (strcmp(name, "y_indexed") == 0) return ADDR_Y_INDEXED;
    if (strcmp(name, "sp_indexed") == 0) return ADDR_SP_INDEXED;
    if (strcmp(name, "relative") == 0) return ADDR_RELATIVE;
    return ADDR_IMMEDIATE;
}

// Error handling
void assembler_error(assembler_t* assembler, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(assembler->error_message, sizeof(assembler->error_message), format, args);
    va_end(args);
    
    printf("Error in %s:%d: %s\n", assembler->filename, assembler->line_number, assembler->error_message);
    assembler->error_occurred = true;
}

// Warning
void assembler_warning(assembler_t* assembler, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char message[256];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    printf("Warning in %s:%d: %s\n", assembler->filename, assembler->line_number, message);
}

// Save binary
bool assembler_save_binary(assembler_t* assembler, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        return false;
    }
    
    size_t written = fwrite(assembler->output, 1, assembler->output_size, file);
    fclose(file);
    
    return written == assembler->output_size;
}

// Save listing
bool assembler_save_listing(assembler_t* assembler, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        return false;
    }
    
    // TODO: Generate listing file
    fprintf(file, "Assembler listing\n");
    fprintf(file, "================\n\n");
    
    fclose(file);
    return true;
}
