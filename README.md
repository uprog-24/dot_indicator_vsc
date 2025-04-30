# Точечный индикатор

## Описание

Краткое описание проекта, его назначения и функциональности.

<h2 id="custom-id">Сборка проекта</h2>

Проект собирается с помощью `CMake`.
Команды выполняются из корневой папки проекта.
Файлы сборки сохраняются в папку build (создается в корневой папке при запуске команды 1).

1. Конфигурация и генерация файлов:

```sh
$ cmake -G "Ninja" -B build
```

2. Сборка исполняемого файла:

```sh
$ cmake --build ./build
```

3. Загрузка файла .elf через STLink (openocd):

```sh
$ cmake --build ./build --target flash
```

4. Очистка папки сборки build: cmake --build ./build --target clean-build
   Или удаление папки: rm -rf build.
5. Rebuild вместо п. 4: cmake --build ./build --clean-first -v -j 8

## Структура проекта

```sh
/project-root
│── /docs
│ ├── /arch
│ │ │── arch_ver_0.1.drawio.png
│ ├── /board
│ │ │── Reference_manual-stm32f103xx.pdf
│ │ │── Schematic_Prints_for_Dot_Uni_v3_CPM.PDF
│ ├── /SDK
│ │ │── file.txt
│ ├── README.md
│── /source
│ ├── /app
│ │ │── config.h
│ │ │── main.c
│ │ │── protocol_selection.h
│ │ │── protocol_selection.c
│ ├── /drivers
│ │ │── dot.h
│ │ │── dot.c
│ ├── /middlewares # Вспомогательные функции
│ │ │── /display_symbols
│ │ │── /peripherals
│ │ │── /protocols_modes
├── .gitignore
├── CMakeLists.txt
├── confug.h.in
├── CMakeLists.txt
├── gcc-arm-none-eabi.cmake
```

## 📂 **[/docs](./docs/)**

Содержит документацию проекта.

- 📂 **[arch](./docs/arch/)** — блок-схема архитектуры ПО.
  - 🖼️ **[arch_ver_0.1.drawio.png](./docs/arch/arch_ver_0.1.drawio.png)** — Версия ПО 0.1.
- 📂 **[board](./docs/board/)** — документация по аппаратной части.
  - 📂 **[/buzzer](./docs/board/buzzer/)** <br>
    📄 **[HC0905A.pdf](./docs/board/buzzer/HC0905A.pdf)** — документация на пассивный бузер.
  - 📂 **[/mcu](./docs/board/mcu/)** <br>
    📄 **[Reference_manual-stm32f103xx.pdf](./docs/board/mcu/Reference_manual-stm32f103xx.pdf)** — документация на микроконторллер stm32f103cbt.
  - 📄 **[Schematic_Prints_for_Dot_Uni_v3_CPM.PDF](./docs/board/Schematic_Prints_for_Dot_Uni_v3_CPM.PDF)** — электрическая схема платы Dot_Uni_v3_CPM.
- 📂 **[SDK](./docs/SDK/)**
  - 📄 **[sdk_versions.txt](./docs/SDK/sdk_versions.txt)** — версии CubeIDE, CubeMX, HAL для файла с настройками периферии **[dot_indicator.ioc](./CubeMX/dot_indicator.ioc)**.

## 📂 **[/source/app](./source/app/)**

<!-- ## 📂 **[/source/app](../source/app/)** <br> -->

Cодержит бизнес-логику программы.

- 📄 **[config.h](../source/app/config.h)** предназначен для задания параметров для выбранного протокола/режима, который указывается при [сборке проекта](#custom-id);
- 📄 **[main.c](../source/app/main.c)** содержит логику работы программы;
- 📄 <a id="source-app"></a> **[protocol_selection.h](../source/app/protocol_selection.h)** интерфейс для работы с протоколом;
- 📄 **[protocol_selection.c](../source/app/protocol_selection.c)** реализация методов интерфейса [protocol_selection.h](#source-app)

## 📂 **[/drivers](./source/drivers/)** <br>

Содержит драйвер для точечной матрицы.

- **[dot.c](../source/drivers/dot.c)**
- **[dot.h](../source/drivers/dot.h)**

## 📂 **[/middlewares](./source/middlewares/)** содержит: <br>

- 📂 **[display_symbols](./source/middlewares/display_symbols/)**
- 📂 **[peripherals](./source/middlewares/peripherals/)**
- 📂 **[protocols_modes](./source/middlewares/protocols_modes/protocols_modes.md)**
