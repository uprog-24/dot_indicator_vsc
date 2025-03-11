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
#include "drawing.h"
#include "uim6100.h"

msg_t msg = {0, 0, 0, 0};

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define FILTER_11_BIT_ID_OFFSET 5 ///< Offset for Standard frame ID filter

/// Structure of Header for receiving data
static CAN_RxHeaderTypeDef rx_header;

/// Buffer for receiving data
static uint8_t rx_data_can[BUFFER_SIZE_BYTES] = {
    0x00,
};

static CAN_Data_Message_t last_received_message = {0};

/// Flag to control is data received by CAN
volatile bool is_data_received = false;

/**
 * @brief  Handle Interrupt by receiving data after transmitting by CAN,
 *         setting is_data_received flag when data with StdId is received.
 *         Set the counter alive_cnt[0] to control interface connection (set
 *         alive_cnt[1] and comparison in tim.c TIM4)
 * @param  hcan: Pointer to a CAN_HandleTypeDef structure
 * @retval None
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {

#if PROTOCOL_NKU
  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header,
                           last_received_message.rx_data_can) == HAL_OK) {

    if (rx_header.StdId == (rx_header.StdId | matrix_settings.group_id << 4)) {

      alive_cnt[0] = (alive_cnt[0] < UINT32_MAX) ? alive_cnt[0] + 1 : 0;
      is_interface_connected = true;
      is_data_received = true;

      last_received_message.std_id = rx_header.StdId;
      last_received_message.dlc = rx_header.DLC;
      last_received_message.is_data_received = true;
    }
  }
#endif

  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data_can) ==
      HAL_OK) {

#if PROTOCOL_UIM_6100

#if 1
    if ((matrix_settings.addr_id == rx_header.StdId) &&
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
#endif

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

#elif PROTOCOL_ALPACA
    alive_cnt[0] = (alive_cnt[0] < UINT32_MAX) ? alive_cnt[0] + 1 : 0;
    is_interface_connected = true;
    is_data_received = true;
#endif
  }
}

/// Counter to control CAN errors
volatile uint8_t cnt = 0;

// Функция для получения последнего пакета данных
CAN_Data_Message_t *get_received_data_by_can(void) {

  // if (last_received_message.is_data_received) {
  //   package = last_received_message; // Копируем данные
  //   last_received_message.is_data_received =
  //       false; // Сбрасываем флаг после выдачи данных
  // } else {
  //   package.is_data_received =
  //       false; // Если нет новых данных, вернуть пустую структуру
  // }

  return &last_received_message;
}

/**
 * @brief  Handle Interrupt by CAN errors.
 * @param  hcan: Structure of CAN
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
#elif PROTOCOL_ALPACA
  hcan.Init.Prescaler = 16; // 125 kbit/s
  hcan.Init.Mode = CAN_MODE_LOOPBACK;

#elif PROTOCOL_NKU
  hcan.Init.Prescaler = 16; // 125 kbit/s
  hcan.Init.Mode = CAN_MODE_NORMAL;
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

  /* Прием всех сообщений */
#if TEST_MODE || PROTOCOL_ALPACA || PROTOCOL_NKU
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

#if DOT_PIN
    GPIO_InitStruct.Pin = GPIO_PIN_11;
#elif DOT_SPI
    GPIO_InitStruct.Pin = GPIO_PIN_8;
#endif
    // GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

#if DOT_PIN
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#elif DOT_SPI
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif
    // HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

#if DOT_PIN
    GPIO_InitStruct.Pin = GPIO_PIN_12;
#elif DOT_SPI
    GPIO_InitStruct.Pin = GPIO_PIN_9;
#endif
    // GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    // HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

#if DOT_PIN
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#elif DOT_SPI
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_AFIO_REMAP_CAN1_2();
#endif

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
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

#if DOT_PIN
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);
#elif DOT_SPI
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);
#endif
    // HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
    /* USER CODE BEGIN CAN1_MspDeInit 1 */

    /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/// Buffer for transmitting data
static uint8_t tx_data_can[8] = {
    0,
};

/// Structure of Header for transmitting data
static CAN_TxHeaderTypeDef tx_header;

/**
 * @brief  Setting frame for transmitting TxData by CAN
 * @param  stdId: The standard ID of the frame
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

void can_send_answer(uint32_t stdId, uint8_t dlc, uint8_t *buffer) {
  tx_header.StdId = stdId;
  tx_header.ExtId = 0;
  tx_header.RTR = CAN_RTR_DATA;
  tx_header.IDE = CAN_ID_STD;
  tx_header.DLC = dlc;
  tx_header.TransmitGlobalTime = 0;

  /// Mailbox for transmitted data
  uint32_t tx_mailbox = 0;

  if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 0) {
    HAL_CAN_AddTxMessage(&hcan, &tx_header, buffer, &tx_mailbox);
  }
}

/**
 * @brief  Set filter for frame ID
 * @param  id: Standard ID of frame
 * @retval None
 */
void CAN_SetFilterId(uint8_t id) {
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
 * @brief  Start CAN
 * @note   Set filter for frame ID, activate notifications for
 *         interrupt callback
 * @param  hcan: Pointer to the CAN_HandleTypeDef structure
 * @retval None
 */
void start_can(CAN_HandleTypeDef *hcan, uint32_t stdId) {
#if PROTOCOL_UIM_6100
  // #if DOT_PIN
  CAN_SetFilterId(stdId);
// #endif
#endif

  HAL_CAN_Start(hcan);
  HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING |
                                         CAN_IT_ERROR | CAN_IT_BUSOFF |
                                         CAN_IT_LAST_ERROR_CODE);
}

/**
 * @brief  Stop CAN
 * @param  hcan: Pointer to the CAN_HandleTypeDef structure
 * @retval None
 */
void stop_can(CAN_HandleTypeDef *hcan) { HAL_CAN_Stop(hcan); }

/**
 * @brief  Transmit data by CAN.
 * @note   If transmitted data is received then set symbols to matrix
 * @param  stdId: Standard ID of frame
 * @retval None
 */
void CAN_TxData(uint32_t stdId) {

#if TEST_MODE
  /// String OK
  char *str_ok = "0K";

  /// Mailbox for transmitted data
  uint32_t tx_mailbox = 0;

  set_frame(stdId);
  if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 0) {
    HAL_CAN_AddTxMessage(&hcan, &tx_header, tx_data_can, &tx_mailbox);
  }

  if (is_data_received) {
    is_data_received = false;
#if DOT_PIN
    draw_string_on_matrix(str_ok);
#elif DOT_SPI
    display_symbols_spi("c0K");
#endif
  }

#elif PROTOCOL_ALPACA

  /// Mailbox for transmitted data
  uint32_t tx_mailbox = 0;

  tx_header.StdId = stdId;
  tx_header.ExtId = 0;
  tx_header.RTR = CAN_RTR_DATA;
  tx_header.IDE = CAN_ID_STD;
  tx_header.DLC = 2;
  tx_header.TransmitGlobalTime = 0;

  uint16_t number_to_send = stdId;
  tx_data_can[0] = (number_to_send >> 8) & 0xFF;
  tx_data_can[1] = number_to_send & 0xFF;

  if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 0) {
    HAL_CAN_AddTxMessage(&hcan, &tx_header, tx_data_can, &tx_mailbox);
  }
#endif
}

bool is_can_data_received() { return is_data_received; }

void reset_value_data_received() { is_data_received = false; }

uint8_t v = 0;
/**
 * @brief  Process data received by CAN.
 * @note   If transmitted data by UIM6100 protocol is received then process
 * data
 * @param  None
 * @retval None
 */
void process_data_from_can() {

#if PROTOCOL_ALPACA
// движения нет
// CAN_TxData(10000);
// движение вверх
// CAN_TxData(10001);
// CAN_TxData(10006); // 3 floor
// CAN_TxData(10003);

// Перегрузка кабины
// CAN_TxData(10738);

// Погрузка
// CAN_TxData(10735);

// Гонг Прибытие
// if (v == 0) {
//   v++;
// CAN_TxData(10739);
// }

// Пожарная опасность
// CAN_TxData(10741);

// Открытие дверей
// if (v == 0) {
//   v++;
//   CAN_TxData(10742);
// }

// Неисправность лифта
// CAN_TxData(10745);

// Эвакуация
// CAN_TxData(10746);
#endif

  if (is_data_received) {
    is_data_received = false;

#if PROTOCOL_UIM_6100
    process_data_uim(&msg);
#elif PROTOCOL_ALPACA
    process_data_alpaca(rx_data_can);
#endif
  }
}
/* USER CODE END 1 */
