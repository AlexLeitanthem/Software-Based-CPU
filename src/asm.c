#include "assembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

// Command line options
typedef struct {
    char* input_file;
    char* output_file;
    char* listing_file;
    bool verbose;
    bool help_requested;
} cli_options_t;

// Function prototypes
void print_usage(const char* program_name);
void print_help(void);
bool parse_cli_options(int argc, char* argv[], cli_options_t* options);
void print_assembler_info(assembler_t* assembler);

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
    
    if (!options.input_file) {
        fprintf(stderr, "No input file specified\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // Create assembler instance
    assembler_t* assembler = assembler_create();
    if (!assembler) {
        fprintf(stderr, "Failed to create assembler instance\n");
        return 1;
    }
    
    // Assemble file
    if (!assembler_assemble_file(assembler, options.input_file)) {
        fprintf(stderr, "Assembly failed\n");
        assembler_destroy(assembler);
        return 1;
    }
    
    // Print assembler info
    if (options.verbose) {
        print_assembler_info(assembler);
    }
    
    // Save binary output
    if (options.output_file) {
        if (!assembler_save_binary(assembler, options.output_file)) {
            fprintf(stderr, "Failed to save binary output\n");
            assembler_destroy(assembler);
            return 1;
        }
        printf("Binary output saved to %s\n", options.output_file);
    }
    
    // Save listing file
    if (options.listing_file) {
        if (!assembler_save_listing(assembler, options.listing_file)) {
            fprintf(stderr, "Failed to save listing file\n");
            assembler_destroy(assembler);
            return 1;
        }
        printf("Listing file saved to %s\n", options.listing_file);
    }
    
    printf("Assembly completed successfully\n");
    printf("Output size: %d bytes\n", assembler->output_size);
    
    // Cleanup
    assembler_destroy(assembler);
    return 0;
}

void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] INPUT_FILE\n", program_name);
    printf("\nOptions:\n");
    printf("  -o, --output FILE      Output binary file\n");
    printf("  -l, --listing FILE     Output listing file\n");
    printf("  -v, --verbose          Verbose output\n");
    printf("  -h, --help             Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s program.asm -o program.bin\n", program_name);
    printf("  %s program.asm -o program.bin -l program.lst\n", program_name);
    printf("  %s program.asm -v\n", program_name);
}

void print_help(void) {
    print_usage("asm");
    printf("\nAssembler Features:\n");
    printf("  - Labels and symbols\n");
    printf("  - Directives (.org, .byte, .word, .string)\n");
    printf("  - Multiple number formats (decimal, hex, binary)\n");
    printf("  - Comments (;)\n");
    printf("  - Include files (.include)\n");
    printf("\nSupported Instructions:\n");
    printf("  LDI, LDA, STA, MOV\n");
    printf("  ADD, SUB, CMP, INC, DEC\n");
    printf("  AND, OR, XOR\n");
    printf("  JMP, JSR, RTS\n");
    printf("  BEQ, BNE, BCS, BCC, BMI, BPL, BVS, BVC\n");
    printf("  PHA, PLA, PHP, PLP, PUSH, POP\n");
    printf("  SEI, CLI, NOP, HLT\n");
    printf("\nAddressing Modes:\n");
    printf("  Immediate: #value\n");
    printf("  Absolute: [address]\n");
    printf("  Indexed: [X+offset], [Y+offset]\n");
    printf("  Relative: offset (for branches)\n");
}

bool parse_cli_options(int argc, char* argv[], cli_options_t* options) {
    static struct option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"listing", required_argument, 0, 'l'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    // Set defaults
    options->input_file = NULL;
    options->output_file = NULL;
    options->listing_file = NULL;
    options->verbose = false;
    options->help_requested = false;
    
    while ((c = getopt_long(argc, argv, "o:l:vh", long_options, &option_index)) != -1) {
        switch (c) {
            case 'o':
                options->output_file = optarg;
                break;
            case 'l':
                options->listing_file = optarg;
                break;
            case 'v':
                options->verbose = true;
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
    
    // Get input file from remaining arguments
    if (optind < argc) {
        options->input_file = argv[optind];
    }
    
    return true;
}

void print_assembler_info(assembler_t* assembler) {
    printf("Assembler Information:\n");
    printf("  Input file: %s\n", assembler->filename);
    printf("  Output size: %d bytes\n", assembler->output_size);
    printf("  Origin address: 0x%04X\n", assembler->origin_address);
    printf("  Labels defined: %d\n", assembler->label_count);
    printf("  Symbols defined: %d\n", assembler->symbol_count);
    
    if (assembler->label_count > 0) {
        printf("\nLabels:\n");
        for (int i = 0; i < assembler->label_count; i++) {
            printf("  %s: 0x%04X (line %d)\n", 
                   assembler->labels[i].name, 
                   assembler->labels[i].address, 
                   assembler->labels[i].line);
        }
    }
    
    if (assembler->symbol_count > 0) {
        printf("\nSymbols:\n");
        for (int i = 0; i < assembler->symbol_count; i++) {
            printf("  %s: 0x%04X\n", 
                   assembler->symbols[i].name, 
                   assembler->symbols[i].value);
        }
    }
}

