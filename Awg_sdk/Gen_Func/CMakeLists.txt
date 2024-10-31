# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Gen_Func C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Gen_Func 
    src/qm/main.c 
    src/awg.c
    )

pico_set_program_name(Gen_Func "Gen_Func")
pico_set_program_version(Gen_Func "0.1")

# Generate PIO header
pico_generate_pio_header(Gen_Func ${CMAKE_CURRENT_LIST_DIR}/blink.pio)

# Directorios para los headers
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/qpc/ports/freertos)
include_directories(${CMAKE_SOURCE_DIR}/qpc/include)
include_directories(${CMAKE_SOURCE_DIR}/src/qm)
include_directories(${CMAKE_SOURCE_DIR}/qpc/src/qf)
include_directories(${CMAKE_SOURCE_DIR}/qpc/src/qs)
#include_directories(quantum_leaps/qv)

# Agrega todos los archivos de Quantum Leaps al ejecutable
file(GLOB_RECURSE QP_SOURCES
    qpc/src/qf/*.c
    src/qm/*.c
    qpc/ports/freertos/*.c
    qpc/src/qs/*.c
)

list(FILTER QP_SOURCES EXCLUDE REGEX ".*qs.*")

file(GLOB QM_SOURCES src/qm/*.c)

target_sources(Gen_Func PRIVATE ${QP_SOURCES})

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Gen_Func 0)
pico_enable_stdio_usb(Gen_Func 1)

# Add the standard library to the build
target_link_libraries(Gen_Func
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(Gen_Func PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# FreeRTOS paths
set(FREERTOS_PATH ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel)
include_directories(${FREERTOS_PATH}/include)
include_directories(${FREERTOS_PATH}/portable/GCC/ARM_CM0)
include_directories(${FREERTOS_PATH}/portable/ThirdParty/GCC/RP2040/include)

# Add FreeRTOS source files
target_sources(Gen_Func PRIVATE
  ${FREERTOS_PATH}/croutine.c
  ${FREERTOS_PATH}/list.c
  ${FREERTOS_PATH}/queue.c
  ${FREERTOS_PATH}/tasks.c
  ${FREERTOS_PATH}/timers.c
  ${FREERTOS_PATH}/event_groups.c
  ${FREERTOS_PATH}/portable/MemMang/heap_4.c
  ${FREERTOS_PATH}/portable/GCC/ARM_CM0/port.c
  ${FREERTOS_PATH}/portable/GCC/ARM_CM0/portasm.c
  ${FREERTOS_PATH}/portable/ThirdParty/GCC/RP2040/port.c
)

# Link FreeRTOS to your project
target_link_libraries(Gen_Func 
        hardware_spi
        hardware_i2c
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_watchdog
        hardware_clocks
        )

pico_add_extra_outputs(Gen_Func)
