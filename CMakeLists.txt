cmake_minimum_required(VERSION 3.22)

set(PROJECT_VERSION "0.1")
configure_file(config.h.in conf.h)

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)

# Supported protocols
option(USE_PROTOCOL_UIM6100 "Use protocol UIM_6100 (CAN)" OFF)
option(USE_PROTOCOL_UEL "Use protocol UEL (UART)" OFF)
option(USE_PROTOCOL_UKL "Use protocol UKL (DATA_PIN)" OFF)
option(USE_PROTOCOL_ALPACA "Use protocol ALPACA (CAN)" OFF)
option(USE_DEMO_MODE "Use DEMO mode" OFF)
option(USE_TEST_MODE "Use TEST mode" OFF)

# # Include protocol-specific configurations

# if(USE_PROTOCOL_UIM6100)
#     add_subdirectory(${PROJECT_DIR}/middlewares/protocols_modes/uim6100)
# elseif(USE_PROTOCOL_UEL)
#     add_subdirectory(${PROJECT_DIR}/middlewares/protocols_modes/uel)
# elseif(USE_PROTOCOL_UKL)
#     add_subdirectory(${PROJECT_DIR}/middlewares/protocols_modes/ukl)
# elseif(USE_PROTOCOL_ALPACA)
#     add_subdirectory(${PROJECT_DIR}/middlewares/protocols_modes/alpaca)
# elseif(USE_DEMO_MODE)
#     message(STATUS "Demo mode is enabled")
#     add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/middlewares/protocols_modes/demo_mode)
# elseif(USE_TEST_MODE)
#     add_subdirectory(${PROJECT_DIR}/middlewares/protocols_modes/test_mode)
# else()
#     message(FATAL_ERROR "No valid protocol selected!")
# endif()

# Core project settings
project(dot_indicator_copy)
enable_language(C CXX ASM)



# set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/build
)
set(PROJECT_INCLUDE_DIRECTORIES
    ${PROJECT_DIR}/app
    ${PROJECT_DIR}/drivers

    ${PROJECT_DIR}/middlewares/display_symbols

    ${PROJECT_DIR}/middlewares/peripherals
    ${PROJECT_DIR}/middlewares/peripherals/interfaces
    ${PROJECT_DIR}/middlewares/peripherals/menu

    # ${PROJECT_DIR}/middlewares/protocols_modes/alpaca
    # ${PROJECT_DIR}/middlewares/protocols_modes/demo_mode
    # ${PROJECT_DIR}/middlewares/protocols_modes/test_mode
    # ${PROJECT_DIR}/middlewares/protocols_modes/uel
    # ${PROJECT_DIR}/middlewares/protocols_modes/uim6100
    # ${PROJECT_DIR}/middlewares/protocols_modes/ukl
)

# # List of source files to compile
file(GLOB_RECURSE STM32CUBEMX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/*.c)
# file(GLOB_RECURSE PROJECT_SOURCES FOLLOW_SYMLINKS
#     # ${PROJECT_DIR}/*.cpp
#     ${PROJECT_DIR}/*.c

#     # ${PROJECT_DIR}/app/main.c
#     # ${PROJECT_DIR}/drivers/*.c
#     # ${PROJECT_DIR}/middlewares/display_symbols/*.c
#     # ${PROJECT_DIR}/middlewares/peripherals/*.c

# )

set(PROJECT_SOURCES
    ${PROJECT_DIR}/app/main.c
    ${PROJECT_DIR}/drivers/dot.c
    ${PROJECT_DIR}/middlewares/display_symbols/drawing.c
    ${PROJECT_DIR}/middlewares/display_symbols/font.c
    ${PROJECT_DIR}/middlewares/peripherals/interfaces/usart.c
    ${PROJECT_DIR}/middlewares/peripherals/interfaces/can.c
    ${PROJECT_DIR}/middlewares/peripherals/tim.c
    ${PROJECT_DIR}/middlewares/peripherals/gpio.c
    ${PROJECT_DIR}/middlewares/peripherals/test_Buzzer.c
    ${PROJECT_DIR}/middlewares/protocols_modes/demo_mode/demo_mode.c
)


# Include protocol-specific configurations
if(USE_PROTOCOL_UIM6100)
    add_subdirectory(${PROJECT_DIR}/middlewares/protocols_modes/uim6100)
elseif(USE_PROTOCOL_UEL)
    add_subdirectory(${PROJECT_DIR}/middlewares/protocols_modes/uel)
elseif(USE_PROTOCOL_UKL)
    add_subdirectory(${PROJECT_DIR}/middlewares/protocols_modes/ukl)
elseif(USE_PROTOCOL_ALPACA)
    add_subdirectory(${PROJECT_DIR}/middlewares/protocols_modes/alpaca)
elseif(USE_DEMO_MODE)
    message(STATUS "Demo mode is enabled")
    # add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/middlewares/protocols_modes/demo_mode)

    add_library(demo_lib ${PROJECT_DIR}/middlewares/protocols_modes/demo_mode/demo_mode.c)
    target_include_directories(demo_lib PUBLIC
        ${CUBEMX_INCLUDE_DIRECTORIES}
        ${PROJECT_DIR}/middlewares/protocols_modes/demo_mode)

    target_compile_definitions(demo_lib PUBLIC
        ${MCU_MODEL}
        DEMO_MODE=1 USE_HAL_DRIVER)

elseif(USE_TEST_MODE)
    add_subdirectory(${PROJECT_DIR}/middlewares/protocols_modes/test_mode)
else()
    message(FATAL_ERROR "No valid protocol selected!")
endif()

add_executable(${EXECUTABLE}
    ${STM32CUBEMX_SOURCES}
    ${PROJECT_SOURCES}
    ${STARTUP_SCRIPT})

# target_link_libraries(${EXECUTABLE} demo_mode)
target_link_libraries(${EXECUTABLE} PUBLIC demo_lib)


target_compile_definitions(${EXECUTABLE} PRIVATE
    ${MCU_MODEL}
    USE_HAL_DRIVER)

# Add header directories (AFTER add_executable !!)
target_include_directories(${EXECUTABLE} PRIVATE
    ${PROJECT_INCLUDE_DIRECTORIES}
    ${CUBEMX_INCLUDE_DIRECTORIES}
    ${PROJECT_DIR}/middlewares/protocols_modes/demo_mode
    # ${PROJECT_INCLUDE_DIRECTORIES}
)


# message(STATUS "STM32CUBEMX Sources: ${STM32CUBEMX_SOURCES}")
# message(STATUS "Project Sources: ${PROJECT_SOURCES}")


# Compiler and linker options
target_compile_options(${EXECUTABLE} PRIVATE
    ${CPU_PARAMETERS}
    # -Wall
    # -Wextra
    # -Wno-unused-parameter
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
