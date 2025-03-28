/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    tim.c
 * @brief   This file provides code for the configuration
 *          of the TIM instances.
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */
#include "config.h"
#include "drawing.h"
#if PROTOCOL_UKL
#include "ukl.h"
#endif

#define TIM4_FREQ TIM2_FREQ  ///< Frequency of APB1 for TIM4
#define TIM4_PERIOD 1000 - 1 ///< Period of TIM4 for 1 sec
#define DISPLAY_STR_DURING_MS                                                  \
  2000 ///< Time in ms to display string on matrix (TEST_MODE)

/**
 * @brief  Start PWM TIM2 for buzzer
 * @retval None
 */
void TIM2_Start_PWM() { HAL_TIM_PWM_Start_IT(&htim2, TIM_CHANNEL_2); }

/**
 * @brief  Stop buzzer PWM (TIM2)
 * @param  None
 * @retval None
 */
void TIM2_Stop_PWM() { HAL_TIM_PWM_Stop_IT(&htim2, TIM_CHANNEL_2); }

/// TIM1 counter to control elapsed time in ms for bips of gong
volatile uint32_t tim1_elapsed_ms = 0;

/**
 * @brief  Stop TIM1
 * @note   Reset counter (duration for bips)
 * @param  None
 * @retval None
 */
void TIM1_Stop() {
  tim1_elapsed_ms = 0;
  HAL_TIM_Base_Stop_IT(&htim1);
  HAL_TIM_OC_Stop_IT(&htim1, TIM_CHANNEL_1);
}

/// Flag to control if period of TIM2 is elapsed
volatile bool is_tim2_period_elapsed = false;

/// Flag to control if period of TIM3 is elapsed
volatile bool is_tim3_period_elapsed = false;

/// Flag to control if period of TIM4 is elapsed
volatile bool is_tim4_period_elapsed = false;

/// Counter for elapsed time in seconds between pressing of buttons
volatile uint32_t time_since_last_press_sec = 0;

/// Counter for elapsed time in seconds to check interface connection
volatile uint32_t connection_sec_is_elapsed = 0;

/// Counter for elapsed time in seconds to check interface connection
volatile uint32_t tim3_mcs_is_elapsed = 0;

/**
 * @brief  Handle Interrupt by TIM's period is elapsed,
 *         setting the state of the flags.
 * @note   When matrix_state = MATRIX_STATE_WORKING: TIM4 control interface
 *         connection, when matrix_state = MATRIX_STATE_MENU: TIM4 count 20 sec
 *         between clicks of btn1 and btn2 (change matrix_state to
 *         MATRIX_STATE_START).
 *         TIM3 is used for UKL protocol to read bit by ukl_timings[] and for
 *         Delay_ms() in TEST_MODE and DEMO_MODE.
 *         TIM2 is used for PWM (buzzer).
 *         TIM1 is used for count duration of bip and its quantity
 * @param  htim: TIM structure
 * @retval None
 */

volatile bool is_time_sec_for_settings_elapsed = false;
volatile uint32_t is_time_ms_for_draw_elapsed = 0;
volatile bool is_tim3_half_period_elapsed = false; // Флаг 500 мкс

volatile bool is_start_indicator = true; // Флаг 1 мс
static uint16_t tim4_ms_counter = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  /// Flag to control first btn1 click
  extern bool is_first_btn_clicked;

  /// Counter for pressing of BUTTON_1
  extern uint8_t btn_1_set_mode_counter;

  /// Counter for pressing of BUTTON_2
  extern uint8_t btn_2_set_value_counter;

  /// Current matrix state: MATRIX_STATE_START, MATRIX_STATE_WORKING,
  /// MATRIX_STATE_MENU
  extern matrix_state_t matrix_state;

  /// Current menu state: MENU_STATE_OPEN, MENU_STATE_WORKING, MENU_STATE_CLOSE
  extern menu_state_t menu_state;

  // Для пассивного бузера
  if (htim->Instance == TIM2) {
    is_tim2_period_elapsed = true;
  }

  if (htim->Instance == TIM3) {
    is_tim3_period_elapsed = true; // для TEST_MODE

#if PROTOCOL_UKL

    read_data_bit();

#elif (PROTOCOL_UIM_6100) && (DOT_SPI)
    extern volatile uint32_t
        button_press_time; // Время удержания кнопки (в миллисекундах)
    extern volatile uint8_t is_button_pressed;

    // Если кнопка все еще нажата
    if (is_button_pressed) {
      button_press_time++; // Увеличиваем время удержания кнопки
    }
#endif
  }

  if (htim->Instance == TIM4) {
    /* Флаг для отсчета 1 мс (время удержания состояния одной строки с
     * колонками) для отображения символов */
    is_tim4_period_elapsed = true;

    /* Счетчик для отображения протокола и версии ПО при запуске индикатора */
    if (is_start_indicator) {
      tim4_ms_counter += 1;
      if (tim4_ms_counter >= 3000) {
        tim4_ms_counter = 0;
        is_start_indicator = false;
      }
    }

#if !DEMO_MODE && !TEST_MODE

    /* Счетчик для проверки подключения интерфейса */
    if (matrix_state == MATRIX_STATE_WORKING) {
      connection_sec_is_elapsed += 1;
      if (connection_sec_is_elapsed >= TIME_SEC_FOR_INTERFACE_CONNECTION) {
        connection_sec_is_elapsed = 0;

#if !PROTOCOL_ALPACA
        is_interface_connected = (alive_cnt[0] == alive_cnt[1]) ? false : true;
        alive_cnt[1] = alive_cnt[0];
#endif
      }
    }

    /* Счетчик для проверки подключения интерфейса */
    if (matrix_state == MATRIX_STATE_MENU) {
      time_since_last_press_sec += 1;
      if (time_since_last_press_sec >= PERIOD_SEC_FOR_SETTINGS) {

#if DOT_SPI
        /* Для выхода из меню С сохранением настроек в памяти устройства */
        is_time_sec_for_settings_elapsed = true;
        return;
#endif
        /* Флаг для выхода из меню по истечении времени бездействия:
         * PERIOD_SEC_FOR_SETTINGS секунд */
        is_time_sec_for_settings_elapsed = true;
      }
    }

#endif
  }
}

/* USER CODE END 0 */

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* TIM1 init function */
void MX_TIM1_Init(void) {
  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim1) != HAL_OK) {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
}
/* TIM2 init function */
void MX_TIM2_Init(void) {
  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);
}
/* TIM3 init function */
void MX_TIM3_Init(void) {
  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 64000 - 1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 100 - 1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */
#if DOT_SPI
  HAL_NVIC_SetPriority(TIM3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);
#endif
  /* USER CODE END TIM3_Init 2 */
}
/* TIM4 init function */
void MX_TIM4_Init(void) {
  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 64000 - 1; // freq tim = 1 000
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 100 - 1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK) {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */
  HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM4_IRQn);
  /* USER CODE END TIM4_Init 2 */
}

void MX_TIM2_Init_1uS(void) {

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 64 - 1; // freq tim = 1 000 000
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *tim_baseHandle) {
  if (tim_baseHandle->Instance == TIM1) {
    /* USER CODE BEGIN TIM1_MspInit 0 */

    /* USER CODE END TIM1_MspInit 0 */
    /* TIM1 clock enable */
    __HAL_RCC_TIM1_CLK_ENABLE();

    /* TIM1 interrupt Init */
    HAL_NVIC_SetPriority(TIM1_CC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
    /* USER CODE BEGIN TIM1_MspInit 1 */

    /* USER CODE END TIM1_MspInit 1 */
  } else if (tim_baseHandle->Instance == TIM2) {
    /* USER CODE BEGIN TIM2_MspInit 0 */

    /* USER CODE END TIM2_MspInit 0 */
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* TIM2 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    /* USER CODE BEGIN TIM2_MspInit 1 */

    /* USER CODE END TIM2_MspInit 1 */
  } else if (tim_baseHandle->Instance == TIM3) {
    /* USER CODE BEGIN TIM3_MspInit 0 */

    /* USER CODE END TIM3_MspInit 0 */
    /* TIM3 clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();

    /* TIM3 interrupt Init */
    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    /* USER CODE BEGIN TIM3_MspInit 1 */

    /* USER CODE END TIM3_MspInit 1 */
  } else if (tim_baseHandle->Instance == TIM4) {
    /* USER CODE BEGIN TIM4_MspInit 0 */

    /* USER CODE END TIM4_MspInit 0 */
    /* TIM4 clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();

    /* TIM4 interrupt Init */
    HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
    /* USER CODE BEGIN TIM4_MspInit 1 */

    /* USER CODE END TIM4_MspInit 1 */
  }
}
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *timHandle) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (timHandle->Instance == TIM2) {
    /* USER CODE BEGIN TIM2_MspPostInit 0 */

    /* USER CODE END TIM2_MspPostInit 0 */

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM2 GPIO Configuration
     PA1     ------> TIM2_CH2
     */

    // #if DOT_PIN
    GPIO_InitStruct.Pin = BUZZ_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BUZZ_GPIO_Port, &GPIO_InitStruct);
    // #endif

    /* USER CODE BEGIN TIM2_MspPostInit 1 */

    /* USER CODE END TIM2_MspPostInit 1 */
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *tim_baseHandle) {
  if (tim_baseHandle->Instance == TIM1) {
    /* USER CODE BEGIN TIM1_MspDeInit 0 */

    /* USER CODE END TIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM1_CLK_DISABLE();

    /* TIM1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM1_CC_IRQn);
    /* USER CODE BEGIN TIM1_MspDeInit 1 */

    /* USER CODE END TIM1_MspDeInit 1 */
  } else if (tim_baseHandle->Instance == TIM2) {
    /* USER CODE BEGIN TIM2_MspDeInit 0 */

    /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();

    /* TIM2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM2_IRQn);
    /* USER CODE BEGIN TIM2_MspDeInit 1 */

    /* USER CODE END TIM2_MspDeInit 1 */
  } else if (tim_baseHandle->Instance == TIM3) {
    /* USER CODE BEGIN TIM3_MspDeInit 0 */

    /* USER CODE END TIM3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM3_CLK_DISABLE();

    /* TIM3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM3_IRQn);
    /* USER CODE BEGIN TIM3_MspDeInit 1 */

    /* USER CODE END TIM3_MspDeInit 1 */
  } else if (tim_baseHandle->Instance == TIM4) {
    /* USER CODE BEGIN TIM4_MspDeInit 0 */

    /* USER CODE END TIM4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM4_CLK_DISABLE();

    /* TIM4 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM4_IRQn);
    /* USER CODE BEGIN TIM4_MspDeInit 1 */

    /* USER CODE END TIM4_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
 * @brief  TIM3 set delay in milliseconds
 * @param  delay: Number between 1..65535
 * @retval None
 */
void TIM3_Delay_ms(uint16_t delay) {
  is_tim3_period_elapsed = false;

  __HAL_TIM_SET_PRESCALER(&htim3, PRESCALER_FOR_MS);
  __HAL_TIM_SET_AUTORELOAD(&htim3, delay);
  HAL_TIM_Base_Start_IT(&htim3);
  while (!is_tim3_period_elapsed) {
  }
  HAL_TIM_Base_Stop_IT(&htim3);
}

/**
 * @brief  TIM3 set delay in microseconds
 * @param  delay: Number between 1..65535
 * @retval None
 */
void TIM3_Delay_us(uint16_t delay) {
  is_tim3_period_elapsed = false;

  __HAL_TIM_SET_PRESCALER(&htim3, PRESCALER_FOR_US);
  __HAL_TIM_SET_AUTORELOAD(&htim3, delay);
  HAL_TIM_Base_Start_IT(&htim3);
  while (!is_tim3_period_elapsed) {
  }
  HAL_TIM_Base_Stop_IT(&htim3);
}

/**
 * @brief  Start TIM1 CH1 Output Compare mode to control bip duration in ms
 * @param  None
 * @retval None
 */
void TIM1_Start() {
  __HAL_TIM_SET_PRESCALER(&htim1, PRESCALER_FOR_MS); // 1 ms
  __HAL_TIM_SET_AUTORELOAD(&htim1, 1);               // 1 ms
  HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1);
}

/**
 * @brief  Start TIM3 with CH1 for reading data bit in UKL protocol
 * @param  prescaler: Value of prescaler (PRESCALER_FOR_US for ukl_timings[],
 *                    PRESCALER_FOR_MS for DELAY_MS_DATA_RECEIVE=200)
 * @param  period:    Value of period of TIM3
 * @retval None
 */
void TIM3_Start(uint16_t prescaler, uint16_t period) {
  __HAL_TIM_SET_PRESCALER(&htim3, prescaler);
  __HAL_TIM_SET_AUTORELOAD(&htim3, period);
  HAL_TIM_Base_Start_IT(&htim3);
}

/**
 * @brief  Start TIM4 to draw symbols, control interface connection and
 * matrix_state.
 * @note   Control connection of CAN/UART/DATA_Pin and MATRIX_STATE_MENU to
 *         MATRIX_STATE_START (20 seconds between button's click).
 *         Timer for 1 ms
 * @param  None
 * @retval None
 */
void TIM4_Start(uint16_t prescaler, uint16_t period) {
  __HAL_TIM_SET_PRESCALER(&htim4, prescaler);
  __HAL_TIM_SET_AUTORELOAD(&htim4, period);
  HAL_TIM_Base_Start_IT(&htim4);
}

/**
 * @brief  Stop TIM3
 * @param  None
 * @retval None
 */
void TIM3_Stop() { HAL_TIM_Base_Stop_IT(&htim3); }

/**
 * @brief  Stop TIM4.
 * @note   Reset counters and flags
 * @param  None
 * @retval None
 */
void TIM4_Stop() {
  HAL_TIM_Base_Stop_IT(&htim4);
  time_since_last_press_sec = 0;
  alive_cnt[0] = 0;
  alive_cnt[1] = 0;
}

/* USER CODE END 1 */
