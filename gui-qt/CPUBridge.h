#pragma once

extern "C" {
#include "isa.h"
#include "cpu.h"
#include "memory.h"
#include "devices.h"
}

#include <QObject>

class CPUBridge : public QObject {
    Q_OBJECT
public:
    explicit CPUBridge(QObject* parent = nullptr);
    ~CPUBridge();

    bool initialize();
    void reset();
    void step();
    void run();
    void stop();

    // getters for display
    uint16_t getPC() const;
    uint16_t getSP() const;
    uint16_t getX() const;
    uint16_t getY() const;
    bool getCarry() const;
    bool getZero() const;
    bool getInterrupt() const;
    uint8_t readMemory(uint16_t addr) const;

signals:
    void registersChanged();
    void memoryChanged(uint16_t addr, uint8_t value);
    void gpioChanged(uint8_t value);

private:
    cpu_state_t* cpu;
};
