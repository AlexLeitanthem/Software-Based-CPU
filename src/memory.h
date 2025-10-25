#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdbool.h>

// Memory map definitions
#define RAM_START 0x0000
#define RAM_END 0x7FFF
#define RAM_SIZE (RAM_END - RAM_START + 1)

#define MMIO_START 0x8000
#define MMIO_END 0xFEFF
#define MMIO_SIZE (MMIO_END - MMIO_START + 1)

#define VECTOR_START 0xFF00
#define VECTOR_END 0xFFFF
#define VECTOR_SIZE (VECTOR_END - VECTOR_START + 1)

// Device addresses
#define UART_TX_ADDR 0x8000
#define UART_RX_ADDR 0x8001
#define UART_STATUS_ADDR 0x8002

#define GPIO_PORT_ADDR 0x8003

#define TIMER_LATCH_ADDR 0x8004
#define TIMER_LATCH_ADDR_H 0x8005
#define TIMER_CTRL_ADDR 0x8006
#define TIMER_COUNT_ADDR 0x8007
#define TIMER_COUNT_ADDR_H 0x8008
#define TIMER_IRQ_ADDR 0x8009

// Memory access types
typedef enum {
    MEM_READ = 0,
    MEM_WRITE = 1
} memory_access_t;

// Memory callback function type
typedef uint8_t (*memory_callback_t)(uint16_t address, uint8_t value, memory_access_t access);

// Memory system functions
void memory_init(uint8_t* memory);
void memory_cleanup(void);

// Memory access functions
uint8_t memory_read(uint8_t* memory, uint16_t address);
void memory_write(uint8_t* memory, uint16_t address, uint8_t value);
uint16_t memory_read16(uint8_t* memory, uint16_t address);
void memory_write16(uint8_t* memory, uint16_t address, uint16_t value);

// Memory region functions
bool memory_is_ram(uint16_t address);
bool memory_is_mmio(uint16_t address);
bool memory_is_vector(uint16_t address);

// Memory dump functions
void memory_dump(uint8_t* memory, uint16_t start, uint16_t end);
void memory_dump_hex(uint8_t* memory, uint16_t start, uint16_t end);
void memory_dump_disasm(uint8_t* memory, uint16_t start, uint16_t end);

// Memory fill functions
void memory_fill(uint8_t* memory, uint16_t start, uint16_t end, uint8_t value);
void memory_copy(uint8_t* memory, uint16_t dest, uint16_t src, uint16_t size);

// Memory validation
bool memory_is_valid_address(uint16_t address);
bool memory_is_writable(uint16_t address);
bool memory_is_readable(uint16_t address);

#endif // MEMORY_H
