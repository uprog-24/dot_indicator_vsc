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
  MX_GPIO_Init();
// MX_TIM2_Init();
// #if !DEMO_MODE
  MX_TIM2_Init_1uS();
  Test_BuzzerStart();
// #endif
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */

  /* USER CODE BEGIN 3 */

#if TEST_MODE
  test_mode_start();
#elif DEMO_MODE
#include "test_Buzzer.h"

  MX_TIM2_Init_1uS();
  // Test_BuzzerStart();
  // play_gong(3, 1000, VOLUME_3);
  TIM2_Start_bip(1000, VOLUME_3);
  while (1) {
    // demo_mode_start();
  }

#else
#include "conf.h"

  TIM4_Start(PRESCALER_FOR_US, 1000); // 1 мс
  display_protocol_name(PROTOCOL_NAME);
  display_protocol_name(PROJECT_VER);

  read_settings(&matrix_settings);
  protocol_init();

  while (1) {
#if PROTOCOL_ALPACA
    is_interface_connected = true;
#endif
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
