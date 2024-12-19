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
#define PROTOCOL_UIM_6100 1
#define PROTOCOL_UEL 0
#define PROTOCOL_UKL 0

#define PERIOD_SEC_FOR_SETTINGS \
  20  ///< Period of TIM4 (seconds) for counting time between clicks of btns in
      ///< SETTINGS mode of matrix

/* Protocol UEL (UART) */
#if PROTOCOL_UEL && !PROTOCOL_UIM_6100 && !PROTOCOL_UKL && !DEMO_MODE && \
    !TEST_MODE
#include "../source/uel.h"
#include "protocol_selection.h"
#include "usart.h"

#define PROTOCOL_NAME "UEL"
#define ADDR_ID_MIN 0
#define ADDR_ID_LIMIT 50
#define MAX_POSITIVE_NUMBER_LOCATION 39
#define MAIN_CABIN_ID 0
#define TIME_SEC_FOR_INTERFACE_CONNECTION \
  3  ///< Time in ms to check interface connection

/* Protocol UIM_6100 (CAN) */
#elif PROTOCOL_UIM_6100 && !PROTOCOL_UEL && !PROTOCOL_UKL && !DEMO_MODE && \
    !TEST_MODE
#include "can.h"
#include "protocol_selection.h"
#include "uim6100.h"

#define PROTOCOL_NAME "SHK"
#define ADDR_ID_MIN 1
#define ADDR_ID_LIMIT 49
#define MAX_POSITIVE_NUMBER_LOCATION 40
#define MAIN_CABIN_ID UIM6100_MAIN_CABIN_CAN_ID
#define TIME_SEC_FOR_INTERFACE_CONNECTION \
  3  ///< Time in ms to check interface connection

/* DEMO_MODE */
#elif DEMO_MODE && !PROTOCOL_UIM_6100 && !PROTOCOL_UEL && !PROTOCOL_UKL && \
    !TEST_MODE
#include "demo_mode.h"
#define MAX_POSITIVE_NUMBER_LOCATION 14
/* TEST_MODE */
#elif TEST_MODE && !DEMO_MODE && !PROTOCOL_UIM_6100 && !PROTOCOL_UEL && \
    !PROTOCOL_UKL
#include "can.h"
#define MAX_POSITIVE_NUMBER_LOCATION 1 /// <
/* Protocol UKL (DATA_Pin) */
#elif PROTOCOL_UKL && !PROTOCOL_UIM_6100 && !PROTOCOL_UEL && !DEMO_MODE && \
    !TEST_MODE
#include "protocol_selection.h"
#include "ukl.h"

#define PROTOCOL_NAME "UKL"
#define ADDR_ID_MIN 0
#define ADDR_ID_LIMIT 63
#define MAX_POSITIVE_NUMBER_LOCATION 55
#define MAIN_CABIN_ID 0
#define TIME_SEC_FOR_INTERFACE_CONNECTION \
  1  ///< Time in ms to check interface connection

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

#endif  // CONFIG_H
