; Add Loop Program
; Sums numbers from 1 to N and prints the result
; N is stored at address 0x1000

.org 0x0200

start:
    ; Initialize variables
    LDI #0
    STA [sum]         ; sum = 0
    LDI #1
    STA [counter]     ; counter = 1
    
    ; Load N from memory
    LDA [N]
    STA [limit]       ; limit = N
    
loop:
    ; Check if counter > limit
    LDA [counter]
    CMP [limit]
    BEQ done          ; If counter == limit, we're done
    
    ; Add counter to sum
    LDA [sum]
    ADD [counter]
    STA [sum]
    
    ; Increment counter
    LDA [counter]
    INC A
    STA [counter]
    
    ; Jump back to loop
    JMP loop
    
done:
    ; Print the result
    LDA [sum]
    STA [uart_tx]     ; Send result to UART
    
    ; Load newline into accumulator
    LDI #'\n'
    STA [uart_tx]     ; Send to UART
    
    ; Halt the CPU
    HLT

; Variables
sum:     .byte 0
counter: .byte 0
limit:   .byte 0

; Constants
N:       .byte 10     ; Sum from 1 to 10
uart_tx: .word 0x8000

