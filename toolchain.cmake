set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Some default GCC settings
# arm-none-eabi- must be part of path environment
set(TOOLCHAIN_PREFIX arm-none-eabi-)
# set(FLAGS "-fdata-sections -ffunction-sections --specs=nano.specs -Wl,--gc-sections")
set(FLAGS "-fdata-sections -ffunction-sections -Wl,--gc-sections")
set(CPP_FLAGS "${FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")

set(CMAKE_C_FLAGS ${FLAGS})
set(CMAKE_CXX_FLAGS ${CPP_FLAGS})

# Define compiler settings
# set(CMAKE_C_COMPILER C:/ST/STM32CubeCLT_1.16.0/GNU-tools-for-STM32/bin/arm-none-eabi-gcc.exe)
# set(CMAKE_CXX_COMPILER C:/ST/STM32CubeCLT_1.16.0/GNU-tools-for-STM32/bin/arm-none-eabi-g++.exe)
# set(CMAKE_ASM_COMPILER C:/ST/STM32CubeCLT_1.16.0/GNU-tools-for-STM32/bin/arm-none-eabi-gcc.exe)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc ${CMAKE_C_FLAGS})
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++ ${CMAKE_CXX_FLAGS})
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size)

set(CMAKE_EXECUTABLE_SUFFIX_ASM ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
