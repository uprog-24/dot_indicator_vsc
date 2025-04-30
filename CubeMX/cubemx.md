# Сгенерированные файлы CubeMX

## **[<- Вернуться назад](../README.md)**

## Файлы CubeMX расположены в 📂 **[CubeMX](../CubeMX/)**

- 📄 **[dot_indicator.ioc](./dot_indicator.ioc)** - главный файл конфигурации проекта. Содержит описание настроек микроконтроллера: тактирование, пины, периферия (GPIO, UART, Timers и т.д.). Используется STM32CubeMX для открытия конфигурации МК.

- 📄 **[startup_stm32f103cbtx.s](./startup_stm32f103cbtx.s)** - файл запуска (startup file).
  Выполняет инициализацию стека, таблицу векторов прерываний.

- 📄 **[STM32F103CBTX_FLASH.ld](./STM32F103CBTX_FLASH.ld)** - линкер скрипт (linker script). Определяет структуру памяти: где в памяти будет размещён .text (код), .data, .bss, стек и т.д. Используется компилятором arm-none-eabi-gcc на этапе линковки. Важен для прошивки: правильно распределяет секции по flash и RAM.

В файле определена секция для сохранения настроек индикатора во flash-памяти:

```ld
 SETTINGS (rx)		: ORIGIN = 0x0801FC00,	LENGTH = 1K

...

 .settings (NOLOAD) :
  {
	. = ALIGN(4);
	__SETTINGS_SECTION_START = .;
	KEEP(*(.settings))
	. = ALIGN(4);
	__SETTINGS_SECTION_END = .;
  } > SETTINGS

```

Параметр `NOLOAD` необходим для сохранения настроек при перезагрузке устройства.
