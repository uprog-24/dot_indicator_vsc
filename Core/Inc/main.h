/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BUTTON_1_Pin GPIO_PIN_14
#define BUTTON_1_GPIO_Port GPIOC
#define BUTTON_1_EXTI_IRQn EXTI15_10_IRQn
#define BUTTON_2_Pin GPIO_PIN_15
#define BUTTON_2_GPIO_Port GPIOC
#define BUTTON_2_EXTI_IRQn EXTI15_10_IRQn
#define ROW_2_Pin GPIO_PIN_0
#define ROW_2_GPIO_Port GPIOA
#define BUZZ_Pin GPIO_PIN_1
#define BUZZ_GPIO_Port GPIOA
#define COL_L1_Pin GPIO_PIN_2
#define COL_L1_GPIO_Port GPIOA
#define COL_L2_Pin GPIO_PIN_3
#define COL_L2_GPIO_Port GPIOA
#define COL_L3_Pin GPIO_PIN_4
#define COL_L3_GPIO_Port GPIOA
#define COL_L4_Pin GPIO_PIN_5
#define COL_L4_GPIO_Port GPIOA
#define COL_L5_Pin GPIO_PIN_6
#define COL_L5_GPIO_Port GPIOA
#define COL_R7_Pin GPIO_PIN_7
#define COL_R7_GPIO_Port GPIOA
#define COL_L6_Pin GPIO_PIN_0
#define COL_L6_GPIO_Port GPIOB
#define COL_L7_Pin GPIO_PIN_1
#define COL_L7_GPIO_Port GPIOB
#define COL_L8_Pin GPIO_PIN_2
#define COL_L8_GPIO_Port GPIOB
#define COL_R1_Pin GPIO_PIN_10
#define COL_R1_GPIO_Port GPIOB
#define COL_R2_Pin GPIO_PIN_11
#define COL_R2_GPIO_Port GPIOB
#define COL_R3_Pin GPIO_PIN_12
#define COL_R3_GPIO_Port GPIOB
#define COL_R4_Pin GPIO_PIN_13
#define COL_R4_GPIO_Port GPIOB
#define COL_R5_Pin GPIO_PIN_14
#define COL_R5_GPIO_Port GPIOB
#define COL_R6_Pin GPIO_PIN_15
#define COL_R6_GPIO_Port GPIOB
#define COL_R8_Pin GPIO_PIN_8
#define COL_R8_GPIO_Port GPIOA
#define ROW_1_Pin GPIO_PIN_15
#define ROW_1_GPIO_Port GPIOA
#define ROW_3_Pin GPIO_PIN_4
#define ROW_3_GPIO_Port GPIOB
#define ROW_4_Pin GPIO_PIN_5
#define ROW_4_GPIO_Port GPIOB
#define ROW_5_Pin GPIO_PIN_6
#define ROW_5_GPIO_Port GPIOB
#define ROW_6_Pin GPIO_PIN_7
#define ROW_6_GPIO_Port GPIOB
#define ROW_7_Pin GPIO_PIN_8
#define ROW_7_GPIO_Port GPIOB
#define ROW_8_Pin GPIO_PIN_9
#define ROW_8_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define DATA_Pin GPIO_PIN_10
#define DATA_GPIO_Port GPIOA

/**
 * Состояние включение/выключение для активного бузера и светодиода.
 */
typedef enum { TURN_OFF, TURN_ON } states_t;

/**
 * Состояния индикатора.
 */
typedef enum {
  MATRIX_STATE_START, // Запуск интерфейса для протокола
  MATRIX_STATE_WORKING, // Обработка протокола
  MATRIX_STATE_MENU     // Режим меню
} matrix_state_t;

/**
 * Состояния меню.
 */
typedef enum {
  MENU_STATE_OPEN,
  MENU_STATE_WORKING,
  MENU_STATE_CLOSE
} menu_state_t;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
