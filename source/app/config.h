/**
 * @file    config.h
 * @brief   This file allows to set configuration: select protocol
 *          (UIM_6100/UEL/UKL) or TEST_MODE/DEMO_MODE. There are global
 * variables that defined in main.c
 */
#ifndef CONFIG_H
#define CONFIG_H

#include "flash.h"

#include <stdbool.h>

/* Select protocol, demo/test mode */
#define TEST_MODE 0
#define DEMO_MODE 0
#define PROTOCOL_UIM_6100 0
#define PROTOCOL_UEL 1
#define PROTOCOL_UKL 0
#define PROTOCOL_ALPACA 0
#if DOT_PIN
#define PERIOD_SEC_FOR_SETTINGS                                                \
  20000 ///< Period of TIM4 (seconds) for counting time between clicks of btns
        ///< in
        ///< SETTINGS mode of matrix

#elif DOT_SPI

#include "can.h"

#define PERIOD_SEC_FOR_SETTINGS                                                \
  10000 ///< Period of TIM4 (seconds) for counting time between clicks of btns
        ///< in
///< SETTINGS mode of matrix

#define config_MU_IT_04_10
//#define config_MU_IT_05_10
//#define config_MU_IT_06_10

#if defined(config_MU_IT_04_10) || defined(config_MU_IT_05_10)
#define config_SPLIT_SYMBOL
#endif

#define __BITMAP_STANDART // стандартный bitmap
// выбор стрелки (направление движения при остановке)
//#define __ARROW_ORDINAR
#define __ARROW_DOUBLE

#endif
/* Protocol UEL (UART) */
#if PROTOCOL_UEL && !PROTOCOL_UIM_6100 && !PROTOCOL_UKL && !PROTOCOL_ALPACA && \
    !DEMO_MODE && !TEST_MODE

#include "protocol_selection.h"
#include "uel.h"
#include "usart.h"

#define PROTOCOL_NAME "UEL"
#define ADDR_ID_MIN 0
#define ADDR_ID_LIMIT 46
#define MAX_POSITIVE_NUMBER_LOCATION 39
#define MAIN_CABIN_ID ADDR_ID_MIN
#define TIME_SEC_FOR_INTERFACE_CONNECTION                                      \
  3000 ///< Time in ms to check interface connection

#define TIME_DISPLAY_STRING_DURING_MS                                          \
  2000 ///< Время в мс, в течение которого отображается строка

/* Protocol UIM_6100 (CAN) */
#elif PROTOCOL_UIM_6100 && !PROTOCOL_UEL && !PROTOCOL_UKL &&                   \
    !PROTOCOL_ALPACA && !DEMO_MODE && !TEST_MODE

#include "can.h"
#include "protocol_selection.h"
#include "uim6100.h"

#define PROTOCOL_NAME "SHK"
#define ADDR_ID_MIN 1
#define ADDR_ID_LIMIT 47
#define MAX_POSITIVE_NUMBER_LOCATION 40
#define MAIN_CABIN_ID UIM6100_MAIN_CABIN_CAN_ID
#define TIME_SEC_FOR_INTERFACE_CONNECTION                                      \
  3000 ///< Time in ms to check interface connection

#if DOT_SPI
#define config_MU_IT_04_10
//#define config_MU_IT_05_10
//#define config_MU_IT_06_10
/*###################### выбор станции ######################*/
//#define SHK6000
//#define DEMO
// #define DEMO

// #if defined(SHK6000)
// #include "SHK6000_config.h"
// #include "SHK6000_menu.h"
// #include "SHK6000_protocol.h"
// #define MAX_FLOOR 40
// #define CAN_USED
// #elif defined(UEL)
// #include "UEL_config.h"
// #include "UEL_menu.h"
// #include "UEL_protocol.h"
// #define MAX_FLOOR 39
// #define MAX_FLOOR_CODE 50

// #elif defined(DEMO)
// #define MAX_FLOOR 40
// #define NUM_OF_P_UNDERFLOORS
// #define NUM_OF_MINUS_UNDERFLOORS
// #endif

#if defined(config_MU_IT_04_10) || defined(config_MU_IT_05_10)
#define config_SPLIT_SYMBOL
#endif

#define __BITMAP_STANDART // стандартный bitmap
// выбор стрелки (направление движения при остановке)
//#define __ARROW_ORDINAR
#define __ARROW_DOUBLE
#endif

/* DEMO_MODE */
#elif DEMO_MODE && !PROTOCOL_UIM_6100 && !PROTOCOL_UEL && !PROTOCOL_UKL &&     \
    !PROTOCOL_ALPACA && !TEST_MODE

#include "demo_mode.h"

#if DOT_PIN
#define MAX_POSITIVE_NUMBER_LOCATION 14
#define ADDR_ID_MIN 1
#define ADDR_ID_LIMIT 14
#define MAIN_CABIN_ID 1
#elif DOT_SPI

#define MAX_POSITIVE_NUMBER_LOCATION 14
#define ADDR_ID_MIN 1
#define ADDR_ID_LIMIT 14
#define MAIN_CABIN_ID 1

#define config_MU_IT_04_10
//#define config_MU_IT_05_10
//#define config_MU_IT_06_10
/*###################### выбор станции ######################*/
//#define SHK6000
//#define DEMO
#define DEMO

#if defined(SHK6000)
#include "SHK6000_config.h"
#include "SHK6000_menu.h"
#include "SHK6000_protocol.h"
#define MAX_FLOOR 40
#define CAN_USED
#elif defined(UEL)
#include "UEL_config.h"
#include "UEL_menu.h"
#include "UEL_protocol.h"
#define MAX_FLOOR 39
#define MAX_FLOOR_CODE 50

#elif defined(DEMO)
#define MAX_FLOOR 40
#define NUM_OF_P_UNDERFLOORS
#define NUM_OF_MINUS_UNDERFLOORS
#endif

#if defined(config_MU_IT_04_10) || defined(config_MU_IT_05_10)
#define config_SPLIT_SYMBOL
#endif

#define __BITMAP_STANDART // стандартный bitmap
// выбор стрелки (направление движения при остановке)
//#define __ARROW_ORDINAR
#define __ARROW_DOUBLE
#endif

#else
#error "Wrong configurations!"
#endif

/* Variables defined globally in main.c */
/// Counters to control CAN/UART/DATA_Pin connection
extern volatile uint32_t alive_cnt[2];

/// Flag to control if CAN/UART/DATA_Pin is connected
extern volatile bool is_interface_connected;

// Settings of matrix: addr_id of matrix and level of volume for buzzer
extern settings_t matrix_settings;

/// String that will be displayed on matrix
extern char matrix_string[3];

#endif // CONFIG_H
