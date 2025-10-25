#ifndef DEVICES_H
#define DEVICES_H

#include <stdint.h>
#include <stdbool.h>

// Device types
typedef enum {
    DEVICE_UART = 0,
    DEVICE_GPIO = 1,
    DEVICE_TIMER = 2
} device_type_t;

// UART device
typedef struct {
    uint8_t tx_data;
    uint8_t rx_data;
    uint8_t status;
    bool tx_ready;
    bool rx_ready;
    bool tx_empty;
    bool rx_full;
} uart_device_t;

// GPIO device
typedef struct {
    uint8_t port;
    uint8_t direction; // 0=input, 1=output
    uint8_t pullup;   // 0=disabled, 1=enabled
} gpio_device_t;

// Timer device
typedef struct {
    uint16_t latch;
    uint16_t count;
    uint8_t control;
    bool irq_enabled;
    bool irq_pending;
    bool running;
    uint32_t prescaler;
    uint32_t prescaler_count;
} timer_device_t;

// Device system functions
void devices_init(void);
void devices_cleanup(void);
void devices_tick(void);

// Device access functions
uint8_t devices_read(uint16_t address);
void devices_write(uint16_t address, uint8_t value);
bool devices_is_readable(uint16_t address);
bool devices_is_writable(uint16_t address);

// UART functions
void uart_init(uart_device_t* uart);
void uart_tick(uart_device_t* uart);
uint8_t uart_read(uart_device_t* uart, uint16_t address);
void uart_write(uart_device_t* uart, uint16_t address, uint8_t value);
void uart_send_char(uart_device_t* uart, char c);
char uart_receive_char(uart_device_t* uart);
bool uart_is_tx_ready(uart_device_t* uart);
bool uart_is_rx_ready(uart_device_t* uart);

// GPIO functions
void gpio_init(gpio_device_t* gpio);
void gpio_tick(gpio_device_t* gpio);
uint8_t gpio_read(gpio_device_t* gpio, uint16_t address);
void gpio_write(gpio_device_t* gpio, uint16_t address, uint8_t value);
void gpio_set_pin(gpio_device_t* gpio, uint8_t pin, bool state);
bool gpio_get_pin(gpio_device_t* gpio, uint8_t pin);

// Timer functions
void timer_init(timer_device_t* timer);
void timer_tick(timer_device_t* timer);
uint8_t timer_read(timer_device_t* timer, uint16_t address);
void timer_write(timer_device_t* timer, uint16_t address, uint8_t value);
void timer_start(timer_device_t* timer);
void timer_stop(timer_device_t* timer);
void timer_reset(timer_device_t* timer);
bool timer_is_irq_pending(timer_device_t* timer);
void timer_clear_irq(timer_device_t* timer);

// Global device instances
extern uart_device_t g_uart;
extern gpio_device_t g_gpio;
extern timer_device_t g_timer;

#endif // DEVICES_H

