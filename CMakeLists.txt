cmake_minimum_required(VERSION 3.22)

# Core project settings
project(dot_indicator_copy)
enable_language(C CXX ASM)

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)

set(MCU_FAMILY STM32F1xx)
# set(MCU_MODEL STM32F103xx)
set(MCU_MODEL STM32F103xB)
set(CPU_PARAMETERS
    -mcpu=cortex-m3
    -mthumb
    -mfloat-abi=soft)

set(STARTUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CubeMX/startup_stm32f103cbtx.s)
set(MCU_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CubeMX/STM32F103CBTX_FLASH.ld)
# ##############################################################################
# set(EXECUTABLE ${CMAKE_PROJECT_NAME}.elf)
set(EXECUTABLE ${CMAKE_PROJECT_NAME})

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Include directories
set(CUBEMX_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/${MCU_FAMILY}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc/Legacy
)
set(PROJECT_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_DIR})

# # List of source files to compile
file(GLOB_RECURSE STM32CUBEMX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/*.c)
file(GLOB_RECURSE PROJECT_SOURCES FOLLOW_SYMLINKS
    ${PROJECT_DIR}/*.cpp
    ${PROJECT_DIR}/*.c)

# Executable files
add_executable(${EXECUTABLE}
    ${STM32CUBEMX_SOURCES}
    ${PROJECT_SOURCES}
    ${STARTUP_SCRIPT})

# Embedded macros(defines)
# add_definitions(-DSTM32F103xB)

target_compile_definitions(${EXECUTABLE} PRIVATE
    ${MCU_MODEL}
    USE_HAL_DRIVER)

# Add header directories (AFTER add_executable !!)
target_include_directories(${EXECUTABLE} PRIVATE
    ${CUBEMX_INCLUDE_DIRECTORIES}
    ${PROJECT_INCLUDE_DIRECTORIES})

# Compiler and linker options
target_compile_options(${EXECUTABLE} PRIVATE
    ${CPU_PARAMETERS}
    -Wall
    -Wextra
    -Wno-unused-parameter
    $<$<COMPILE_LANGUAGE:CXX>:
    -Wno-volatile
    -Wold-style-cast
    -Wuseless-cast
    -Wsuggest-override>
    $<$<CONFIG:Debug>:-Og -g3 -ggdb>
    $<$<CONFIG:Release>:-Og -g0>)

# Linker options
target_link_options(${EXECUTABLE} PRIVATE
    -T${MCU_LINKER_SCRIPT}
    ${CPU_PARAMETERS}
    -Wl,-Map=${CMAKE_PROJECT_NAME}.map
    --specs=nosys.specs
    -Wl,--start-group
    -lc
    -lm
    -lstdc++
    -lsupc++
    -Wl,--end-group
    -Wl,--print-memory-usage)

# Execute post-build to print size    
add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${EXECUTABLE}>)

# Convert output to hex and binary
add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${EXECUTABLE}>
    ${EXECUTABLE}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${EXECUTABLE}>
    ${EXECUTABLE}.bin)


# The command to flash the device using openocd
set(OPENOCD_PATH "C:/Program Files (x86)/OpenOCD-20240916-0.12.0")
set(OPENOCD_INTERFACE_PATH "${OPENOCD_PATH}/share/openocd/scripts/interface/stlink.cfg")
set(OPENOCD_TARGET_PATH "${OPENOCD_PATH}/share/openocd/scripts/target/stm32f1x.cfg")
set(FLASH_COMMAND "openocd")
set(FLASH_OPTIONS "-f \"${OPENOCD_INTERFACE_PATH}\" -f \"${OPENOCD_TARGET_PATH}\"")

add_custom_target(flash
    COMMAND ${FLASH_COMMAND} ${FLASH_OPTIONS} -c "program ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.elf verify reset exit"
    DEPENDS ${PROJECT_NAME}.elf
    COMMENT "Flashing STM32 device with ST-Link"
)

# Указываем путь к каталогу сборки
set(BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build")

# Удалить все содержимое папки сборки, но не саму папку
add_custom_target(clean-build
    # COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_DIR}/* # Удаляем все файлы в каталоге
    # COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_DIR}/.* # Удаляем все скрытые файлы (например, .ninja, .cmake)

    COMMAND rm -rf ${BUILD_DIR}/*
    COMMAND rm -rf ${BUILD_DIR}/.*

    COMMENT "Remove build files"
)
