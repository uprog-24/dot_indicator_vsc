cmake_minimum_required(VERSION 3.22)

set(PROJECT_VERSION "0.1")
configure_file(config.h.in conf.h)

# Core project settings
project(dot_indicator_copy)
enable_language(C CXX ASM)

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Supported modes/protocols
set(VERSIONS
    USE_DOT_PIN
    USE_DOT_SPI
)
# Switch OFF all modes
foreach(VERSION IN LISTS VERSIONS)
    set(${VERSION} OFF CACHE BOOL "Disable ${VERSION}" FORCE)
endforeach()
# Switch ON mode defined in USE_MODE
if(DEFINED USE_VERSION AND USE_VERSION IN_LIST VERSIONS)
    set(${USE_VERSION} ON CACHE BOOL "Enable ${USE_VERSION}" FORCE)
else()
    message(FATAL_ERROR "Invalid USE_VERSION specified! Available options: ${VERSIONS}")
endif()

set(MCU_FAMILY STM32F1xx)
# set(MCU_MODEL STM32F103xx)
set(MCU_MODEL STM32F103xB)
set(CPU_PARAMETERS
    -mcpu=cortex-m3
    -mthumb
    -mfloat-abi=soft)

# set(STARTUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CubeMX/startup_stm32f103cbtx.s)
# set(MCU_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CubeMX/STM32F103CBTX_FLASH.ld)

if(USE_DOT_PIN)
    set(VERSION DOT_PIN=1)
    set(STARTUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CubeMX/pin/startup_stm32f103cbtx.s)
    set(MCU_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CubeMX/pin/STM32F103CBTX_FLASH.ld)
elseif(USE_DOT_SPI)
    set(VERSION DOT_SPI=1)
    set(STARTUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CubeMX/pin/startup_stm32f103cbtx.s)
    set(MCU_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CubeMX/pin/STM32F103CBTX_FLASH.ld)
    # set(STARTUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CubeMX/spi/startup_stm32f103cbtx.s)
    # set(MCU_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/CubeMX/spi/STM32F103CBTX_FLASH.ld)
endif()

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



if(USE_DOT_PIN)

    # Include directories
    set(CUBEMX_INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/${MCU_FAMILY}/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
        ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc
        ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc/Legacy
        ${CMAKE_CURRENT_SOURCE_DIR}/build
    )

    set(PROJECT_INCLUDE_DIRECTORIES
        ${PROJECT_DIR}/app
        ${PROJECT_DIR}/drivers

        ${PROJECT_DIR}/middlewares/display_symbols

        ${PROJECT_DIR}/middlewares/peripherals
        ${PROJECT_DIR}/middlewares/peripherals/interfaces
        ${PROJECT_DIR}/middlewares/peripherals/menu

        ${PROJECT_DIR}/middlewares/protocols_modes/alpaca
        ${PROJECT_DIR}/middlewares/protocols_modes/demo_mode
        ${PROJECT_DIR}/middlewares/protocols_modes/test_mode
        ${PROJECT_DIR}/middlewares/protocols_modes/uel
        ${PROJECT_DIR}/middlewares/protocols_modes/uim6100
        ${PROJECT_DIR}/middlewares/protocols_modes/ukl
    )

    # List of source files common for ALL MODES
    set(PROJECT_SOURCES
        ${PROJECT_DIR}/app/main.c
        ${PROJECT_DIR}/drivers/dot.c
        ${PROJECT_DIR}/middlewares/display_symbols/drawing.c
        ${PROJECT_DIR}/middlewares/display_symbols/font.c
        ${PROJECT_DIR}/middlewares/peripherals/interfaces/usart.c
        ${PROJECT_DIR}/middlewares/peripherals/interfaces/can.c
        ${PROJECT_DIR}/middlewares/peripherals/tim.c
        ${PROJECT_DIR}/middlewares/peripherals/gpio.c

        # ${PROJECT_DIR}/middlewares/peripherals/buzzer.c
        # ${PROJECT_DIR}/middlewares/peripherals/test_Buzzer.c


        # ${PROJECT_DIR}/middlewares/peripherals/menu/button.c
        ${PROJECT_DIR}/middlewares/protocols_modes/demo_mode/demo_mode.c
        ${PROJECT_DIR}/middlewares/protocols_modes/uim6100/uim6100.c
        ${PROJECT_DIR}/middlewares/protocols_modes/ukl/ukl.c
        ${PROJECT_DIR}/middlewares/protocols_modes/uel/uel.c
    )

    file(GLOB_RECURSE STM32CUBEMX_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/*.c)

elseif(USE_DOT_SPI)

    # Include directories
    set(CUBEMX_INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/${MCU_FAMILY}/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
        ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc
        ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/${MCU_FAMILY}_HAL_Driver/Inc/Legacy
        ${CMAKE_CURRENT_SOURCE_DIR}/build
    )

    set(PROJECT_INCLUDE_DIRECTORIES
        ${PROJECT_DIR}/app
        ${PROJECT_DIR}/drivers

        ${PROJECT_DIR}/middlewares/display_symbols

        ${PROJECT_DIR}/middlewares/peripherals
        ${PROJECT_DIR}/middlewares/peripherals/interfaces
        ${PROJECT_DIR}/middlewares/peripherals/menu

        ${PROJECT_DIR}/middlewares/protocols_modes/alpaca
        ${PROJECT_DIR}/middlewares/protocols_modes/demo_mode
        ${PROJECT_DIR}/middlewares/protocols_modes/test_mode
        ${PROJECT_DIR}/middlewares/protocols_modes/uel
        ${PROJECT_DIR}/middlewares/protocols_modes/uim6100
        ${PROJECT_DIR}/middlewares/protocols_modes/ukl
    )

    # List of source files common for ALL MODES
    set(PROJECT_SOURCES
        ${PROJECT_DIR}/app/main.c
        ${PROJECT_DIR}/drivers/LED_driver.c
        ${PROJECT_DIR}/middlewares/display_symbols/drawing.c
        ${PROJECT_DIR}/middlewares/display_symbols/font.c

        ${PROJECT_DIR}/middlewares/display_symbols/bitmap.c

        ${PROJECT_DIR}/middlewares/peripherals/interfaces/usart.c
        ${PROJECT_DIR}/middlewares/peripherals/interfaces/can.c
        ${PROJECT_DIR}/middlewares/peripherals/interfaces/software_SPI.c
        ${PROJECT_DIR}/middlewares/peripherals/interfaces/SysTick_Delay.c
        ${PROJECT_DIR}/middlewares/peripherals/tim.c
        ${PROJECT_DIR}/middlewares/peripherals/menu/button.c

        ${PROJECT_DIR}/middlewares/protocols_modes/uim6100/uim6100.c
        ${PROJECT_DIR}/middlewares/protocols_modes/demo_mode/demo_mode.c
        ${PROJECT_DIR}/middlewares/protocols_modes/test_mode/test_mode.c
        # ${PROJECT_DIR}/middlewares/peripherals/gpio.c
    )

    file(GLOB_RECURSE STM32CUBEMX_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/*.c)
endif()

set(COMMON_PROTOCOL_SOURCES
    ${PROJECT_DIR}/app/protocol_selection.c
    ${PROJECT_DIR}/middlewares/peripherals/flash.c
    ${PROJECT_DIR}/middlewares/peripherals/buzzer.c
    # ${PROJECT_DIR}/middlewares/peripherals/test_Buzzer.c
    ${PROJECT_DIR}/middlewares/peripherals/menu/button.c
)

# # List of source files to compile
# file(GLOB_RECURSE STM32CUBEMX_SOURCES
#     ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.c
#     ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/*.c)
# file(GLOB_RECURSE PROJECT_SOURCES FOLLOW_SYMLINKS
#     # ${PROJECT_DIR}/*.cpp
#     ${PROJECT_DIR}/*.c
# )



# Executable files
add_executable(${EXECUTABLE}
    ${STM32CUBEMX_SOURCES}
    ${COMMON_PROTOCOL_SOURCES}
    ${PROJECT_SOURCES}
    ${STARTUP_SCRIPT})

# Embedded macros(defines)
# add_definitions(-DSTM32F103xB)

target_compile_definitions(${EXECUTABLE} PRIVATE
    ${MCU_MODEL}
    ${VERSION}
    USE_HAL_DRIVER)

# Add header directories (AFTER add_executable !!)
target_include_directories(${EXECUTABLE} PRIVATE
    ${PROJECT_INCLUDE_DIRECTORIES}
    ${CUBEMX_INCLUDE_DIRECTORIES}
    # ${PROJECT_INCLUDE_DIRECTORIES}
)

# Compiler and linker options
target_compile_options(${EXECUTABLE} PRIVATE
    ${CPU_PARAMETERS}
    # -Wall
    # -Wextra
    # -Wno-unused-parameter
    $<$<COMPILE_LANGUAGE:CXX>:
    # -Wno-volatile
    # -Wold-style-cast
    # -Wuseless-cast
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


# Flash the device using openocd (--targrt flash)
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

# Clear build directory (--targrt clean-build)
set(BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build")

add_custom_target(clean-build
    # COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_DIR}/* # Удаляем все файлы в каталоге
    # COMMAND ${CMAKE_COMMAND} -E remove ${BUILD_DIR}/.* # Удаляем все скрытые файлы (например, .ninja, .cmake)

    COMMAND rm -rf ${BUILD_DIR}/*
    COMMAND rm -rf ${BUILD_DIR}/.*

    COMMENT "Remove build files"
)
