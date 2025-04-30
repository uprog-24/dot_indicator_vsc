set(CMAKE_SYSTEM_NAME Generic) # Определяем систему как Generic (не конкретная ОС)
set(CMAKE_SYSTEM_PROCESSOR arm) # Определяем процессор ARM

# Настройка префикса компилятора (компилятор должен находиться в PATH, либо можно задать полный путь)
set(TOOLCHAIN_PREFIX arm-none-eabi-)

# Флаги компиляции
set(COMMON_FLAGS "-fdata-sections -ffunction-sections --specs=nano.specs -Wl,--gc-sections")
set(CMAKE_C_FLAGS ${COMMON_FLAGS})
set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")

# Define compiler settings
# set(CMAKE_C_COMPILER C:/ST/STM32CubeCLT_1.16.0/GNU-tools-for-STM32/bin/arm-none-eabi-gcc.exe)

# Назначение компиляторов (флаги компиляции отдельно)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)

set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy) # Нужен для создания .bin и .hex файлов
set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}size) # Показывает размер секций ELF-файла

# Расширения выходных файлов
set(CMAKE_EXECUTABLE_SUFFIX_ASM ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")

# Отключаем попытку запускать исполняемые файлы при проверке компиляции
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
