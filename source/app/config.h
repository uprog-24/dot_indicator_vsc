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

/* Протокол NKU_SD7 */
#if PROTOCOL_NKU_SD7 && !DEMO_MODE && !TEST_MODE

#include "nku_sd7.h"
#include "protocol_selection.h"

#define DINAMIC_ARROW 0 // 1: >> => и << <=; 0: => <=

#define PROTOCOL_NAME "SD7"
#define ADDR_ID_MIN 0
#define ADDR_ID_LIMIT 40
#define MAIN_CABIN_ID ADDR_ID_MIN
#define GROUP_ID_MIN 0
#define GROUP_ID_MAX 0
#define TIME_MS_FOR_INTERFACE_CONNECTION                                       \
  3000 ///< Время в мс для проверки подключения интерфейса (3 с)

#define TIME_DISPLAY_STRING_DURING_MS                                          \
  3000 ///< Время в мс, в течение которого отображается строка при подаче
       ///< питания

#define SOUND_ON_OFF_DURING_MS                                                 \
  500 ///< Периодичность сигнала при Перегрузке (Вкл/Выкл)

#define BUFFER_SIZE_BYTES 6

#if DINAMIC_ARROW
#define CYCLE_ANIMATION_MS 150

#else
#define CYCLE_ANIMATION_MS 60 // 50 // 60 // 150

#endif

#define ARROW_DOUBLE
// #define ARROW_ORDINAR

#elif DEMO_MODE && !PROTOCOL_NKU_SD7 && !TEST_MODE

#include "demo_mode.h"

#define ARROW_DOUBLE
// #define ARROW_ORDINAR

#define TIME_DISPLAY_STRING_DURING_MS                                          \
  2000 ///< Время в мс, в течение которого отображается строка
#define BUFFER_SIZE_BYTES 1

/* TEST_MODE */
#elif TEST_MODE && !DEMO_MODE && !PROTOCOL_NKU_SD7

#include "test_mode.h"

#define CYCLE_ANIMATION_MS 150
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

#endif // CONFIG_H