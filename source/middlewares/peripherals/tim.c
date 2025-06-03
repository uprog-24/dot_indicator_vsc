/**
 * @file    tim.c
 */

/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */
#include "config.h"

#define TIM4_FREQ TIM2_FREQ ///< Частота линии APB1 для TIM4

#if PROTOCOL_UIM_6100 || PROTOCOL_UEL || PROTOCOL_UKL || PROTOCOL_ALPACA
#define TIME_DISPLAY_STRING_DURING_MS                                          \
  3000 ///< Время в мс, в течение которого отображается строка при подаче
       ///< питания

#elif DEMO_MODE
#define TIME_DISPLAY_STRING_DURING_MS                                          \
  2000 ///< Время в мс, в течение которого отображается строка

#endif

/**
 * @brief  Запуск таймера 2 на 2-ом канале в режиме ШИМ (для пассивного бузера).
 * @retval None
 */
static void TIM2_Start_PWM() { HAL_TIM_PWM_Start_IT(&htim2, TIM_CHANNEL_2); }

/**
 * @brief  Включение тона пассивного бузера (подключен к таймеру 2).
 * @param  frequency: Значение частоты 1..65535.
 * @param  volume:    Уровень громкости (volume_t из buzzer.h).
 * @retval None
 */
void TIM2_Start_bip(uint16_t frequency, uint8_t volume) {

  TIM2_Start_PWM();
  /* Инициализация таймера 2 на 1 мкс, поэтому используем 1000000UL */
  TIM2->ARR = (1000000UL / frequency) - 1; //

  float k = 1.0;

  /* Устанавливаем коэффициенты для уравновешивания уровней громкостей при
   * подаче разных частот гонга:
   * 1000 Гц, 900 Гц, 800 Гц - частоты тонов для гонга;
   * 3000 Гц - частота для режима Перегруз кабины и Пожар (максимальная
   * громкость, 80 дБ на расстоянии в 1 м). HC0905A (горизонтальный дот). */
  switch (volume) {
  case VOLUME_3:
    switch (frequency) {
    case 3000:
      k = 1.0;
      break;

    case 1000:
      k = 1.0;
      break;

    case 900:
      k = 1.25;
      break;

    case 800:
      k = 1.2;
      break;

    default:
      break;
    }

    break;

  case VOLUME_2:
    switch (frequency) {
    case 1000:
      k = 1.0;
      break;

    case 900:
      k = 0.9;
      break;

    case 800:
      k = 0.8;
      break;

    default:
      break;
    }

    break;

  case VOLUME_1:
    switch (frequency) {
    case 1000:
      k = 1.0;
      break;

    case 900:
      k = 0.76;
      break;

    case 800:
      k = 0.8;
      break;

    default:
      break;
    }

    break;

  default:
    k = 1.0;
    break;
  }

  /* Установка громкости */
  TIM2->CCR2 = ((TIM2->ARR / 100) * volume * k);
}

/**
 * @brief  Остановка пассивного бузера (ШИМ TIM2).
 * @param  None
 * @retval None
 */
static void TIM2_Stop_PWM() { HAL_TIM_PWM_Stop_IT(&htim2, TIM_CHANNEL_2); }

/**
 * @brief  Выключение тона бузера.
 * @retval None
 */
void TIM2_Stop_bip() {
  uint16_t prescaler = 0;
  // __HAL_TIM_SET_PRESCALER(&htim2, prescaler);
  TIM2_Stop_PWM();
}

/// Счетчик для подсчета продолжительности тонов гонга.
volatile uint32_t tim1_elapsed_ms = 0;

/**
 * @brief  Остановка TIM1.
 * @note   Сброс счетчика для подсчета продолжительности тонов гонга.
 * @param  None
 * @retval None
 */
static void TIM1_Stop() {
  tim1_elapsed_ms = 0;
  HAL_TIM_Base_Stop_IT(&htim1);
  HAL_TIM_OC_Stop_IT(&htim1, TIM_CHANNEL_1);
}

/// Флаг для контроля завершения периода TIM3
volatile bool is_tim3_period_elapsed = false;

/// Флаг для удержания состояния строки в темение 1 мс (максимвльная яркость,
/// частота обновления матрицы 125 Гц).
volatile bool is_tim4_period_elapsed = false;

/// Счетчик прошедшего в мс времени между последними нажатиями кнопок.
volatile uint32_t time_since_last_press_ms = 0;

/// Счетчик прошедшего в мс времени для проверки подключения интерфейса (CAN,
/// USART)
volatile uint32_t connection_ms_is_elapsed = 0;

/// Флаг для отображения строки в течение TIME_DISPLAY_STRING_DURING_MS
volatile bool is_time_ms_for_display_str_elapsed = false;

/// Счетчик времени отображения строки при запуске индикатора в мс
static uint16_t tim4_ms_counter = 0;

/**
 * @brief  Обработка прерываний по завершении периода таймеров.
 * @param  htim: Указатель на структуру таймера.
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  /// Флаг для детектирования первого нажатия кнопки 1 (вход в меню - считывание
  /// настроек из flash-памяти).
  extern bool is_first_btn_clicked;

  /// Счетчик кол-ва нажатий BUTTON_1
  extern uint8_t btn_1_set_mode_counter;

  /// Счетчик кол-ва нажатий BUTTON_2
  extern uint8_t btn_2_set_value_counter;

  /// Текущее состояние индикатора: MATRIX_STATE_START, MATRIX_STATE_WORKING,
  /// MATRIX_STATE_MENU
  extern matrix_state_t matrix_state;

  /// Текущее состояние меню: MENU_STATE_OPEN, MENU_STATE_WORKING,
  /// MENU_STATE_CLOSE
  extern menu_state_t menu_state;

  if (htim->Instance == TIM3) {
    is_tim3_period_elapsed = true; // для TEST_MODE

#if PROTOCOL_UKL

    read_data_bit();

#endif
  }

  if (htim->Instance == TIM4) {
    /* Флаг для отсчета 1 мс (время удержания состояния одной строки с
     * колонками) для отображения символов */
    is_tim4_period_elapsed = true;

/* Счетчик для отображения строки в течение TIME_DISPLAY_STRING_DURING_MS
 * для протоколов: название протокола и номер версии ПО при запуске
 * индикатора; для DEMO_MODE: отображение строки */
#if !TEST_MODE
    if (!is_time_ms_for_display_str_elapsed) {
      tim4_ms_counter += 1;
      if (tim4_ms_counter >= TIME_DISPLAY_STRING_DURING_MS) {
        tim4_ms_counter = 0;
        is_time_ms_for_display_str_elapsed = true;
      }
    }
#endif

#if PROTOCOL_UIM_6100 || PROTOCOL_UEL || PROTOCOL_UKL

    /* Счетчик для проверки подключения интерфейса */
    if (matrix_state == MATRIX_STATE_WORKING) {
      connection_ms_is_elapsed += 1;
      if (connection_ms_is_elapsed >= TIME_MS_FOR_INTERFACE_CONNECTION) {
        connection_ms_is_elapsed = 0;

        is_interface_connected = (alive_cnt[0] == alive_cnt[1]) ? false : true;
        alive_cnt[1] = alive_cnt[0];
      }
    }

    /* Счетчик для проверки бездействия кнопок в течение TIME_MS_FOR_SETTINGS
     * мс */
    if (matrix_state == MATRIX_STATE_MENU) {
      time_since_last_press_ms += 1;

      if (time_since_last_press_ms >= TIME_MS_FOR_SETTINGS) {
        time_since_last_press_ms = 0;

        btn_1_set_mode_counter = 0;
        btn_2_set_value_counter = 0;
        is_first_btn_clicked = true;
        matrix_state = MATRIX_STATE_START;
        menu_state = MENU_STATE_OPEN;
      }
    }

#endif
  }
}

/// Знвчение частоты тона гонга для HAL_TIM_OC_DelayElapsedCallback
static uint16_t _bip_freq = 0;

/// Кол-во тонов гонга для HAL_TIM_OC_DelayElapsedCallback
uint8_t _bip_counter = 0;

/// Продолжительность тона гонга для HAL_TIM_OC_DelayElapsedCallback
static uint32_t _bip_duration_ms = 0;

/// Уровень громкости тона гонга для HAL_TIM_OC_DelayElapsedCallback
static uint16_t _bip_volume = 0;

/**
 * @brief  Выключение бузера (ШИМ TIM2 и TIM1 для подсчета продолжительности
 *         тона бузера).
 * @param  None
 * @retval None
 */
extern bool is_door_sound;
void stop_buzzer_sound() {
  TIM1_Stop();
  TIM2_Stop_bip();
  _bip_counter = 0;
   is_door_sound = false;
}

/**
 * @brief  Output Compare колбек, подсчет продолжительности тона гонга.
 * @param  htim: Указатель на структуру таймера.
 * @retval None
 */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM1) {
    tim1_elapsed_ms++;

    if (tim1_elapsed_ms == _bip_duration_ms) {

      TIM2_Start_bip(900, _bip_volume); // Запускаем 2-ой тон

      if (_bip_counter == 1) {
        stop_buzzer_sound();
      }
    }

    if (tim1_elapsed_ms == 2 * _bip_duration_ms) {

      TIM2_Start_bip(800, _bip_volume); // Запускаем 3-ий тон

      if (_bip_counter == 2) {
        stop_buzzer_sound();
      }
    }

    if (tim1_elapsed_ms == 3 * _bip_duration_ms) { // stop bip 3

      if (_bip_counter == 3) {
        stop_buzzer_sound();
      }
    }
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
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
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

void Timer_Buzzer_Init_1uS(void) {

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
  htim4.Init.Prescaler = 64000 - 1;
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
    GPIO_InitStruct.Pin = BUZZ_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BUZZ_GPIO_Port, &GPIO_InitStruct);

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
 * @brief  Установка задержки в мс (таймер 3).
 * @param  delay: Значение задержки 1..65535.
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
 * @brief  Установка задержки в мкс (таймер 3).
 * @param  delay: Значение задержки 1..65535.
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
 * @brief  Запуск гонга (первый тон).
 * @note   Установка частоты, bip_counter - кол-ва тонов, bip_duration_ms -
 *         продолжительность тона и volume - уровень громкости.
 * @param  frequency:       Частота тона.
 * @param  bip_counter:     Кол-во тонов.
 * @param  bip_duration_ms: Продолжительность тона в мс.
 * @retval None
 */
void TIM2_Set_pwm_sound(uint16_t frequency, uint16_t bip_counter,
                        uint16_t bip_duration_ms, uint8_t volume) {
  _bip_freq = frequency;
  _bip_counter = bip_counter;
  _bip_duration_ms = bip_duration_ms;
  _bip_volume = volume;

  // start bip 1
  // TIM2_Start_PWM();
  TIM2_Start_bip(_bip_freq, volume);
}

/**
 * @brief  Запуск таймера 1 с каналом CH1 Output Compare mode для подсчета
 *         продолжительности тона бузера в мс.
 * @param  None
 * @retval None
 */
void TIM1_Start() {
  __HAL_TIM_SET_PRESCALER(&htim1, PRESCALER_FOR_MS); // 1 ms
  __HAL_TIM_SET_AUTORELOAD(&htim1, 1);               // 1 ms
  HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1);
}

/**
 * @brief  Запуск TIM3 с каналом CH1 для чтения бита данных для протокола
 *         УЛ/УКЛ.
 * @param  prescaler: Значение прескелера (PRESCALER_FOR_US для ukl_timings[],
 *                    PRESCALER_FOR_MS для DELAY_MS_DATA_RECEIVE=200 по
 *                    завершении приема).
 * @param  period:    Значение периода для TIM3.
 * @retval None
 */
void TIM3_Start(uint16_t prescaler, uint16_t period) {
  __HAL_TIM_SET_PRESCALER(&htim3, prescaler);
  __HAL_TIM_SET_AUTORELOAD(&htim3, period);
  HAL_TIM_Base_Start_IT(&htim3);
}

/**
 * @brief  Остановка таймера 3.
 * @param  None
 * @retval None
 */
void TIM3_Stop() { HAL_TIM_Base_Stop_IT(&htim3); }

/**
 * @brief  Запуск TIM4 на 1 мс.
 * @note   Используется:
 *         1. для отображения символов (яркость, удержание строки в течение 1
 *            мс);
 *         2. для отображения строк в течение TIME_DISPLAY_STRING_DURING_MS;
 *         3. для контроля подключения интерфейса (CAN, USART);
 *         4. для проверки бездействия кнопок в течение TIME_MS_FOR_SETTINGS в
 *            режиме меню.
 * @param  None
 * @retval None
 */
void TIM4_Start(uint16_t prescaler, uint16_t period) {
  __HAL_TIM_SET_PRESCALER(&htim4, prescaler);
  __HAL_TIM_SET_AUTORELOAD(&htim4, period);
  HAL_TIM_Base_Start_IT(&htim4);
}

/* USER CODE END 1 */
