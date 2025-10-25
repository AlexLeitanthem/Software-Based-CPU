; Hello World Program
; Prints "HELLO" to the UART

.org 0x0200

start:
    ; Load 'H' into accumulator
    LDI #'H'
    STA [uart_tx]     ; Send to UART
    
    ; Load 'E' into accumulator
    LDI #'E'
    STA [uart_tx]     ; Send to UART
    
    ; Load 'L' into accumulator
    LDI #'L'
    STA [uart_tx]     ; Send to UART
    
    ; Load 'L' into accumulator
    LDI #'L'
    STA [uart_tx]     ; Send to UART
    
    ; Load 'O' into accumulator
    LDI #'O'
    STA [uart_tx]     ; Send to UART
    
    ; Load newline into accumulator
    LDI #'\n'
    STA [uart_tx]     ; Send to UART
    
    ; Halt the CPU
    HLT

; Constants
uart_tx: .word 0x8000

