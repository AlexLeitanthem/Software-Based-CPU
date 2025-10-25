cmake_minimum_required(VERSION 3.10)
project(SoftwareCPU VERSION 1.0.0 LANGUAGES C CXX)

# Set C99 and C++17 standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include FetchContent for downloading dependencies
include(FetchContent)

# Configure SDL2 and handle paths with spaces
set(SDL2_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl2-src")
set(SDL2_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl2-build")

FetchContent_Declare(
    sdl2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.28.4
    SOURCE_DIR "${SDL2_SOURCE_DIR}"
    BINARY_DIR "${SDL2_BINARY_DIR}"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)
FetchContent_MakeAvailable(sdl2)

# Configure SDL2 with explicit paths
file(TO_NATIVE_PATH "${SDL2_SOURCE_DIR}" SDL2_SOURCE_DIR_NATIVE)
file(TO_NATIVE_PATH "${SDL2_BINARY_DIR}" SDL2_BINARY_DIR_NATIVE)

# Configure Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.89.9
)
FetchContent_MakeAvailable(imgui)

# ImGui source files
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

# Compiler flags
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2 -g")
endif()

# Create CPU library for reuse
add_library(cpu_lib
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

set(GUI_SOURCES
    gui/main.cpp
    gui/visualizer.cpp
    ${IMGUI_SOURCES}
)

# Executables
add_executable(cpu-sim src/cpu-sim.c)
add_executable(asm src/asm.c ${ASM_SOURCES})
add_executable(disasm src/disasm.c ${DISASM_SOURCES})
add_executable(monitor src/monitor.c)
add_executable(tests tests/test_runner.c)
add_executable(cpu-visualizer ${GUI_SOURCES})

# Link with CPU library
target_link_libraries(cpu-sim PRIVATE cpu_lib)
target_link_libraries(monitor PRIVATE cpu_lib)
target_link_libraries(tests PRIVATE cpu_lib)

# Configure GUI target
target_include_directories(cpu-visualizer PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(cpu-visualizer PRIVATE
    cpu_lib
    SDL2::SDL2
    SDL2::SDL2main
)

# Debug helper (not installed)
add_executable(debug_lditest src/debug_lditest.c src/cpu.c src/isa.c src/memory.c src/devices.c)

# Set output directory
set_target_properties(cpu-sim asm disasm monitor tests cpu-visualizer
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build
)

# Install targets
install(TARGETS cpu-sim asm disasm monitor tests cpu-visualizer
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
    COMMAND ${CMAKE_COMMAND} -E echo "  cpu-sim       - Build CPU simulator"
    COMMAND ${CMAKE_COMMAND} -E echo "  cpu-visualizer - Build visual CPU simulator (GUI)"
    COMMAND ${CMAKE_COMMAND} -E echo "  asm           - Build assembler"
    COMMAND ${CMAKE_COMMAND} -E echo "  disasm        - Build disassembler"
    COMMAND ${CMAKE_COMMAND} -E echo "  monitor       - Build monitor/debugger"
    COMMAND ${CMAKE_COMMAND} -E echo "  tests         - Build test suite"
    COMMAND ${CMAKE_COMMAND} -E echo "  examples      - Build example programs"
    COMMAND ${CMAKE_COMMAND} -E echo "  test          - Run test suite"
    COMMAND ${CMAKE_COMMAND} -E echo "  docs          - Generate documentation"
    COMMAND ${CMAKE_COMMAND} -E echo "  help          - Show this help"
    COMMENT "Available targets"
)

# Set default target
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY DEFAULT_TARGET cpu-visualizer)

# Optionally add a Qt-based GUI if Qt is available
find_package(Qt6 COMPONENTS Widgets QUIET)
if(Qt6_FOUND)
    message(STATUS "Found Qt6: adding Qt GUI target")
    add_subdirectory(gui-qt)
else()
    find_package(Qt5 COMPONENTS Widgets QUIET)
    if(Qt5_FOUND)
        message(STATUS "Found Qt5: adding Qt GUI target")
        add_subdirectory(gui-qt)
    else()
        message(WARNING "Qt5/6 not found: Qt GUI (gui-qt) will not be built. Install Qt and reconfigure to enable the desktop GUI.")
    endif()
endif()

