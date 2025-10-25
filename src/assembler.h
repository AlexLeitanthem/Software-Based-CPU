#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "isa.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Assembler configuration
#define MAX_LABELS 1000
#define MAX_LABEL_LENGTH 64
#define MAX_LINE_LENGTH 256
#define MAX_INCLUDES 10

// Token types
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_COMMA,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_HASH,
    TOKEN_DOT,
    TOKEN_NEWLINE,
    TOKEN_UNKNOWN
} token_type_t;

// Token structure
typedef struct {
    token_type_t type;
    char* text;
    uint16_t value;
    int line;
    int column;
} token_t;

// Label structure
typedef struct {
    char name[MAX_LABEL_LENGTH];
    uint16_t address;
    bool defined;
    int line;
} label_t;

// Symbol structure
typedef struct {
    char name[MAX_LABEL_LENGTH];
    uint16_t value;
    bool defined;
} symbol_t;

// Assembler state
typedef struct {
    // Input
    char* filename;
    FILE* file;
    char* current_line;
    int line_number;
    int column_number;
    
    // Output
    uint8_t* output;
    uint16_t output_size;
    uint16_t current_address;
    uint16_t origin_address;
    
    // Labels and symbols
    label_t labels[MAX_LABELS];
    symbol_t symbols[MAX_LABELS];
    int label_count;
    int symbol_count;
    
    // Current token
    token_t current_token;
    
    // Error handling
    bool error_occurred;
    char error_message[256];
    
    // Include files
    char* includes[MAX_INCLUDES];
    int include_count;
} assembler_t;

// Assembler functions
assembler_t* assembler_create(void);
void assembler_destroy(assembler_t* assembler);
bool assembler_assemble_file(assembler_t* assembler, const char* filename);
bool assembler_assemble_string(assembler_t* assembler, const char* source);
bool assembler_save_binary(assembler_t* assembler, const char* filename);
bool assembler_save_listing(assembler_t* assembler, const char* filename);

// Token functions
token_t assembler_next_token(assembler_t* assembler);
void assembler_skip_whitespace(assembler_t* assembler);
bool assembler_expect_token(assembler_t* assembler, token_type_t expected);
bool assembler_peek_token(assembler_t* assembler, token_type_t expected);

// Label functions
bool assembler_add_label(assembler_t* assembler, const char* name, uint16_t address);
label_t* assembler_find_label(assembler_t* assembler, const char* name);
bool assembler_resolve_labels(assembler_t* assembler);

// Symbol functions
bool assembler_add_symbol(assembler_t* assembler, const char* name, uint16_t value);
symbol_t* assembler_find_symbol(assembler_t* assembler, const char* name);

// Expression parsing
uint16_t assembler_parse_expression(assembler_t* assembler);
uint16_t assembler_parse_term(assembler_t* assembler);
uint16_t assembler_parse_factor(assembler_t* assembler);

// Instruction parsing
bool assembler_parse_instruction(assembler_t* assembler);
bool assembler_parse_directive(assembler_t* assembler);
bool assembler_parse_label(assembler_t* assembler);

// Output functions
void assembler_emit_byte(assembler_t* assembler, uint8_t value);
void assembler_emit_word(assembler_t* assembler, uint16_t value);
void assembler_emit_string(assembler_t* assembler, const char* str);

// Error handling
void assembler_error(assembler_t* assembler, const char* format, ...);
void assembler_warning(assembler_t* assembler, const char* format, ...);

// Utility functions
bool assembler_is_identifier_char(char c);
bool assembler_is_number_char(char c);
uint16_t assembler_parse_number(const char* str);
bool assembler_is_register_name(const char* name);
register_t assembler_get_register(const char* name);
bool assembler_is_instruction_name(const char* name);
opcode_t assembler_get_opcode(const char* name);
addressing_mode_t assembler_get_addressing_mode(const char* name);

// Missing function declarations
bool assembler_parse_line(assembler_t* assembler);
uint16_t assembler_parse_number_literal(assembler_t* assembler);
uint16_t assembler_parse_identifier(assembler_t* assembler);

#endif // ASSEMBLER_H
