cmake_minimum_required(VERSION 3.10)
project(SoftwareCPU_GuiQt LANGUAGES CXX)

if(TARGET cpu_lib)
    set(CPU_LIB cpu_lib)
else()
    message(FATAL_ERROR "cpu_lib target not found; ensure top-level CMakeLists defines cpu_lib before adding gui-qt")
endif()

find_package(Qt6 COMPONENTS Widgets REQUIRED QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    set(QT_LIBS Qt5::Widgets)
else()
    set(QT_LIBS Qt6::Widgets)
endif()

set(SRC
    main.cpp
    MainWindow.cpp
    MainWindow.h
    CPUBridge.cpp
    CPUBridge.h
)

add_executable(cpu-qt-visualizer ${SRC})
set_target_properties(cpu-qt-visualizer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build
)

target_include_directories(cpu-qt-visualizer PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(cpu-qt-visualizer PRIVATE ${CPU_LIB} ${QT_LIBS})

# Enable automoc for Qt headers
set_target_properties(cpu-qt-visualizer PROPERTIES
    AUTOMOC TRUE
    AUTOUIC TRUE
    AUTORCC TRUE
)
