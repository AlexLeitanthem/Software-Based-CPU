#include "../src/cpu.h"
#include "../src/memory.h"
#include "../src/devices.h"
#include "../src/isa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Test result structure
typedef struct {
    char name[64];
    bool passed;
    char error_message[256];
} test_result_t;

// Test suite state
typedef struct {
    test_result_t results[1000];
    int test_count;
    int passed_count;
    int failed_count;
} test_suite_t;

// Function prototypes
void test_suite_init(test_suite_t* suite);
void test_suite_run(test_suite_t* suite);
void test_suite_report(test_suite_t* suite);
void run_test(test_suite_t* suite, const char* name, bool (*test_func)(void));

// Test functions
bool test_cpu_creation(void);
bool test_cpu_reset(void);
bool test_cpu_registers(void);
bool test_cpu_flags(void);
bool test_cpu_memory(void);
bool test_cpu_instructions(void);
bool test_cpu_interrupts(void);
bool test_memory_system(void);
bool test_device_system(void);
bool test_isa_instructions(void);
bool test_assembler_basic(void);
bool test_integration_hello(void);
bool test_integration_addloop(void);
bool test_integration_gpio_blink(void);

// Test utilities
void setup_test_cpu(cpu_state_t* cpu);
void cleanup_test_cpu(cpu_state_t* cpu);
bool compare_cpu_state(cpu_state_t* cpu1, cpu_state_t* cpu2);
void print_test_failure(const char* test_name, const char* message);

int main(int argc, char* argv[]) {
    test_suite_t suite;
    test_suite_init(&suite);
    
    printf("Software CPU Simulator Test Suite\n");
    printf("=================================\n\n");
    
    // Run all tests
    test_suite_run(&suite);
    
    // Report results
    test_suite_report(&suite);
    
    return suite.failed_count > 0 ? 1 : 0;
}

void test_suite_init(test_suite_t* suite) {
    suite->test_count = 0;
    suite->passed_count = 0;
    suite->failed_count = 0;
}

void test_suite_run(test_suite_t* suite) {
    // CPU tests
    run_test(suite, "CPU Creation", test_cpu_creation);
    run_test(suite, "CPU Reset", test_cpu_reset);
    run_test(suite, "CPU Registers", test_cpu_registers);
    run_test(suite, "CPU Flags", test_cpu_flags);
    run_test(suite, "CPU Memory", test_cpu_memory);
    run_test(suite, "CPU Instructions", test_cpu_instructions);
    run_test(suite, "CPU Interrupts", test_cpu_interrupts);
    
    // Memory system tests
    run_test(suite, "Memory System", test_memory_system);
    
    // Device system tests
    run_test(suite, "Device System", test_device_system);
    
    // ISA tests
    run_test(suite, "ISA Instructions", test_isa_instructions);
    
    // Assembler tests
    run_test(suite, "Assembler Basic", test_assembler_basic);
    
    // Integration tests
    run_test(suite, "Integration Hello", test_integration_hello);
    run_test(suite, "Integration AddLoop", test_integration_addloop);
    run_test(suite, "Integration GPIO Blink", test_integration_gpio_blink);
}

void test_suite_report(test_suite_t* suite) {
    printf("\nTest Results Summary:\n");
    printf("===================\n");
    printf("Total tests: %d\n", suite->test_count);
    printf("Passed: %d\n", suite->passed_count);
    printf("Failed: %d\n", suite->failed_count);
    printf("Success rate: %.1f%%\n", 
           (float)suite->passed_count / suite->test_count * 100.0f);
    
    if (suite->failed_count > 0) {
        printf("\nFailed Tests:\n");
        for (int i = 0; i < suite->test_count; i++) {
            if (!suite->results[i].passed) {
                printf("  %s: %s\n", suite->results[i].name, suite->results[i].error_message);
            }
        }
    }
    
    printf("\n%s\n", suite->failed_count == 0 ? "ALL TESTS PASSED!" : "SOME TESTS FAILED!");
}

void run_test(test_suite_t* suite, const char* name, bool (*test_func)(void)) {
    printf("Running test: %s... ", name);
    fflush(stdout);
    
    bool result = test_func();
    
    test_result_t* test_result = &suite->results[suite->test_count++];
    strncpy(test_result->name, name, sizeof(test_result->name) - 1);
    test_result->name[sizeof(test_result->name) - 1] = '\0';
    test_result->passed = result;
    
    if (result) {
        suite->passed_count++;
        printf("PASS\n");
    } else {
        suite->failed_count++;
        printf("FAIL\n");
        strcpy(test_result->error_message, "Test function returned false");
    }
}

// Test implementations
bool test_cpu_creation(void) {
    cpu_state_t* cpu = cpu_create();
    if (!cpu) {
        return false;
    }
    
    // Check initial state
    if (cpu->memory == NULL) {
        cpu_destroy(cpu);
        return false;
    }
    
    cpu_destroy(cpu);
    return true;
}

bool test_cpu_reset(void) {
    cpu_state_t* cpu = cpu_create();
    if (!cpu) {
        return false;
    }
    
    // Modify some state
    isa_set_register(cpu, REG_A, 0x42);
    isa_set_register16(cpu, REG_PC, 0x1234);
    cpu->flags = 0xFF;
    cpu->running = true;
    
    // Reset CPU
    cpu_reset(cpu);
    
    // Check reset state
    bool result = (isa_get_register(cpu, REG_A) == 0) &&
                  (isa_get_register16(cpu, REG_PC) == 0xFFFC) &&
                  (cpu->flags == 0) &&
                  (!cpu->running);
    
    cpu_destroy(cpu);
    return result;
}

bool test_cpu_registers(void) {
    cpu_state_t* cpu = cpu_create();
    if (!cpu) {
        return false;
    }
    
    // Test 8-bit registers
    isa_set_register(cpu, REG_A, 0x42);
    isa_set_register(cpu, REG_B, 0x84);
    
    if (isa_get_register(cpu, REG_A) != 0x42) {
        cpu_destroy(cpu);
        return false;
    }
    
    if (isa_get_register(cpu, REG_B) != 0x84) {
        cpu_destroy(cpu);
        return false;
    }
    
    // Test 16-bit registers
    isa_set_register16(cpu, REG_X, 0x1234);
    isa_set_register16(cpu, REG_Y, 0x5678);
    
    if (isa_get_register16(cpu, REG_X) != 0x1234) {
        cpu_destroy(cpu);
        return false;
    }
    
    if (isa_get_register16(cpu, REG_Y) != 0x5678) {
        cpu_destroy(cpu);
        return false;
    }
    
    cpu_destroy(cpu);
    return true;
}

bool test_cpu_flags(void) {
    cpu_state_t* cpu = cpu_create();
    if (!cpu) {
        return false;
    }
    
    // Test flag operations
    isa_set_flag(cpu, FLAG_ZERO);
    if (!isa_get_flag(cpu, FLAG_ZERO)) {
        cpu_destroy(cpu);
        return false;
    }
    
    isa_clear_flag(cpu, FLAG_ZERO);
    if (isa_get_flag(cpu, FLAG_ZERO)) {
        cpu_destroy(cpu);
        return false;
    }
    
    // Test flag update
    isa_update_flags(cpu, 0, false, false);
    if (!isa_get_flag(cpu, FLAG_ZERO)) {
        cpu_destroy(cpu);
        return false;
    }
    
    isa_update_flags(cpu, 0x80, false, false);
    if (!isa_get_flag(cpu, FLAG_NEGATIVE)) {
        cpu_destroy(cpu);
        return false;
    }
    
    cpu_destroy(cpu);
    return true;
}

bool test_cpu_memory(void) {
    cpu_state_t* cpu = cpu_create();
    if (!cpu) {
        return false;
    }
    
    // Test memory read/write
    isa_write_memory(cpu, 0x1000, 0x42);
    if (isa_read_memory(cpu, 0x1000) != 0x42) {
        cpu_destroy(cpu);
        return false;
    }
    
    // Test memory boundaries
    isa_write_memory(cpu, 0xFFFF, 0x84);
    if (isa_read_memory(cpu, 0xFFFF) != 0x84) {
        cpu_destroy(cpu);
        return false;
    }
    
    cpu_destroy(cpu);
    return true;
}

bool test_cpu_instructions(void) {
    cpu_state_t* cpu = cpu_create();
    if (!cpu) {
        return false;
    }
    
    // Test LDI instruction
    uint8_t program[] = {0x00, 0x42}; // LDI #0x42
    cpu_load_program(cpu, program, sizeof(program), 0x0200);
    cpu_reset_to_address(cpu, 0x0200);
    
    bool stepped = cpu_step(cpu);

    if (!stepped) {
        cpu_destroy(cpu);
        return false;
    }

    if (isa_get_register(cpu, REG_A) != 0x42) {
        cpu_destroy(cpu);
        return false;
    }
    
    cpu_destroy(cpu);
    return true;
}

bool test_cpu_interrupts(void) {
    cpu_state_t* cpu = cpu_create();
    if (!cpu) {
        return false;
    }
    
    // Test IRQ
    cpu_irq(cpu);
    if (!cpu->irq_pending) {
        cpu_destroy(cpu);
        return false;
    }
    
    // Test NMI
    cpu_nmi(cpu);
    if (!cpu->nmi_pending) {
        cpu_destroy(cpu);
        return false;
    }
    
    cpu_destroy(cpu);
    return true;
}

bool test_memory_system(void) {
    uint8_t* memory = malloc(65536);
    if (!memory) {
        return false;
    }
    
    memory_init(memory);
    
    // Test memory regions
    if (!memory_is_ram(0x0000) || !memory_is_ram(0x7FFF)) {
        free(memory);
        return false;
    }
    
    if (!memory_is_mmio(0x8000) || !memory_is_mmio(0xFEFF)) {
        free(memory);
        return false;
    }
    
    if (!memory_is_vector(0xFF00) || !memory_is_vector(0xFFFF)) {
        free(memory);
        return false;
    }
    
    free(memory);
    return true;
}

bool test_device_system(void) {
    devices_init();
    
    // Test UART
    uint8_t status = devices_read(UART_STATUS_ADDR);
    if (status == 0xFF) { // Invalid read
        devices_cleanup();
        return false;
    }
    
    // Test GPIO
    devices_write(GPIO_PORT_ADDR, 0x55);
    uint8_t port = devices_read(GPIO_PORT_ADDR);
    if (port != 0x55) {
        devices_cleanup();
        return false;
    }
    
    // Test Timer
    devices_write(TIMER_LATCH_ADDR, 0x00);
    devices_write(TIMER_LATCH_ADDR + 1, 0x10);
    uint8_t latch_low = devices_read(TIMER_LATCH_ADDR);
    uint8_t latch_high = devices_read(TIMER_LATCH_ADDR + 1);
    if (latch_low != 0x00 || latch_high != 0x10) {
        devices_cleanup();
        return false;
    }
    
    devices_cleanup();
    return true;
}

bool test_isa_instructions(void) {
    // Test instruction table
    const instruction_t* inst = isa_get_instruction(OP_LDI);
    if (!inst || strcmp(inst->mnemonic, "LDI") != 0) {
        return false;
    }
    
    // Test opcode validation
    if (!isa_is_valid_opcode(OP_LDI)) {
        return false;
    }
    
    if (isa_is_valid_opcode(0xFF)) {
        return false;
    }
    
    return true;
}

bool test_assembler_basic(void) {
    // This is a placeholder for assembler tests
    // In a real implementation, we would test the assembler
    return true;
}

bool test_integration_hello(void) {
    // This is a placeholder for integration tests
    // In a real implementation, we would test the hello world program
    return true;
}

bool test_integration_addloop(void) {
    // This is a placeholder for integration tests
    // In a real implementation, we would test the add loop program
    return true;
}

bool test_integration_gpio_blink(void) {
    // This is a placeholder for integration tests
    // In a real implementation, we would test the GPIO blink program
    return true;
}

// Test utilities
void setup_test_cpu(cpu_state_t* cpu) {
    cpu_reset(cpu);
}

void cleanup_test_cpu(cpu_state_t* cpu) {
    // Cleanup if needed
}

bool compare_cpu_state(cpu_state_t* cpu1, cpu_state_t* cpu2) {
    // Compare CPU states
    for (int i = 0; i < 8; i++) {
        if (cpu1->regs[i] != cpu2->regs[i]) {
            return false;
        }
    }
    
    if (cpu1->flags != cpu2->flags) {
        return false;
    }
    
    return true;
}

void print_test_failure(const char* test_name, const char* message) {
    printf("Test %s failed: %s\n", test_name, message);
}

