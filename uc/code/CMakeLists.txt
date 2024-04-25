cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(PI_ContrHorlo C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

list(APPEND PICO_BOARD_HEADER_DIRS ${CMAKE_CURRENT_LIST_DIR})

pico_sdk_init()

file(GLOB SRC *.cpp)
add_executable(${PROJECT_NAME} ${SRC})

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(${PROJECT_NAME} pico_stdlib pico_multicore)

pico_add_extra_outputs(${PROJECT_NAME})
