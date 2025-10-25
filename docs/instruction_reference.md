# Instruction Set Architecture Reference

## Overview

The Software CPU Simulator implements a custom 8-bit instruction set architecture with 16-bit addressing. The CPU features 8-bit general-purpose registers, 16-bit index registers, and a comprehensive set of instructions for arithmetic, logical, memory, and control operations.

## Registers

### 8-bit Registers
- **A**: Accumulator (primary register for arithmetic and logical operations)
- **B**: General-purpose register
- **C**: General-purpose register  
- **D**: General-purpose register

### 16-bit Registers
- **X**: Index register (used for indexed addressing)
- **Y**: Index register (used for indexed addressing)
- **SP**: Stack pointer (points to top of stack)
- **PC**: Program counter (points to next instruction)

### Status Flags
- **Z**: Zero flag (set when result is zero)
- **N**: Negative flag (set when result is negative)
- **C**: Carry flag (set when operation produces carry)
- **V**: Overflow flag (set when signed overflow occurs)
- **I**: Interrupt disable flag (set when interrupts are disabled)

## Instruction Format

All instructions are encoded as:
- **1 byte**: Opcode
- **0-2 bytes**: Operands (depending on instruction and addressing mode)

## Addressing Modes

### Immediate (#value)
Loads a constant value directly into the instruction.
```
LDI #0x42    ; Load immediate value 0x42 into accumulator
```

### Register (r)
Uses a register as the operand.
```
MOV A, B      ; Move register B to register A
```

### Absolute ([address])
Uses an absolute memory address.
```
LDA [0x1000] ; Load from memory address 0x1000
STA [0x2000] ; Store to memory address 0x2000
```

### Indexed ([X+offset], [Y+offset])
Uses an index register plus an offset.
```
LDA [X+0x10] ; Load from address X + 0x10
STA [Y+0x20] ; Store to address Y + 0x20
```

### Relative (offset)
Used for branch instructions. Calculates target as PC + offset.
```
BEQ loop     ; Branch to 'loop' if zero flag is set
```

## Instruction Set

### Load/Store Instructions

#### LDI (Load Immediate)
- **Opcode**: 0x00
- **Bytes**: 2
- **Cycles**: 2
- **Description**: Loads an immediate value into the accumulator
- **Syntax**: `LDI #value`
- **Example**: `LDI #0x42`

#### LDA (Load Accumulator)
- **Opcode**: 0x01
- **Bytes**: 3
- **Cycles**: 3
- **Description**: Loads a value from memory into the accumulator
- **Syntax**: `LDA [address]`
- **Example**: `LDA [0x1000]`

#### STA (Store Accumulator)
- **Opcode**: 0x02
- **Bytes**: 3
- **Cycles**: 3
- **Description**: Stores the accumulator value to memory
- **Syntax**: `STA [address]`
- **Example**: `STA [0x2000]`

#### MOV (Move Register)
- **Opcode**: 0x03
- **Bytes**: 2
- **Cycles**: 1
- **Description**: Moves a value from one register to another
- **Syntax**: `MOV dest, src`
- **Example**: `MOV A, B`

### Arithmetic Instructions

#### ADD (Add)
- **Opcode**: 0x10
- **Bytes**: 2
- **Cycles**: 2
- **Description**: Adds a value to the accumulator
- **Syntax**: `ADD #value`
- **Example**: `ADD #0x10`
- **Flags**: Z, N, C, V

#### SUB (Subtract)
- **Opcode**: 0x11
- **Bytes**: 2
- **Cycles**: 2
- **Description**: Subtracts a value from the accumulator
- **Syntax**: `SUB #value`
- **Example**: `SUB #0x05`
- **Flags**: Z, N, C, V

#### CMP (Compare)
- **Opcode**: 0x14
- **Bytes**: 2
- **Cycles**: 2
- **Description**: Compares a value with the accumulator (sets flags, doesn't modify accumulator)
- **Syntax**: `CMP #value`
- **Example**: `CMP #0x20`
- **Flags**: Z, N, C, V

#### INC (Increment)
- **Opcode**: 0x15
- **Bytes**: 1
- **Cycles**: 1
- **Description**: Increments a register by 1
- **Syntax**: `INC r`
- **Example**: `INC A`
- **Flags**: Z, N

#### DEC (Decrement)
- **Opcode**: 0x16
- **Bytes**: 1
- **Cycles**: 1
- **Description**: Decrements a register by 1
- **Syntax**: `DEC r`
- **Example**: `DEC B`
- **Flags**: Z, N

### Logical Instructions

#### AND (Logical AND)
- **Opcode**: 0x20
- **Bytes**: 2
- **Cycles**: 2
- **Description**: Performs bitwise AND between accumulator and value
- **Syntax**: `AND #value`
- **Example**: `AND #0x0F`
- **Flags**: Z, N

#### OR (Logical OR)
- **Opcode**: 0x21
- **Bytes**: 2
- **Cycles**: 2
- **Description**: Performs bitwise OR between accumulator and value
- **Syntax**: `OR #value`
- **Example**: `OR #0x80`
- **Flags**: Z, N

#### XOR (Logical XOR)
- **Opcode**: 0x22
- **Bytes**: 2
- **Cycles**: 2
- **Description**: Performs bitwise XOR between accumulator and value
- **Syntax**: `XOR #value`
- **Example**: `XOR #0xFF`
- **Flags**: Z, N

### Jump/Call Instructions

#### JMP (Jump)
- **Opcode**: 0x40
- **Bytes**: 3
- **Cycles**: 3
- **Description**: Unconditionally jumps to an address
- **Syntax**: `JMP address`
- **Example**: `JMP 0x1000`

#### JSR (Jump to Subroutine)
- **Opcode**: 0x41
- **Bytes**: 3
- **Cycles**: 6
- **Description**: Jumps to a subroutine, saving return address on stack
- **Syntax**: `JSR address`
- **Example**: `JSR 0x2000`

#### RTS (Return from Subroutine)
- **Opcode**: 0x42
- **Bytes**: 1
- **Cycles**: 6
- **Description**: Returns from a subroutine, restoring return address from stack
- **Syntax**: `RTS`
- **Example**: `RTS`

### Branch Instructions

#### BEQ (Branch if Equal)
- **Opcode**: 0x50
- **Bytes**: 2
- **Cycles**: 2
- **Description**: Branches if zero flag is set
- **Syntax**: `BEQ address`
- **Example**: `BEQ loop`

#### BNE (Branch if Not Equal)
- **Opcode**: 0x51
- **Bytes**: 2
- **Cycles**: 2
- **Description**: Branches if zero flag is clear
- **Syntax**: `BNE address`
- **Example**: `BNE loop`

#### BCS (Branch if Carry Set)
- **Opcode**: 0x52
- **Bytes**: 2
- **Cycles**: 2
- **Description**: Branches if carry flag is set
- **Syntax**: `BCS address`
- **Example**: `BCS error`

#### BCC (Branch if Carry Clear)
- **Opcode**: 0x53
- **Bytes**: 2
- **Cycles**: 2
- **Description**: Branches if carry flag is clear
- **Syntax**: `BCC address`
- **Example**: `BCC ok`

#### BMI (Branch if Minus)
- **Opcode**: 0x54
- **Bytes**: 2
- **Cycles**: 2
- **Description**: Branches if negative flag is set
- **Syntax**: `BMI address`
- **Example**: `BMI negative`

#### BPL (Branch if Plus)
- **Opcode**: 0x55
- **Bytes**: 2
- **Cycles**: 2
- **Description**: Branches if negative flag is clear
- **Syntax**: `BPL address`
- **Example**: `BPL positive`

#### BVS (Branch if Overflow Set)
- **Opcode**: 0x56
- **Bytes**: 2
- **Cycles**: 2
- **Description**: Branches if overflow flag is set
- **Syntax**: `BVS address`
- **Example**: `BVS overflow`

#### BVC (Branch if Overflow Clear)
- **Opcode**: 0x57
- **Bytes**: 2
- **Cycles**: 2
- **Description**: Branches if overflow flag is clear
- **Syntax**: `BVC address`
- **Example**: `BVC no_overflow`

### Stack Instructions

#### PHA (Push Accumulator)
- **Opcode**: 0x60
- **Bytes**: 1
- **Cycles**: 3
- **Description**: Pushes accumulator onto stack
- **Syntax**: `PHA`
- **Example**: `PHA`

#### PLA (Pull Accumulator)
- **Opcode**: 0x61
- **Bytes**: 1
- **Cycles**: 4
- **Description**: Pulls accumulator from stack
- **Syntax**: `PLA`
- **Example**: `PLA`

#### PHP (Push Processor Status)
- **Opcode**: 0x62
- **Bytes**: 1
- **Cycles**: 3
- **Description**: Pushes processor status flags onto stack
- **Syntax**: `PHP`
- **Example**: `PHP`

#### PLP (Pull Processor Status)
- **Opcode**: 0x63
- **Bytes**: 1
- **Cycles**: 4
- **Description**: Pulls processor status flags from stack
- **Syntax**: `PLP`
- **Example**: `PLP`

#### PUSH (Push Register)
- **Opcode**: 0x64
- **Bytes**: 2
- **Cycles**: 3
- **Description**: Pushes a register onto stack
- **Syntax**: `PUSH r`
- **Example**: `PUSH B`

#### POP (Pull Register)
- **Opcode**: 0x65
- **Bytes**: 2
- **Cycles**: 4
- **Description**: Pulls a register from stack
- **Syntax**: `POP r`
- **Example**: `POP C`

### System Instructions

#### SEI (Set Interrupt Disable)
- **Opcode**: 0x70
- **Bytes**: 1
- **Cycles**: 2
- **Description**: Sets the interrupt disable flag
- **Syntax**: `SEI`
- **Example**: `SEI`

#### CLI (Clear Interrupt Disable)
- **Opcode**: 0x71
- **Bytes**: 1
- **Cycles**: 2
- **Description**: Clears the interrupt disable flag
- **Syntax**: `CLI`
- **Example**: `CLI`

#### NOP (No Operation)
- **Opcode**: 0x72
- **Bytes**: 1
- **Cycles**: 1
- **Description**: Performs no operation (useful for timing)
- **Syntax**: `NOP`
- **Example**: `NOP`

#### HLT (Halt)
- **Opcode**: 0x73
- **Bytes**: 1
- **Cycles**: 1
- **Description**: Halts the CPU
- **Syntax**: `HLT`
- **Example**: `HLT`

## Memory Map

| Address Range | Description | Size |
|---------------|-------------|------|
| 0x0000-0x7FFF | RAM | 32KB |
| 0x8000-0xFEFF | MMIO Window | 30KB |
| 0xFF00-0xFFFF | Vector Table | 256 bytes |

### Vector Table
- **0xFFFC-0xFFFD**: Reset vector
- **0xFFFA-0xFFFB**: NMI vector  
- **0xFFFE-0xFFFF**: IRQ vector

### MMIO Devices
- **0x8000**: UART TX (write character)
- **0x8001**: UART RX (read character)
- **0x8002**: UART STATUS (read status)
- **0x8003**: GPIO PORT (read/write)
- **0x8004**: TIMER LATCH (low byte)
- **0x8005**: TIMER LATCH (high byte)
- **0x8006**: TIMER CTRL (control register)
- **0x8007**: TIMER IRQ (interrupt flag)

## Programming Examples

### Hello World
```assembly
.org 0x0200

start:
    LDI #'H'
    STA [0x8000]     ; Send to UART
    LDI #'e'
    STA [0x8000]
    LDI #'l'
    STA [0x8000]
    LDI #'l'
    STA [0x8000]
    LDI #'o'
    STA [0x8000]
    LDI #'\n'
    STA [0x8000]
    HLT
```

### Simple Loop
```assembly
.org 0x0200

start:
    LDI #0
    STA [counter]
    
loop:
    LDA [counter]
    INC A
    STA [counter]
    CMP #10
    BNE loop
    
    HLT

counter: .byte 0
```

### Subroutine Call
```assembly
.org 0x0200

start:
    LDI #0x42
    JSR print_char
    HLT

print_char:
    STA [0x8000]     ; Send to UART
    RTS
```

## Assembler Directives

### .org address
Sets the current assembly address.
```assembly
.org 0x0200
```

### .byte value
Defines a byte value.
```assembly
counter: .byte 0
```

### .word value
Defines a 16-bit word value.
```assembly
address: .word 0x1000
```

### .string "text"
Defines a null-terminated string.
```assembly
message: .string "Hello, World!"
```

### Comments
Use semicolon (;) for comments.
```assembly
; This is a comment
LDI #0x42    ; Load 0x42 into accumulator
```

## Number Formats

### Decimal
```assembly
LDI #42      ; Decimal 42
```

### Hexadecimal
```assembly
LDI #$2A     ; Hexadecimal 2A (42 decimal)
```

### Binary
```assembly
LDI #%101010 ; Binary 101010 (42 decimal)
```

## Best Practices

1. **Use meaningful labels** for addresses and constants
2. **Comment your code** to explain complex logic
3. **Use subroutines** for reusable code
4. **Save registers** before calling subroutines
5. **Check flags** after arithmetic operations
6. **Use appropriate addressing modes** for efficiency
7. **Test thoroughly** with different input values
8. **Document your code** for future reference

