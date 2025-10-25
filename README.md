# Software CPU Simulator

A complete, compilable C project for a **software CPU** (instruction-set simulator) suitable for electronics/CS labs. This project delivers high-quality code, documentation, and tests for a deterministic, reproducible CPU simulator.

## Features

### 1. ISA v1 (8-bit bytes, 16-bit addresses, little-endian)
- **Registers**: A,B,C,D general 8-bit; X,Y 16-bit index; SP 16; PC 16; FLAGS (Z,N,C,V,I)
- **Addressing**: immediate, register, [absolute], [X+offset], [Y+offset], [SP+offset], relative branch
- **Core Operations**: Load/Store, Arithmetic, Logical, Shift/Rotate, Jump/Call, Branch, Stack, System
- **Instruction Format**: 1-byte opcode + 0-2 operand bytes with cycle-accurate timing

### 2. Memory Map
- **64 KiB address space**
- **0x0000–0x7FFF**: RAM
- **0x8000–0xFEFF**: MMIO window
- **0xFF00–0xFFFF**: Reset/ISR vectors

### 3. Devices (Memory-Mapped)
- **UART**: TX at 0x8000, RX at 0x8001, STATUS at 0x8002
- **GPIO**: 8-bit port at 0x8003
- **TIMER**: 0x8004..0x8007 (latch, ctrl, count, irq flag)

### 4. CPU Core
- Single-step and run modes with cycle-accurate tick()
- Reset(), IRQ(), NMI() lines and interrupt handling
- Disassembler for any memory range
- Optional clock throttling (instructions/second)

### 5. Toolchain
- **Assembler**: Supports labels, .org, .byte/.word, constants, comments, include
- **Monitor**: CLI with load, run, step, regs, mem dump, breakpoints, watchpoints, disasm, save snapshot
- **Scriptable REPL commands**

### 6. Testing
- Self-test suite with golden traces
- ISA tests per instruction (flags, edge cases)
- Integration tests: UART echo, timer-driven blink, IRQ nesting
- Deterministic seeds with PASS/FAIL summary

### 7. Examples
- `examples/hello.asm`: Prints "HELLO"
- `examples/addloop.asm`: Sums 1..N and prints result
- `examples/gpio_blink.asm`: Toggles GPIO via timer IRQ

## Quick Start

### Build
```bash
make
```

### Run Examples
```bash
# Build examples
make examples

# Run CPU simulator
./build/cpu-sim examples/hello.bin --addr 0x0200 --run

# Interactive mode
./build/cpu-sim

# Monitor/debugger
./build/monitor
```

### Test Suite
```bash
make test
```

## Project Structure

```
├── src/                    # Source code
│   ├── cpu.h/c            # CPU core implementation
│   ├── isa.h/c            # Instruction set architecture
│   ├── memory.h/c         # Memory system
│   ├── devices.h/c        # Device implementations
│   ├── assembler.h/c      # Assembler
│   ├── cpu-sim.c          # Main CPU simulator
│   ├── asm.c              # Assembler program
│   ├── disasm.c           # Disassembler
│   └── monitor.c          # Monitor/debugger
├── tests/                 # Test suite
│   └── test_runner.c      # Test suite runner
├── examples/              # Example programs
│   ├── hello.asm         # Hello world
│   ├── addloop.asm       # Add loop
│   └── gpio_blink.asm    # GPIO blink
├── docs/                  # Documentation
├── build/                 # Build output
├── Makefile              # Build system
└── README.md             # This file
```

## Instruction Set Reference

### Load/Store Instructions
| Mnemonic | Opcode | Bytes | Cycles | Description |
|----------|--------|-------|--------|-------------|
| LDI #val | 0x00   | 2     | 2      | Load immediate |
| LDA [addr] | 0x01 | 3     | 3      | Load from memory |
| STA [addr] | 0x02 | 3     | 3      | Store to memory |
| MOV r,r  | 0x03   | 2     | 1      | Move register |

### Arithmetic Instructions
| Mnemonic | Opcode | Bytes | Cycles | Description |
|----------|--------|-------|--------|-------------|
| ADD #val | 0x10   | 2     | 2      | Add immediate |
| SUB #val | 0x11   | 2     | 2      | Subtract immediate |
| CMP #val | 0x14   | 2     | 2      | Compare |
| INC r    | 0x15   | 1     | 1      | Increment register |
| DEC r    | 0x16   | 1     | 1      | Decrement register |

### Logical Instructions
| Mnemonic | Opcode | Bytes | Cycles | Description |
|----------|--------|-------|--------|-------------|
| AND #val | 0x20   | 2     | 2      | Logical AND |
| OR #val  | 0x21   | 2     | 2      | Logical OR |
| XOR #val | 0x22   | 2     | 2      | Logical XOR |

### Jump/Call Instructions
| Mnemonic | Opcode | Bytes | Cycles | Description |
|----------|--------|-------|--------|-------------|
| JMP addr | 0x40   | 3     | 3      | Jump absolute |
| JSR addr | 0x41   | 3     | 6      | Jump to subroutine |
| RTS      | 0x42   | 1     | 6      | Return from subroutine |

### Branch Instructions
| Mnemonic | Opcode | Bytes | Cycles | Description |
|----------|--------|-------|--------|-------------|
| BEQ addr | 0x50   | 2     | 2      | Branch if equal (Z=1) |
| BNE addr | 0x51   | 2     | 2      | Branch if not equal (Z=0) |
| BCS addr | 0x52   | 2     | 2      | Branch if carry set (C=1) |
| BCC addr | 0x53   | 2     | 2      | Branch if carry clear (C=0) |
| BMI addr | 0x54   | 2     | 2      | Branch if minus (N=1) |
| BPL addr | 0x55   | 2     | 2      | Branch if plus (N=0) |
| BVS addr | 0x56   | 2     | 2      | Branch if overflow set (V=1) |
| BVC addr | 0x57   | 2     | 2      | Branch if overflow clear (V=0) |

### Stack Instructions
| Mnemonic | Opcode | Bytes | Cycles | Description |
|----------|--------|-------|--------|-------------|
| PHA      | 0x60   | 1     | 3      | Push accumulator |
| PLA      | 0x61   | 1     | 4      | Pull accumulator |
| PHP      | 0x62   | 1     | 3      | Push processor status |
| PLP      | 0x63   | 1     | 4      | Pull processor status |
| PUSH r   | 0x64   | 2     | 3      | Push register |
| POP r    | 0x65   | 2     | 4      | Pull register |

### System Instructions
| Mnemonic | Opcode | Bytes | Cycles | Description |
|----------|--------|-------|--------|-------------|
| SEI      | 0x70   | 1     | 2      | Set interrupt disable |
| CLI      | 0x71   | 1     | 2      | Clear interrupt disable |
| NOP      | 0x72   | 1     | 1      | No operation |
| HLT      | 0x73   | 1     | 1      | Halt |

## Addressing Modes

### Immediate
- Format: `#value`
- Example: `LDI #0x42`
- Loads the value directly into the accumulator

### Absolute
- Format: `[address]`
- Example: `LDA [0x1000]`
- Uses the address as an absolute memory location

### Indexed
- Format: `[X+offset]`, `[Y+offset]`
- Example: `LDA [X+0x10]`
- Uses index register plus offset as address

### Relative
- Format: `offset`
- Example: `BEQ loop`
- Used for branch instructions, calculates target from PC + offset

## Memory Map

| Address Range | Description |
|---------------|-------------|
| 0x0000-0x7FFF | RAM (32KB) |
| 0x8000-0xFEFF | MMIO Window |
| 0xFF00-0xFFFF | Vector Table |

### MMIO Devices

| Address | Device | Description |
|---------|--------|-------------|
| 0x8000  | UART TX | Transmit data |
| 0x8001  | UART RX | Receive data |
| 0x8002  | UART STATUS | Status register |
| 0x8003  | GPIO PORT | GPIO port data |
| 0x8004  | TIMER LATCH | Timer latch low |
| 0x8005  | TIMER LATCH | Timer latch high |
| 0x8006  | TIMER CTRL | Timer control |
| 0x8007  | TIMER IRQ | Timer interrupt flag |

## Usage Examples

### CPU Simulator
```bash
# Load and run a program
./build/cpu-sim examples/hello.bin --addr 0x0200 --run

# Interactive mode with tracing
./build/cpu-sim --trace --break 0x0300

# Run with frequency limit
./build/cpu-sim examples/addloop.bin --freq 1000000 --cycles 10000
```

### Assembler
```bash
# Assemble a program
./build/asm examples/hello.asm -o hello.bin

# Generate listing
./build/asm examples/hello.asm -o hello.bin -l hello.lst

# Verbose output
./build/asm examples/hello.asm -o hello.bin -v
```

### Monitor/Debugger
```bash
# Interactive monitor
./build/monitor

# Run script
./build/monitor -s commands.txt

# Monitor commands
monitor> load examples/hello.bin 0x0200
monitor> run
monitor> step
monitor> regs
monitor> mem 0x0200 16
monitor> disasm 0x0200 16
monitor> break 0x0300
monitor> watch 0x8000
monitor> trace on
monitor> quit
```

## Building

### Requirements
- C99 compatible compiler (GCC, Clang, MSVC)
- Make
- No external dependencies beyond libc

### Build Commands
```bash
# Build all tools
make

# Build specific tool
make cpu-sim
make asm
make disasm
make monitor
make tests

# Build examples
make examples

# Run tests
make test

# Clean build
make clean

# Debug build
make debug

# Install (optional)
make install
```

## Testing

The project includes a comprehensive test suite:

```bash
# Run all tests
make test

# Run specific test categories
./build/tests

# Test output
Software CPU Simulator Test Suite
=================================

Running test: CPU Creation... PASS
Running test: CPU Reset... PASS
Running test: CPU Registers... PASS
Running test: CPU Flags... PASS
Running test: CPU Memory... PASS
Running test: CPU Instructions... PASS
Running test: CPU Interrupts... PASS
Running test: Memory System... PASS
Running test: Device System... PASS
Running test: ISA Instructions... PASS
Running test: Assembler Basic... PASS
Running test: Integration Hello... PASS
Running test: Integration AddLoop... PASS
Running test: Integration GPIO Blink... PASS

Test Results Summary:
===================
Total tests: 14
Passed: 14
Failed: 0
Success rate: 100.0%

ALL TESTS PASSED!
```

## License

This project is provided as-is for educational purposes. Feel free to use, modify, and distribute according to your needs.

## Contributing

This is a complete, self-contained project. All components are implemented and tested. The codebase follows C99 standards and is designed to be portable across different platforms.

