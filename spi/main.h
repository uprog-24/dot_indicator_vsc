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
#define MBI5026_SCK_PIN_Pin GPIO_PIN_5
#define MBI5026_SCK_PIN_GPIO_Port GPIOA
#define MBI5026_MOSI_PIN_Pin GPIO_PIN_7
#define MBI5026_MOSI_PIN_GPIO_Port GPIOA
#define MBI5026_NOE_PIN_Pin GPIO_PIN_8
#define MBI5026_NOE_PIN_GPIO_Port GPIOA
#define MBI5026_LE_PIN_Pin GPIO_PIN_9
#define MBI5026_LE_PIN_GPIO_Port GPIOA
#define SW_IN_3_Pin GPIO_PIN_11
#define SW_IN_3_GPIO_Port GPIOA
// #define BUZ_3_Pin GPIO_PIN_12
// #define BUZ_3_GPIO_Port GPIOA

#define BUZZ_Pin GPIO_PIN_12
#define BUZZ_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
/**
 * Stores dot/active buzzer states
 */
typedef enum { TURN_OFF, TURN_ON } states_t;

/**
 * Stores matrix states
 */
typedef enum {
  MATRIX_STATE_START,
  MATRIX_STATE_WORKING,
  MATRIX_STATE_MENU
} matrix_state_t;

/**
 * Stores menu states
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
