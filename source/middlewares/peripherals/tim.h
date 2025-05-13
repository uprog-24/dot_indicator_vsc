/**
 * @file    tim.h
 * @brief   Этот файл содержит прототипы функций для файла tim.c
 */

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
#define TIM2_FREQ 64000000  ///< Частота линии APB1 для TIM2
#define TIM3_FREQ TIM2_FREQ ///< Частота линии APB1 для TIM3

#define FREQ_FOR_MS 1000    ///< Частота для 1 мс
#define FREQ_FOR_US 1000000 ///< Частота для 1 мкс

#define PRESCALER_FOR_MS                                                       \
  TIM3_FREQ / FREQ_FOR_MS - 1 ///< Прескелер для таймера в 1 мс
#define PRESCALER_FOR_US                                                       \
  TIM3_FREQ / FREQ_FOR_US - 1 ///< Прескелер для таймера в 1 мкс

/* USER CODE END Private defines */

void MX_TIM1_Init(void);
void MX_TIM2_Init(void);
void MX_TIM3_Init(void);
void MX_TIM4_Init(void);

void Timer_Buzzer_Init_1uS(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE BEGIN Prototypes */

/**
 * @brief  Установка задержки в мс (таймер 3).
 * @param  delay: Значение задержки 1..65535.
 * @retval None
 */
void TIM3_Delay_ms(uint16_t delay);

/**
 * @brief  Установка задержки в мкс (таймер 3).
 * @param  delay: Значение задержки 1..65535.
 * @retval None
 */
void TIM3_Delay_us(uint16_t delay);

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
void TIM4_Start(uint16_t prescaler, uint16_t period);

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
                        uint16_t bip_duration_ms, uint8_t volume);

/**
 * @brief  Запуск TIM3 с каналом CH1 для чтения бита данных для протокола
 *         УЛ/УКЛ.
 * @param  prescaler: Значение прескелера (PRESCALER_FOR_US для ukl_timings[],
 *                    PRESCALER_FOR_MS для DELAY_MS_DATA_RECEIVE=200 по
 *                    завершении приема).
 * @param  period:    Значение периода для TIM3.
 * @retval None
 */
void TIM3_Start(uint16_t prescaler, uint16_t period);

/**
 * @brief  Остановка таймера 3.
 * @param  None
 * @retval None
 */
void TIM3_Stop();

/**
 * @brief  Включение тона пассивного бузера (подключен к каналу таймера 2).
 * @param  frequency: Значение частоты 1..65535.
 * @param  volume:    Уровень громкости (volume_t из buzzer.h).
 * @retval None
 */
void start_buzzer_sound(uint16_t frequency, uint8_t volume);

/**
 * @brief  Выключение бузера (ШИМ TIM2).
 * @param  None
 * @retval None
 */
void stop_buzzer_sound();

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */
