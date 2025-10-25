#include "devices.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global device instances
uart_device_t g_uart;
gpio_device_t g_gpio;
timer_device_t g_timer;

// Device system functions
void devices_init(void) {
    uart_init(&g_uart);
    gpio_init(&g_gpio);
    timer_init(&g_timer);
}

void devices_cleanup(void) {
    // Cleanup if needed
}

void devices_tick(void) {
    uart_tick(&g_uart);
    gpio_tick(&g_gpio);
    timer_tick(&g_timer);
}

// Device access functions
uint8_t devices_read(uint16_t address) {
    switch (address) {
        case UART_TX_ADDR:
        case UART_RX_ADDR:
        case UART_STATUS_ADDR:
            return uart_read(&g_uart, address);
            
        case GPIO_PORT_ADDR:
            return gpio_read(&g_gpio, address);
            
        case TIMER_LATCH_ADDR:
        case TIMER_LATCH_ADDR_H:
        case TIMER_CTRL_ADDR:
        case TIMER_COUNT_ADDR:
        case TIMER_COUNT_ADDR_H:
        case TIMER_IRQ_ADDR:
            return timer_read(&g_timer, address);
            
        default:
            return 0;
    }
}

void devices_write(uint16_t address, uint8_t value) {
    switch (address) {
        case UART_TX_ADDR:
        case UART_RX_ADDR:
        case UART_STATUS_ADDR:
            uart_write(&g_uart, address, value);
            break;
            
        case GPIO_PORT_ADDR:
            // Allow writes to set the port value directly (tests expect this behavior)
            gpio_write(&g_gpio, address, value);
            break;
            
        case TIMER_LATCH_ADDR:
        case TIMER_LATCH_ADDR_H:
        case TIMER_CTRL_ADDR:
        case TIMER_COUNT_ADDR:
        case TIMER_COUNT_ADDR_H:
        case TIMER_IRQ_ADDR:
            timer_write(&g_timer, address, value);
            break;
    }
}

bool devices_is_readable(uint16_t address) {
    switch (address) {
        case UART_RX_ADDR:
        case UART_STATUS_ADDR:
        case GPIO_PORT_ADDR:
        case TIMER_LATCH_ADDR:
        case TIMER_LATCH_ADDR_H:
        case TIMER_CTRL_ADDR:
        case TIMER_COUNT_ADDR:
        case TIMER_COUNT_ADDR_H:
        case TIMER_IRQ_ADDR:
            return true;
        default:
            return false;
    }
}

bool devices_is_writable(uint16_t address) {
    switch (address) {
        case UART_TX_ADDR:
        case UART_STATUS_ADDR:
        case GPIO_PORT_ADDR:
        case TIMER_LATCH_ADDR:
        case TIMER_LATCH_ADDR_H:
        case TIMER_CTRL_ADDR:
        case TIMER_COUNT_ADDR:
        case TIMER_COUNT_ADDR_H:
        case TIMER_IRQ_ADDR:
            return true;
        default:
            return false;
    }
}

// UART implementation
void uart_init(uart_device_t* uart) {
    uart->tx_data = 0;
    uart->rx_data = 0;
    uart->status = 0;
    uart->tx_ready = true;
    uart->rx_ready = false;
    uart->tx_empty = true;
    uart->rx_full = false;
}

void uart_tick(uart_device_t* uart) {
    // Simulate UART timing
    // In a real implementation, this would handle serial communication
}

uint8_t uart_read(uart_device_t* uart, uint16_t address) {
    switch (address) {
        case UART_RX_ADDR:
            uart->rx_ready = false;
            return uart->rx_data;
            
        case UART_STATUS_ADDR:
            return (uart->tx_ready ? 0x01 : 0x00) | 
                   (uart->rx_ready ? 0x02 : 0x00) |
                   (uart->tx_empty ? 0x04 : 0x00) |
                   (uart->rx_full ? 0x08 : 0x00);
            
        default:
            return 0;
    }
}

void uart_write(uart_device_t* uart, uint16_t address, uint8_t value) {
    switch (address) {
        case UART_TX_ADDR:
            uart->tx_data = value;
            uart->tx_ready = false;
            uart->tx_empty = false;
            
            // Simulate character output
            printf("%c", value);
            fflush(stdout);
            
            // Mark as ready for next character
            uart->tx_ready = true;
            uart->tx_empty = true;
            break;
            
        case UART_STATUS_ADDR:
            // Status register is read-only
            break;
    }
}

void uart_send_char(uart_device_t* uart, char c) {
    uart_write(uart, UART_TX_ADDR, c);
}

char uart_receive_char(uart_device_t* uart) {
    if (uart->rx_ready) {
        uart->rx_ready = false;
        return uart->rx_data;
    }
    return 0;
}

bool uart_is_tx_ready(uart_device_t* uart) {
    return uart->tx_ready;
}

bool uart_is_rx_ready(uart_device_t* uart) {
    return uart->rx_ready;
}

// GPIO implementation
void gpio_init(gpio_device_t* gpio) {
    gpio->port = 0;
    gpio->direction = 0; // All inputs by default
    gpio->pullup = 0;    // No pullups by default
}

void gpio_tick(gpio_device_t* gpio) {
    // GPIO doesn't need periodic updates
}

uint8_t gpio_read(gpio_device_t* gpio, uint16_t address) {
    switch (address) {
        case GPIO_PORT_ADDR:
            return gpio->port;
            
        default:
            return 0;
    }
}

void gpio_write(gpio_device_t* gpio, uint16_t address, uint8_t value) {
    switch (address) {
        case GPIO_PORT_ADDR:
            // Write the full value to the port. Tests expect writes to set the value directly.
            gpio->port = value;
            break;
    }
}

void gpio_set_pin(gpio_device_t* gpio, uint8_t pin, bool state) {
    if (pin < 8) {
        if (state) {
            gpio->port |= (1 << pin);
        } else {
            gpio->port &= ~(1 << pin);
        }
    }
}

bool gpio_get_pin(gpio_device_t* gpio, uint8_t pin) {
    if (pin < 8) {
        return (gpio->port & (1 << pin)) != 0;
    }
    return false;
}

// Timer implementation
void timer_init(timer_device_t* timer) {
    timer->latch = 0;
    timer->count = 0;
    timer->control = 0;
    timer->irq_enabled = false;
    timer->irq_pending = false;
    timer->running = false;
    timer->prescaler = 1;
    timer->prescaler_count = 0;
}

void timer_tick(timer_device_t* timer) {
    if (!timer->running) {
        return;
    }
    
    timer->prescaler_count++;
    if (timer->prescaler_count >= timer->prescaler) {
        timer->prescaler_count = 0;
        
        if (timer->count > 0) {
            timer->count--;
            
            if (timer->count == 0) {
                // Timer expired
                if (timer->irq_enabled) {
                    timer->irq_pending = true;
                }
                
                // Reload from latch if in continuous mode
                if (timer->control & 0x01) {
                    timer->count = timer->latch;
                }
            }
        }
    }
}

uint8_t timer_read(timer_device_t* timer, uint16_t address) {
    switch (address) {
        case TIMER_LATCH_ADDR:
            return timer->latch & 0xFF;
            
        case TIMER_LATCH_ADDR_H:
            return (timer->latch >> 8) & 0xFF;
            
        case TIMER_CTRL_ADDR:
            return timer->control;
            
        case TIMER_COUNT_ADDR:
            return timer->count & 0xFF;
            
        case TIMER_COUNT_ADDR_H:
            return (timer->count >> 8) & 0xFF;
            
        case TIMER_IRQ_ADDR:
            return timer->irq_pending ? 0x01 : 0x00;
            
        default:
            return 0;
    }
}

void timer_write(timer_device_t* timer, uint16_t address, uint8_t value) {
    switch (address) {
        case TIMER_LATCH_ADDR:
            timer->latch = (timer->latch & 0xFF00) | value;
            break;
            
        case TIMER_LATCH_ADDR_H:
            timer->latch = (timer->latch & 0x00FF) | (value << 8);
            break;
            
        case TIMER_CTRL_ADDR:
            timer->control = value;
            timer->irq_enabled = (value & 0x02) != 0;
            
            if (value & 0x04) {
                timer_start(timer);
            } else {
                timer_stop(timer);
            }
            break;
            
        case TIMER_COUNT_ADDR:
            timer->count = (timer->count & 0xFF00) | value;
            break;
            
        case TIMER_COUNT_ADDR_H:
            timer->count = (timer->count & 0x00FF) | (value << 8);
            break;
            
        case TIMER_IRQ_ADDR:
            if (value & 0x01) {
                timer_clear_irq(timer);
            }
            break;
    }
}

void timer_start(timer_device_t* timer) {
    timer->running = true;
    timer->prescaler_count = 0;
}

void timer_stop(timer_device_t* timer) {
    timer->running = false;
}

void timer_reset(timer_device_t* timer) {
    timer->count = timer->latch;
    timer->prescaler_count = 0;
    timer->irq_pending = false;
}

bool timer_is_irq_pending(timer_device_t* timer) {
    return timer->irq_pending;
}

void timer_clear_irq(timer_device_t* timer) {
    timer->irq_pending = false;
}
