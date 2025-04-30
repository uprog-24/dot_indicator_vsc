/**
 * @file    config.h
 * @brief   Этот файл позволяет задать конфигурацию: параметры для протоколов и
 *          режимов. Содержит глобальные переменные, определённые в main.c
 */
#ifndef CONFIG_H
#define CONFIG_H

#include "flash.h"

#include <stdbool.h>

#define TIME_MS_FOR_SETTINGS                                                   \
  20000 ///< Время в мс для проверки бездействия кнопок в режиме меню (20 с)

/* Протокол UIM_6100 (CAN) */
#if PROTOCOL_UIM_6100 && !DEMO_MODE && !TEST_MODE

#include "can.h"
#include "protocol_selection.h"
#include "uim6100.h"

#define PROTOCOL_NAME "SHK"
#define ADDR_ID_MIN 1
#define ADDR_ID_LIMIT 47
#define MAX_POSITIVE_NUMBER_FLOOR 40
#define MAIN_CABIN_ID UIM6100_MAIN_CABIN_CAN_ID
#define TIME_MS_FOR_INTERFACE_CONNECTION                                       \
  3000 ///< Время в мс для проверки подключения интерфейса (3 с)

#define BUFFER_SIZE_BYTES 6

/* DEMO_MODE */
#elif DEMO_MODE && !PROTOCOL_UIM_6100 && !TEST_MODE

#include "demo_mode.h"

#define BUFFER_SIZE_BYTES 1

/* TEST_MODE */
#elif TEST_MODE && !DEMO_MODE && !PROTOCOL_UIM_6100

#include "test_mode.h"

#define BUFFER_SIZE_BYTES 8

#else
#error "Wrong configurations!"
#endif

/* Общие переменные, определенные глобально в main.c */
/// Счетчики для проверки подключения интерфейса CAN/UART/DATA_Pin
extern volatile uint32_t alive_cnt[2];

/// Флаг для состояния подключения интерфейса CAN/UART/DATA_Pin
extern volatile bool is_interface_connected;

// Настройки индикатора: адрес индикатора и уровень громкости пассивного бузера
extern settings_t matrix_settings;

/// Строка для отображения на матрице
extern char matrix_string[3];

#endif // CONFIG_H