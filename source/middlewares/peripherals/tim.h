/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    tim.h
 * @brief   This file contains all the function prototypes for
 *          the tim.c file
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
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

extern TIM_HandleTypeDef htim1;

extern TIM_HandleTypeDef htim2;

extern TIM_HandleTypeDef htim3;

extern TIM_HandleTypeDef htim4;

/* USER CODE BEGIN Private defines */
#define TIM2_FREQ 64000000  ///< Frequency of APB1 for TIM2
#define TIM2_PERIOD 1200    ///< Period of TIM2 for buzzer
#define TIM3_FREQ TIM2_FREQ ///< Frequency of APB1 for TIM3

#define FREQ_FOR_MS 1000    ///< Frequency for ms
#define FREQ_FOR_US 1000000 ///< Frequency for us

#define PRESCALER_FOR_MS TIM3_FREQ / FREQ_FOR_MS - 1 ///< Prescaler for ms
#define PRESCALER_FOR_US TIM3_FREQ / FREQ_FOR_US - 1 ///< Prescaler for us

#define TIM4_PERIOD 1000 - 1 ///< Period of TIM4 for 1 sec
/* USER CODE END Private defines */

void MX_TIM1_Init(void);
void MX_TIM2_Init(void);
void MX_TIM3_Init(void);
void MX_TIM4_Init(void);

void MX_TIM2_Init_1uS(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE BEGIN Prototypes */

/**
 * @brief  TIM3 set delay in milliseconds
 * @param  delay: Number between 1..65535
 * @retval None
 */
void TIM3_Delay_ms(uint16_t delay);

/**
 * @brief  TIM3 set delay in microseconds
 * @param  delay: Number between 1..65535
 * @retval None
 */
void TIM3_Delay_us(uint16_t delay);

/**
 * @brief  Set frequency for sound of buzzer (turning on buzzer using TIM2)
 * @param  frequency: Number between 1..65535
 * @retval None
 */
void TIM2_Start_bip(uint16_t frequency, uint8_t volume);

/**
 * @brief  Turn off the sound of buzzer.
 * @note   Stop bip using prescaler of TIM2
 * @retval None
 */
void TIM2_Stop_bip();

/**
 * @brief  Display symbols on matrix (DEMO_MODE)
 * @param  time_ms:     The time (ms) during which the symbols will be displayed
 * @param  str_symbols: Pointer to the string to be displayed
 * @retval None
 */
void TIM4_Diaplay_symbols_on_matrix(uint16_t time_ms, char *str_symbols);

/**
 * @brief  Start TIM1 CH1 Output Compare mode to control bip duration in ms
 * @param  None
 * @retval None
 */
void TIM1_Start();

/**
 * @brief  Start TIM4 to control interface connection and matrix_state.
 * @note   Control connection of CAN/UART/DATA_Pin and MATRIX_STATE_MENU to
 *         MATRIX_STATE_START (20 seconds between button's click).
 *         Timer for 1 second
 * @param  None
 * @retval None
 */
// void TIM4_Start();
void TIM4_Start(uint16_t prescaler, uint16_t period);

/**
 * @brief  Stop TIM4.
 * @note   Reset counters and flags
 * @param  None
 * @retval None
 */
void TIM4_Stop();

/**
 * @brief  Start bip for gong.
 * @note   Set frequency, bip_counter, bip_duration_ms and volume
 * @param  frequency:       Frequency for buzzer sound
 * @param  bip_counter:     Number of bips
 * @param  bip_duration_ms: Duration of the bip
 * @retval None
 */
void TIM2_Set_pwm_sound(uint16_t frequency, uint16_t bip_counter,
                        uint16_t bip_duration_ms, uint8_t volume);

/**
 * @brief  Start TIM3 with CH1 for reading data bit in UKL protocol
 * @param  prescaler: Value of prescaler (PRESCALER_FOR_US for ukl_timings[],
 *                    PRESCALER_FOR_MS for DELAY_MS_DATA_RECEIVE=200)
 * @param  period:    Value of period of TIM3
 * @retval None
 */
void TIM3_Start(uint16_t prescaler, uint16_t period);

/**
 * @brief  Stop TIM3
 * @param  None
 * @retval None
 */
void TIM3_Stop();

/**
 * @brief  Stop sound (PWM TIM2 and TIM1 for durations of bips)
 * @param  None
 * @retval None
 */
void stop_buzzer_sound();

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */
