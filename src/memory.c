#include "memory.h"
#include "devices.h"
#include <stdio.h>
#include <string.h>

// Memory access functions
uint8_t memory_read(uint8_t* memory, uint16_t address) {
    if (!memory_is_valid_address(address)) {
        return 0;
    }
    
    if (memory_is_ram(address)) {
        return memory[address];
    } else if (memory_is_mmio(address)) {
        return devices_read(address);
    } else if (memory_is_vector(address)) {
        return memory[address];
    }
    
    return 0;
}

void memory_write(uint8_t* memory, uint16_t address, uint8_t value) {
    if (!memory_is_valid_address(address)) {
        return;
    }
    
    if (memory_is_ram(address)) {
        memory[address] = value;
    } else if (memory_is_mmio(address)) {
        devices_write(address, value);
    } else if (memory_is_vector(address)) {
        memory[address] = value;
    }
}

uint16_t memory_read16(uint8_t* memory, uint16_t address) {
    uint8_t low = memory_read(memory, address);
    uint8_t high = memory_read(memory, address + 1);
    return low | (high << 8);
}

void memory_write16(uint8_t* memory, uint16_t address, uint16_t value) {
    memory_write(memory, address, value & 0xFF);
    memory_write(memory, address + 1, (value >> 8) & 0xFF);
}

// Memory region functions
bool memory_is_ram(uint16_t address) {
    // RAM_START is 0x0000 for this address space, so only the upper bound is needed
    return address <= RAM_END;
}

bool memory_is_mmio(uint16_t address) {
    return address >= MMIO_START && address <= MMIO_END;
}

bool memory_is_vector(uint16_t address) {
    // VECTOR_END is 0xFFFF (max for uint16_t), so only the lower bound matters
    return address >= VECTOR_START;
}

// Memory dump functions
void memory_dump(uint8_t* memory, uint16_t start, uint16_t end) {
    printf("Memory dump from 0x%04X to 0x%04X:\n", start, end);
    printf("Address  ");
    for (int i = 0; i < 16; i++) {
        printf("%02X ", i);
    }
    printf("\n");
    
    for (uint16_t addr = start; addr <= end; addr += 16) {
        printf("0x%04X: ", addr);
        for (int i = 0; i < 16 && addr + i <= end; i++) {
            printf("%02X ", memory_read(memory, addr + i));
        }
        printf("\n");
    }
}

void memory_dump_hex(uint8_t* memory, uint16_t start, uint16_t end) {
    printf("Memory hex dump from 0x%04X to 0x%04X:\n", start, end);
    for (uint16_t addr = start; addr <= end; addr += 16) {
        printf("%04X: ", addr);
        for (int i = 0; i < 16 && addr + i <= end; i++) {
            printf("%02X ", memory_read(memory, addr + i));
        }
        printf("\n");
    }
}

void memory_dump_disasm(uint8_t* memory, uint16_t start, uint16_t end) {
    printf("Memory disassembly from 0x%04X to 0x%04X:\n", start, end);
    // This would need to be implemented with the disassembler
    // For now, just show hex
    memory_dump_hex(memory, start, end);
}

// Memory fill functions
void memory_fill(uint8_t* memory, uint16_t start, uint16_t end, uint8_t value) {
    for (uint16_t addr = start; addr <= end; addr++) {
        if (memory_is_ram(addr) || memory_is_vector(addr)) {
            memory[addr] = value;
        }
    }
}

void memory_copy(uint8_t* memory, uint16_t dest, uint16_t src, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        uint8_t value = memory_read(memory, src + i);
        memory_write(memory, dest + i, value);
    }
}

// Memory validation
bool memory_is_valid_address(uint16_t address) {
    // address is a uint16_t so it's always a valid 16-bit address
    return true;
}

bool memory_is_writable(uint16_t address) {
    if (!memory_is_valid_address(address)) {
        return false;
    }
    
    if (memory_is_ram(address)) {
        return true;
    } else if (memory_is_mmio(address)) {
        return devices_is_writable(address);
    } else if (memory_is_vector(address)) {
        return true;
    }
    
    return false;
}

bool memory_is_readable(uint16_t address) {
    if (!memory_is_valid_address(address)) {
        return false;
    }
    
    if (memory_is_ram(address)) {
        return true;
    } else if (memory_is_mmio(address)) {
        return devices_is_readable(address);
    } else if (memory_is_vector(address)) {
        return true;
    }
    
    return false;
}

// Initialize memory system
void memory_init(uint8_t* memory) {
    // Clear all memory
    memset(memory, 0, 0x10000);
    
    // Set up reset vectors (default to 0x0200)
    memory_write16(memory, 0xFFFC, 0x0200); // Reset vector
    memory_write16(memory, 0xFFFA, 0x0200); // NMI vector
    memory_write16(memory, 0xFFFE, 0x0200); // IRQ vector
    
    // Initialize devices
    devices_init();
}

// Cleanup memory system
void memory_cleanup(void) {
    devices_cleanup();
}

