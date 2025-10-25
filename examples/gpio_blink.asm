; GPIO Blink Program
; Blinks GPIO pin 0 using timer interrupts
; Timer interrupt handler toggles GPIO pin

.org 0x0200

start:
    ; Initialize GPIO port (all outputs)
    LDI #0xFF
    STA [gpio_port]
    
    ; Initialize timer
    LDI #0x00
    STA [timer_latch]     ; Timer latch low
    LDI #0x10
    STA [timer_latch+1]   ; Timer latch high (0x1000)
    
    ; Enable timer interrupt
    LDI #0x06
    STA [timer_ctrl]      ; Start timer, enable IRQ
    
    ; Enable interrupts
    CLI
    
    ; Main loop - just wait for interrupts
main_loop:
    NOP
    JMP main_loop

; Timer interrupt handler
.org 0x0300

timer_irq:
    ; Save registers
    PHA
    PHP
    
    ; Toggle GPIO pin 0
    LDA [gpio_port]
    XOR #0x01
    STA [gpio_port]
    
    ; Clear timer interrupt
    LDI #0x01
    STA [timer_irq]
    
    ; Restore registers
    PLP
    PLA
    
    ; Return from interrupt
    RTI

; Variables
gpio_port:   .word 0x8003
timer_latch: .word 0x8004
timer_ctrl:  .word 0x8005
timer_irq:   .word 0x8007

