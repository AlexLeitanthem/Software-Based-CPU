cmake_minimum_required(VERSION 3.10)
project(SoftwareCPU VERSION 1.0.0 LANGUAGES C)

# Set C99 standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2 -g")
endif()

# Source files
set(CPU_SOURCES
    src/cpu.c
    src/memory.c
    src/devices.c
    src/isa.c
)

set(ASM_SOURCES
    src/assembler.c
    src/isa.c
)

set(DISASM_SOURCES
    src/disasm.c
    src/isa.c
)

set(MONITOR_SOURCES
    src/monitor.c
    src/cpu.c
    src/memory.c
    src/devices.c
    src/isa.c
)

set(TEST_SOURCES
    tests/test_runner.c
    src/cpu.c
    src/memory.c
    src/devices.c
    src/isa.c
)

# Executables
add_executable(cpu-sim src/cpu-sim.c ${CPU_SOURCES})
add_executable(asm src/asm.c ${ASM_SOURCES})
add_executable(disasm src/disasm.c ${DISASM_SOURCES})
add_executable(monitor src/monitor.c ${MONITOR_SOURCES})
add_executable(tests ${TEST_SOURCES})

# Debug helper (not installed)
add_executable(debug_lditest src/debug_lditest.c src/cpu.c src/isa.c src/memory.c src/devices.c)

# Set output directory
set_target_properties(cpu-sim asm disasm monitor tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build
)

# Install targets
install(TARGETS cpu-sim asm disasm monitor tests
    RUNTIME DESTINATION bin
)

# Custom targets
add_custom_target(examples
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/build/examples
    COMMAND ${CMAKE_BINARY_DIR}/build/asm ${CMAKE_SOURCE_DIR}/examples/hello.asm -o ${CMAKE_BINARY_DIR}/build/examples/hello.bin
    COMMAND ${CMAKE_BINARY_DIR}/build/asm ${CMAKE_SOURCE_DIR}/examples/addloop.asm -o ${CMAKE_BINARY_DIR}/build/examples/addloop.bin
    COMMAND ${CMAKE_BINARY_DIR}/build/asm ${CMAKE_SOURCE_DIR}/examples/gpio_blink.asm -o ${CMAKE_BINARY_DIR}/build/examples/gpio_blink.bin
    DEPENDS asm
    COMMENT "Building example programs"
)

add_custom_target(test
    COMMAND ${CMAKE_BINARY_DIR}/build/tests
    DEPENDS tests
    COMMENT "Running test suite"
)

# Documentation
add_custom_target(docs
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/docs
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/README.md ${CMAKE_BINARY_DIR}/docs/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/docs/instruction_reference.md ${CMAKE_BINARY_DIR}/docs/
    COMMENT "Generating documentation"
)

# Help target
add_custom_target(proj_help
    COMMAND ${CMAKE_COMMAND} -E echo "Available targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  cpu-sim    - Build CPU simulator"
    COMMAND ${CMAKE_COMMAND} -E echo "  asm        - Build assembler"
    COMMAND ${CMAKE_COMMAND} -E echo "  disasm     - Build disassembler"
    COMMAND ${CMAKE_COMMAND} -E echo "  monitor    - Build monitor/debugger"
    COMMAND ${CMAKE_COMMAND} -E echo "  tests      - Build test suite"
    COMMAND ${CMAKE_COMMAND} -E echo "  examples   - Build example programs"
    COMMAND ${CMAKE_COMMAND} -E echo "  test       - Run test suite"
    COMMAND ${CMAKE_COMMAND} -E echo "  docs       - Generate documentation"
    COMMAND ${CMAKE_COMMAND} -E echo "  help       - Show this help"
    COMMENT "Available targets"
)

# Set default target
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY DEFAULT_TARGET cpu-sim)

