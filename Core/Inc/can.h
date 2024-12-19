/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    can.h
 * @brief   This file contains all the function prototypes for
 *          the can.c file
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
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdint.h>
/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN Private defines */
#define TEST_MODE_STD_ID 0x0378  ///< ID for CAN in TEST_MODE
/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */

/**
 * @brief  Start CAN
 * @note   Set filter for frame ID, activate notifications for
 *         interrupt callback
 * @param  hcan: Pointer to the CAN_HandleTypeDef structure
 * @retval None
 */
void start_can(CAN_HandleTypeDef *hcan, uint32_t stdId);

/**
 * @brief  Stop CAN
 * @param  hcan: Pointer to the CAN_HandleTypeDef structure
 * @retval None
 */
void stop_can(CAN_HandleTypeDef *hcan);

/**
 * @brief  Transmit data by CAN.
 * @note   If transmitted data is received then set symbols to matrix
 * @param  stdId: Standard ID of frame
 * @retval None
 */
void CAN_TxData(uint32_t stdId);

/**
 * @brief  Process data received by CAN.
 * @note   If transmitted data by UIM6100 protocol is received then process data
 * @param  None
 * @retval None
 */
void process_data_from_can();

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */
