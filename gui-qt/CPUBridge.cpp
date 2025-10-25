#include "CPUBridge.h"
#include <QTimer>
#include <cstdio>

CPUBridge::CPUBridge(QObject* parent) : QObject(parent), cpu(nullptr) {
}

CPUBridge::~CPUBridge() {
    if (cpu) cpu_destroy(cpu);
}

bool CPUBridge::initialize() {
    cpu = cpu_create();
    if (!cpu) return false;

    // Initialize memory system if needed
    memory_init(cpu->memory);
    return true;
}

void CPUBridge::reset() {
    if (cpu) cpu_reset(cpu);
    emit registersChanged();
}

void CPUBridge::step() {
    if (cpu) {
        cpu_step(cpu);
        emit registersChanged();
        uint8_t gpio = isa_read_memory(cpu, GPIO_PORT_ADDR);
        emit gpioChanged(gpio);
    }
}

void CPUBridge::run() {
    if (cpu) cpu->running = true;
}

void CPUBridge::stop() {
    if (cpu) cpu->running = false;
}

uint16_t CPUBridge::getPC() const { return cpu ? isa_get_register16(cpu, REG_PC) : 0; }
uint16_t CPUBridge::getSP() const { return cpu ? isa_get_register16(cpu, REG_SP) : 0; }
uint16_t CPUBridge::getX() const { return cpu ? isa_get_register16(cpu, REG_X) : 0; }
uint16_t CPUBridge::getY() const { return cpu ? isa_get_register16(cpu, REG_Y) : 0; }
bool CPUBridge::getCarry() const { return cpu ? isa_get_flag(cpu, FLAG_CARRY) : false; }
bool CPUBridge::getZero() const { return cpu ? isa_get_flag(cpu, FLAG_ZERO) : false; }
bool CPUBridge::getInterrupt() const { return cpu ? isa_get_flag(cpu, FLAG_INTERRUPT) : false; }

uint8_t CPUBridge::readMemory(uint16_t addr) const {
    if (!cpu) return 0;
    return isa_read_memory(cpu, addr);
}
