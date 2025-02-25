/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    tim.c
 * @brief   This file provides code for the configuration
 *          of the TIM instances.
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
#include "tim.h"

/* USER CODE BEGIN 0 */
#include "buzzer.h"
#include "config.h"
#include "drawing.h"
#include "ukl.h"

#define TIM4_FREQ TIM2_FREQ  ///< Frequency of APB1 for TIM4
#define TIM4_PERIOD 1000 - 1 ///< Period of TIM4 for 1 sec
#define DISPLAY_STR_DURING_MS                                                  \
  2000 ///< Time in ms to display string on matrix (TEST_MODE)

#if DOT_PIN
#define BIP_OFFSET_MS 0
#elif DOT_SPI
#define BIP_OFFSET_MS 200
#endif

/**
 * @brief  Get prescaler for TIM2 (PWM) by current frequency.
 * @note   Prescaler = tim_freq / (tim_period_ARR * buzz_signal_freq)
 * @param  frequency: Number between 1..65535
 * @retval prescaler
 */
static uint16_t TIM2_get_prescaler_frequency(uint16_t frequency) {
  if (frequency == 0)
    return 0;
  return ((TIM2_FREQ / (TIM2_PERIOD * frequency)) - 1);
}

/**
 * @brief  Set buzzer volume using PWM duty cycle (0 to 100 percent)
 * @param  volume: Volume level percentage (0 to 100)
 * @retval None
 */
static void TIM2_Set_volume(uint8_t volume) {
  if (volume > 90) {
    volume = 90;
  }

  // uint32_t pulse = (volume * TIM2_PERIOD) / 100;
  uint32_t pulse = (volume * TIM2->ARR) / 100;

  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse);
  // TIM2->CCR2 = pulse;
}

/**
 * @brief  Start PWM TIM2 for buzzer
 * @retval None
 */
static void buzzer_start() { HAL_TIM_PWM_Start_IT(&htim2, TIM_CHANNEL_2); }

/**
 * @brief  Set frequency for sound of buzzer (turning on buzzer using TIM2)
 * @param  frequency: Number between 1..65535
 * @retval None
 */
void TIM2_Start_bip(uint16_t frequency, uint8_t volume) {

#if DOT_PIN

#if 1
  buzzer_start();
  // uint16_t prescaler = TIM2_get_prescaler_frequency(frequency);
  // __HAL_TIM_SET_PRESCALER(&htim2, prescaler);
  // TIM2_Set_volume(volume);
  // __HAL_TIM_SET_PRESCALER(&htim2, 64 - 1);
  TIM2->ARR = (1000000UL / frequency) - 1;

  float k = 1.0;

  switch (volume) {
  case VOLUME_3:
    switch (frequency) {
    case 3000:
      k = 1.0;
      break;

    case 1000:
      k = 1.0; // 0.85;
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
    // switch (frequency) {
    // case 1000:
    //   k = 1.0;
    //   break;

    // case 900:
    //   k = 1.0;
    //   break;

    // case 800:
    //   k = 0.8;
    //   break;

    // default:
    //   break;
    // }

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
    // switch (frequency) {
    // case 1000:
    //   k = 1.0;
    //   break;

    // case 900:
    //   k = 1.3;
    //   break;

    // case 800:
    //   k = 1.9;
    //   break;

    // default:
    //   break;
    // }

    break;

  default:
    k = 1.0;
    break;
  }

#if 0
  if (frequency == 1319) {
    TIM2->CCR2 = ((TIM2->ARR / 100) * volume * 1.7);
  } else if (frequency == 900) {
    TIM2->CCR2 = ((TIM2->ARR / 100) * volume * 1.7);
  } else {
    TIM2->CCR2 = ((TIM2->ARR / 100) * volume); // 75 73 70 3
                                               // TIM2_Set_volume(volume);
  }

#endif
  TIM2->CCR2 = ((TIM2->ARR / 100) * volume * k);
  // buzzer_start();
#endif

#elif DOT_SPI
  set_active_buzzer_state(TURN_ON);
#endif
}

/**
 * @brief  Stop buzzer PWM (TIM2)
 * @param  None
 * @retval None
 */
static void TIM2_Stop_PWM() { HAL_TIM_PWM_Stop_IT(&htim2, TIM_CHANNEL_2); }

/**
 * @brief  Turn off the sound of buzzer.
 * @note   Stop bip using prescaler of TIM2
 * @retval None
 */
void TIM2_Stop_bip() {
  uint16_t prescaler = 0;
  // __HAL_TIM_SET_PRESCALER(&htim2, prescaler);

#if DOT_PIN
  TIM2_Stop_PWM();
#elif DOT_SPI
  set_active_buzzer_state(TURN_OFF);
#endif
}

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

static volatile bool is_start_indicator = true; // Флаг 1 мс
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
    // tim3_mcs_is_elapsed++;

    // // if (tim3_mcs_is_elapsed >= 1000) {
    // tim3_mcs_is_elapsed = 0;
    // is_tim3_period_elapsed = true;
    // // }

    // is_tim3_period_elapsed = true;

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
    is_tim4_period_elapsed = true;

    if (is_start_indicator) {
      tim4_ms_counter += 1;
      if (tim4_ms_counter >= 3000) {
        tim4_ms_counter = 0;
        is_start_indicator = false;
      }
    }

#if PROTOCOL_UIM_6100 || PROTOCOL_UEL || PROTOCOL_UKL || PROTOCOL_ALPACA

    if (matrix_state == MATRIX_STATE_WORKING) {
      connection_sec_is_elapsed += 1;
      if (connection_sec_is_elapsed >= TIME_SEC_FOR_INTERFACE_CONNECTION) {
        connection_sec_is_elapsed = 0;

        is_interface_connected = (alive_cnt[0] == alive_cnt[1]) ? false : true;
        alive_cnt[1] = alive_cnt[0];
      }
    }

    if (matrix_state == MATRIX_STATE_MENU) {
      time_since_last_press_sec += 1;
      if (time_since_last_press_sec >= PERIOD_SEC_FOR_SETTINGS) {

#if DOT_SPI
        /* Для выхода из меню С сохранением настроек в памяти устройства */
        is_time_sec_for_settings_elapsed = true;
        return;
#endif
        /* Для выхода из меню БЕЗ сохранением настроек в памяти устройства */
        time_since_last_press_sec = 0;

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

/// Value of bip frequency for HAL_TIM_OC_DelayElapsedCallback
static uint16_t _bip_freq = 0;

/// Value of bip counter for HAL_TIM_OC_DelayElapsedCallback
static uint8_t _bip_counter = 0;

/// Value of bip duration for HAL_TIM_OC_DelayElapsedCallback
static uint32_t _bip_duration_ms = 0;

/// Value of bip volume for HAL_TIM_OC_DelayElapsedCallback
static uint16_t _bip_volume = 0;

/**
 * @brief  Stop sound (PWM TIM2 and TIM1 for durations of bips)
 * @param  None
 * @retval None
 */
void stop_buzzer_sound() {
  TIM1_Stop();
#if DOT_PIN
  TIM2_Stop_bip();
#elif DOT_SPI
  set_active_buzzer_state(TURN_OFF);
#endif
  _bip_counter = 0;
}

/**
 * @brief  Output Compare callback, control duration of bips for gong
 * @param  htim: Structure of TIM
 * @retval None
 */
#include "main.h"
extern matrix_state_t matrix_state;

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {

  if (htim->Instance == TIM1) {

    tim1_elapsed_ms++;

    // if (tim1_elapsed_ms == 1056 / 2) { // stop bip 1
    if (tim1_elapsed_ms == _bip_duration_ms) { // stop bip 1

      // TIM2_Stop_bip();
      // TIM2_Start_bip(900, _bip_volume);

#if DOT_PIN
      TIM2_Stop_bip();
      TIM2_Start_bip(900, _bip_volume);
#elif DOT_SPI
      set_active_buzzer_state(TURN_OFF);
#endif

      if (_bip_counter == 1) {
        stop_buzzer_sound();
      }
    }

#if DOT_SPI
    if (tim1_elapsed_ms == BIP_OFFSET_MS + _bip_duration_ms) { // start bip 2
      // TIM2_Start_bip(1200, _bip_volume);             // 1200      // 1319
      set_active_buzzer_state(TURN_ON);
    }
#endif
    //  if (tim1_elapsed_ms == 100 + 2 * _bip_duration_ms) { // stop bip 2
    // if (tim1_elapsed_ms == (1048 + 1056) / 2) { // stop bip 2
    if (tim1_elapsed_ms == BIP_OFFSET_MS + 2 * _bip_duration_ms) { // stop bip 2

      // TIM2_Stop_bip();
      // TIM2_Start_bip(800, _bip_volume);

#if DOT_PIN
      TIM2_Stop_bip();
      TIM2_Start_bip(800, _bip_volume);

#elif DOT_SPI
      set_active_buzzer_state(TURN_OFF);
#endif

      if (_bip_counter == 2) {
        stop_buzzer_sound();
      }
    }
#if DOT_SPI
    if (tim1_elapsed_ms ==
        2 * BIP_OFFSET_MS + 2 * _bip_duration_ms) { // start bip 3
      // TIM2_Start_bip(1300, _bip_volume);
      set_active_buzzer_state(TURN_ON);
    }
#endif
    //  if (tim1_elapsed_ms == 200 + 3 * _bip_duration_ms) { // stop bip 3
    // if (tim1_elapsed_ms == (1048 + 1056 + 882) / 2) { // stop bip 3
    if (tim1_elapsed_ms ==
        2 * BIP_OFFSET_MS + 3 * _bip_duration_ms) { // stop bip 3
                                                    // TIM2_Stop_bip();

#if DOT_PIN
      TIM2_Stop_bip();

#elif DOT_SPI
      set_active_buzzer_state(TURN_OFF);
#endif

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
 * @brief  Start bip for gong.
 * @note   Set frequency, bip_counter, bip_duration_ms and volume
 * @param  frequency:       Frequency for buzzer sound
 * @param  bip_counter:     Number of bips
 * @param  bip_duration_ms: Duration of the bip
 * @retval None
 */
void TIM2_Set_pwm_sound(uint16_t frequency, uint16_t bip_counter,
                        uint16_t bip_duration_ms, uint8_t volume) {
  _bip_freq = frequency;
  _bip_counter = bip_counter;
  _bip_duration_ms = bip_duration_ms;
  _bip_volume = volume;

  // start bip 1
  //	is_gong_play = true;

#if DOT_PIN
  buzzer_start();
#endif
  TIM2_Start_bip(_bip_freq, volume);
}

/**
 * @brief  Display symbols on matrix (DEMO_MODE)
 * @param  time_ms:     The time (ms) during which the symbols will be displayed
 * @param  str_symbols: Pointer to the string to be displayed
 * @retval None
 */
void TIM4_Diaplay_symbols_on_matrix(uint16_t time_ms, char *str_symbols) {
  is_tim4_period_elapsed = false;
  is_start_indicator = true;
  // uint16_t tim4_ms_counter = 0;

  // __HAL_TIM_SET_PRESCALER(&htim4, PRESCALER_FOR_MS); // 1 ms
  // __HAL_TIM_SET_AUTORELOAD(&htim4, TIM4_PERIOD);     // 1 s

  while (is_start_indicator) {
    // HAL_TIM_Base_Start_IT(&htim4);
    // is_tim4_period_elapsed = false;
    // while (!is_tim4_period_elapsed) {
#if DOT_PIN
    draw_string_on_matrix(str_symbols);
#elif DOT_SPI
    display_symbols_spi(str_symbols);
#endif
    // }
    // HAL_TIM_Base_Stop_IT(&htim4);
    // tim4_ms_counter += 1;
  }
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
 * @brief  Start TIM4 to control interface connection and matrix_state.
 * @note   Control connection of CAN/UART/DATA_Pin and MATRIX_STATE_MENU to
 *         MATRIX_STATE_START (20 seconds between button's click).
 *         Timer for 1 second
 * @param  None
 * @retval None
 */
#if 0
void TIM4_Start() {
  __HAL_TIM_SET_PRESCALER(&htim4, PRESCALER_FOR_MS); // 1 ms
  __HAL_TIM_SET_AUTORELOAD(&htim4, TIM4_PERIOD);     // 1 s
  HAL_TIM_Base_Start_IT(&htim4);
}
#endif

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
