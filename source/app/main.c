/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "gpio.h"
#include "main.h"
#include "tim.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "button.h"
#include "config.h"

#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#if PROTOCOL_UIM_6100 || PROTOCOL_UEL || PROTOCOL_UKL || PROTOCOL_ALPACA

/// Settings of matrix: addr_id of matrix and level of volume for buzzer
settings_t matrix_settings = {.addr_id = MAIN_CABIN_ID, .volume = VOLUME_1};

#endif

/// Counters to control interface connection (UART/CAN/DATA_Pin)
volatile uint32_t alive_cnt[2] = {
    0,
};

/// Flag to control is CAN/UART/DATA_Pin connected
volatile bool is_interface_connected = true;

/// Current matrix state: MATRIX_STATE_START, MATRIX_STATE_WORKING,
/// MATRIX_STATE_MENU
volatile matrix_state_t matrix_state = MATRIX_STATE_START;

/// Current menu state: MENU_STATE_OPEN, MENU_STATE_WORKING, MENU_STATE_CLOSE
menu_state_t menu_state = MENU_STATE_OPEN;

/// String that will be displayed on matrix
char matrix_string[3];

volatile bool is_saved_settings = 0;

/* USER CODE END 0 */
/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */

#if DOT_PIN
  MX_GPIO_Init();
  MX_TIM2_Init_1uS(); /* Инициализация таймера 2 для бузера */

#elif DOT_SPI
  MX_GPIO_Init_SPI();

  software_SPI_addPins(MBI5026_MOSI_PIN_GPIO_Port, MBI5026_MOSI_PIN_Pin,
                       MBI5026_SCK_PIN_GPIO_Port, MBI5026_SCK_PIN_Pin);
  LED_driver_set_pins(MBI5026_LE_PIN_GPIO_Port, MBI5026_LE_PIN_Pin,
                      MBI5026_NOE_PIN_GPIO_Port, MBI5026_NOE_PIN_Pin);

  MX_TIM1_Init_1(); /* Инициализация таймера 1 для бузера */
#endif

  MX_TIM3_Init();
  MX_TIM4_Init();

#if TEST_MODE
  test_mode_start();
#elif DEMO_MODE

  TIM4_Start(PRESCALER_FOR_US, 1000); // 1 мс

  // start_buzzer_sound(5000, VOLUME_3); // 65
  // start_buzzer_sound(7000, VOLUME_3); // 70
  start_buzzer_sound(3000, 55); // 70 работает
  HAL_Delay(2000);
  stop_buzzer_sound();

  // start_buzzer_sound(3100, 55); // 73-78 !!!

  // start_buzzer_sound(7500, 55); // для плоского бузера 75 дБ

  // play_gong(3, 1000, VOLUME_3);

  while (1) {
    // demo_mode_start();
  }

#else
#include "conf.h"

#if 1
  TIM4_Start(PRESCALER_FOR_US, 1000); // 1 мс
  display_protocol_name(PROTOCOL_NAME);
  display_protocol_name(PROJECT_VER);
#endif

  read_settings(&matrix_settings);
  protocol_init();

  while (1) {
    switch (matrix_state) {
    case MATRIX_STATE_START:
      protocol_start();
      matrix_state = MATRIX_STATE_WORKING;
      break;

    case MATRIX_STATE_WORKING:
      protocol_process_data();
      break;

    case MATRIX_STATE_MENU:

      switch (menu_state) {
      case MENU_STATE_OPEN:
        protocol_stop();
        menu_state = MENU_STATE_WORKING;
        break;

      case MENU_STATE_WORKING:
        press_button();
        break;

      case MENU_STATE_CLOSE:
        overwrite_settings(&matrix_settings);
        matrix_state = MATRIX_STATE_START;
        menu_state = MENU_STATE_OPEN;
        break;
      }

      break;
    }
  }

#endif

  /* USER CODE END 3 */
}

#if DOT_SPI
void MX_GPIO_Init_SPI(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA,
                    MBI5026_SCK_PIN_Pin | MBI5026_NOE_PIN_Pin |
                        MBI5026_LE_PIN_Pin | BUZZ_Pin,
                    GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MBI5026_MOSI_PIN_GPIO_Port, MBI5026_MOSI_PIN_Pin,
                    GPIO_PIN_SET);

  /*Configure GPIO pins : MBI5026_SCK_PIN_Pin MBI5026_MOSI_PIN_Pin
     MBI5026_NOE_PIN_Pin MBI5026_LE_PIN_Pin BUZ_3_Pin */
  GPIO_InitStruct.Pin = MBI5026_SCK_PIN_Pin | MBI5026_MOSI_PIN_Pin |
                        MBI5026_NOE_PIN_Pin | MBI5026_LE_PIN_Pin | BUZZ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SW_IN_3_Pin */
  GPIO_InitStruct.Pin = SW_IN_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SW_IN_3_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = BUZZ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BUZZ_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  // HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  /* USER CODE END MX_GPIO_Init_2 */
}

void MX_TIM1_Init_1(void) {

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 64 - 1; // freq tim = 1 000 000
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim1);
}
#endif

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
