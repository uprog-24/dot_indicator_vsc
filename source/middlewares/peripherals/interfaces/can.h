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
#define TEST_MODE_STD_ID 0x0378 ///< ID сообщения для CAN в TEST_MODE
/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */

/**
 * @brief  Запуск интерфейса CAN.
 * @note   Установка фильтра для ID, включение нотификаций для колбека.
 * @param  hcan: Указатель на структуру CAN_HandleTypeDef.
 * @retval None
 */
void start_can(CAN_HandleTypeDef *hcan, uint32_t stdId);

/**
 * @brief  Завершение работы CAN.
 * @param  hcan: Указатель на структуру CAN_HandleTypeDef.
 * @retval None
 */
void stop_can(CAN_HandleTypeDef *hcan);

/**
 * @brief  Отправка данных по CAN (для TEST_MODE, loopback).
 * @note   Если отправленные данные получены, то отобразить строку.
 * @param  stdId: ID сообщения.
 * @retval None
 */
void CAN_TxData(uint32_t stdId);

/**
 * @brief  Обработка данных, полученных по CAN.
 * @note   Если получены данные от станции управления (СУЛ), то начать обработку
 *         по протоколу.
 * @param  None
 * @retval None
 */
void process_data_from_can();

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */
