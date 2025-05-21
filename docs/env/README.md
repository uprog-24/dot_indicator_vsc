# Сборка проекта

## **[<- Вернуться назад](../README.md)**

Команды выполняются из корневой папки проекта.
Файлы сборки сохраняются в папку **_build_** (создается в корневой папке при запуске команды 1).

[Руководство по сборке](https://github.com/MaJerle/stm32-cube-cmake-vscode?tab=readme-ov-file).

### В сборке используется:

1. Система сборки [`CMake`](https://cmake.org/) + генератор [`Ninja`](https://github.com/ninja-build/ninja/releases). <br>
2. `STM32CubeCLT`: набор утилит от STMicroelectronics. Содержит инструменты `CMake`, `Ninja`, `GCC`, <br>
   `GNU-tools-for-STM32` компилятор ARM GCC (arm-none-eabi-gcc) и сопутствующие утилиты (objcopy, gdb, as, ld), <br>
   `STLInk-gdb-server` для Build & Debug в [launch.json](./.vscode/launch.json), <br>
   `STM32CubeProgrammer` для команды прошивки в [tasks.json](./.vscode/tasks.json).

`!!! все инструменты должны быть добавлены в системную переменную PATH (автоматически для STM32CubeCLT) !!! ` <br>

### Этапы сборки (команды для терминала)

1. Конфигурация и генерация файлов. Необходимо задать одно из доступных значений для **_-DUSE_MODE_**. <br>

- USE_PROTOCOL_NKU_SD7 - протокол НКУ-SD7;
- USE_DEMO_MODE - демонстрационный режим;
- USE_TEST_MODE - тестовый режим.

```sh
$ cmake -G "Ninja" -DUSE_MODE=MODE -B build
```

2. Сборка исполняемого файла в папку **_build_**:

```sh
$ cmake --build ./build
```

Файлы и команды сборки записываются в файл `compile_commands.json` в папке **_build_**.

3. Загрузка файла .elf, прошивка устройства через STLink ([OpenOCD for Windows](https://gnutoolchains.com/arm-eabi/openocd/)):

```sh
$ cmake --build ./build --target flash
```

`!!! openocd должен быть добавлен в системную переменную PATH !!! ` <br>
Команда `--target flash` определена в [CMakeLists.txt](./CMakeLists.txt).
