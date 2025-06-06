/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    can.c
 * @brief   This file provides code for the configuration
 *          of the CAN instances.
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
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
#include "config.h"

#if PROTOCOL_UIM_6100
msg_t msg = {0, 0, 0, 0};
#endif

#include <stdbool.h>
#include <stdio.h>

#define FILTER_11_BIT_ID_OFFSET                                                \
  5 ///< Смещение для стандартного фильтра идентификации кадра.

/// Структура заголовка для получения данных.
static CAN_RxHeaderTypeDef rx_header;

/// Буфер для полученных данных.
static uint8_t rx_data_can[BUFFER_SIZE_BYTES] = {
    0x00,
};

/// Флаг для контроля полученных данных по CAN.
volatile bool is_data_received = false;

/// Структура заголовка для отправленных данных.
static CAN_TxHeaderTypeDef tx_header;

/**
 * @brief Отправка сообщений-ответов по CAN для PROTOCOL_UIM_6100.
 *
 * @param stdId:  Адрес, на который отправляется ответ.
 * @param dlc:    Размер сообщения в байтах.
 * @param buffer: Указатель на буфер с данными для ответа.
 */
static void can_send_answer(uint32_t stdId, uint8_t dlc, uint8_t *buffer) {
  tx_header.StdId = stdId;
  tx_header.ExtId = 0;
  tx_header.RTR = CAN_RTR_DATA;
  tx_header.IDE = CAN_ID_STD;
  tx_header.DLC = dlc;
  tx_header.TransmitGlobalTime = 0;

  uint32_t tx_mailbox = 0;

  if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 0) {
    HAL_CAN_AddTxMessage(&hcan, &tx_header, buffer, &tx_mailbox);
  }
}

/**
 * @brief  Обработка прерывания: получение данных по CAN.
 *         Устанавливаем флаг is_data_received при получении данных.
 *         Для протоколов: устанавливаем счетчик alive_cnt[0] для проверки
 *         подключения интерфейса (устанавливаем alive_cnt[1] в tim.c TIM4).
 * @param  hcan: Указатель на структуру CAN_HandleTypeDef.
 * @retval None
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {

  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data_can) ==
      HAL_OK) {

#if PROTOCOL_UIM_6100

    if ((matrix_settings.addr_id == rx_header.StdId) &&
    // Для КАБИНЫ и 47 адреса
        (rx_header.StdId >= 46) && (rx_header.StdId != 49)) {
      if (rx_header.DLC == 2) {
        if ((rx_data_can[0] == 0x00) && (rx_data_can[1] == 0x00)) {
          uint8_t buf2[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

          can_send_answer((uint32_t)(matrix_settings.addr_id + 0x80), 6, buf2);
        }

      } else if (rx_header.DLC == 6) {
        if ((rx_data_can[0] == 0x81) && (rx_data_can[1] == 0x00) &&
            ((rx_data_can[5] & 0x80) != 0x80)) {
          uint8_t buf2[2] = {0x81, 0x00};

          can_send_answer((uint32_t)(matrix_settings.addr_id + 0x80), 2, buf2);

        } else if ((rx_data_can[0] == 0x82) && (rx_data_can[1] == 0x00) &&
                   ((rx_data_can[5] & 0x80) != 0x80)) {
          uint8_t buf2[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

          can_send_answer((uint32_t)(matrix_settings.addr_id + 0x80), 6, buf2);
        }
      }
    }

    /* Полученные данные записываем в структуру msg */
    if (rx_header.DLC == 6 && rx_data_can[0] == 0x81 &&
        rx_data_can[1] == 0x00) {

      alive_cnt[0] = (alive_cnt[0] < UINT32_MAX) ? alive_cnt[0] + 1 : 0;
      is_interface_connected = true;
      is_data_received = true;

      msg.w0 = rx_data_can[2];
      msg.w1 = rx_data_can[3];
      msg.w2 = rx_data_can[4];
      msg.w3 = rx_data_can[5];
    }

#elif TEST_MODE

    if (rx_header.StdId == TEST_MODE_STD_ID) {
      is_data_received = true;
    }

#endif
  }
}

/// Счетчик для контроля ошибок для CAN.
static volatile uint8_t cnt = 0;

/**
 * @brief  Обработка прерывания для ошибок по CAN.
 * @param  hcan: Указатель на структуру CAN_HandleTypeDef.
 * @retval None
 */
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
  cnt = (cnt < UINT8_MAX) ? cnt + 1 : 0;
  uint32_t er = HAL_CAN_GetError(hcan);
}
/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void) {
  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
#if PROTOCOL_UIM_6100
  hcan.Init.Prescaler = 10; // 200 kbit/s
  hcan.Init.Mode = CAN_MODE_NORMAL;
#elif TEST_MODE
  hcan.Init.Prescaler = 4;
  hcan.Init.Mode = CAN_MODE_LOOPBACK;
#endif
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = ENABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = ENABLE;

  if (HAL_CAN_Init(&hcan) != HAL_OK) {
    Error_Handler();
  }

#if TEST_MODE
  CAN_FilterTypeDef canFilterConfig;

  canFilterConfig.FilterBank = 0;
  canFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  canFilterConfig.FilterIdHigh = 0x0000;
  canFilterConfig.FilterIdLow = 0x0000;
  canFilterConfig.FilterMaskIdHigh = 0x0000;
  canFilterConfig.FilterMaskIdLow = 0x0000;
  canFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canFilterConfig.FilterActivation = ENABLE;
  canFilterConfig.SlaveStartFilterBank = 14;
  if (HAL_CAN_ConfigFilter(&hcan, &canFilterConfig) != HAL_OK) {
    Error_Handler();
  }
#endif

  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *canHandle) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (canHandle->Instance == CAN1) {
    /* USER CODE BEGIN CAN1_MspInit 0 */

    /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
     PA11     ------> CAN_RX
     PA12     ------> CAN_TX
     */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
    /* USER CODE BEGIN CAN1_MspInit 1 */

    /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef *canHandle) {
  if (canHandle->Instance == CAN1) {
    /* USER CODE BEGIN CAN1_MspDeInit 0 */

    /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
     PA11     ------> CAN_RX
     PA12     ------> CAN_TX
     */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
    /* USER CODE BEGIN CAN1_MspDeInit 1 */

    /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/// Буфер с данными для отправки по CAN.
static uint8_t tx_data_can[8] = {
    0,
};

/**
 * @brief  Настройка кадра для передачи tx_data_can по CAN.
 * @param  stdId: Стандартный ID сообщения.
 * @retval None
 */
static void set_frame(uint32_t stdId) {
  tx_header.StdId = stdId;
  tx_header.ExtId = 0;
  tx_header.RTR = CAN_RTR_DATA;
  tx_header.IDE = CAN_ID_STD;
  tx_header.DLC = 6;
  tx_header.TransmitGlobalTime = 0;

  for (uint8_t i = 0; i < 8; i++) {
    tx_data_can[i] = (i + 10);
  }
}

/**
 * @brief  Установка фильтра для сообщений по ID.
 * @param  id: Стандартный ID сообщения.
 * @retval None
 */
static void CAN_SetFilterId(uint8_t id) {
  CAN_FilterTypeDef canFilterConfig;

  canFilterConfig.FilterBank = 0;
  canFilterConfig.FilterMode = CAN_FILTERMODE_IDLIST;
  canFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

  canFilterConfig.FilterIdHigh = id << FILTER_11_BIT_ID_OFFSET;
  canFilterConfig.FilterIdLow = 0x0000;
  canFilterConfig.FilterMaskIdHigh = id << FILTER_11_BIT_ID_OFFSET;
  canFilterConfig.FilterMaskIdLow = 0x0000;

  canFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canFilterConfig.FilterActivation = ENABLE;
  canFilterConfig.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(&hcan, &canFilterConfig) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief  Запуск интерфейса CAN.
 * @note   Установка фильтра для ID, включение нотификаций для колбека.
 * @param  hcan: Указатель на структуру CAN_HandleTypeDef.
 * @retval None
 */
void start_can(CAN_HandleTypeDef *hcan, uint32_t stdId) {
#if PROTOCOL_UIM_6100
  CAN_SetFilterId(stdId);
#endif

  HAL_CAN_Start(hcan);
  HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING |
                                         CAN_IT_ERROR | CAN_IT_BUSOFF |
                                         CAN_IT_LAST_ERROR_CODE);
}

/**
 * @brief  Завершение работы CAN.
 * @param  hcan: Указатель на структуру CAN_HandleTypeDef.
 * @retval None
 */
void stop_can(CAN_HandleTypeDef *hcan) { HAL_CAN_Stop(hcan); }

/**
 * @brief  Отправка данных по CAN (для TEST_MODE, loopback).
 * @note   Если отправленные данные получены, то отобразить строку.
 * @param  stdId: ID сообщения.
 * @retval None
 */
void CAN_TxData(uint32_t stdId) {

#if TEST_MODE

  /// Mailbox для отправляемых данных
  uint32_t tx_mailbox = 0;

  set_frame(stdId);
  if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 0) {
    HAL_CAN_AddTxMessage(&hcan, &tx_header, tx_data_can, &tx_mailbox);
  }

#endif
}

/**
 * @brief  Обработка данных, полученных по CAN.
 * @note   Если получены данные от станции управления (СУЛ), то начать обработку
 *         по протоколу.
 * @param  None
 * @retval None
 */
void process_data_from_can() {

  if (is_data_received) {
    is_data_received = false;

#if PROTOCOL_UIM_6100
    process_data_uim(&msg);
#endif
  }
}
/* USER CODE END 1 */
