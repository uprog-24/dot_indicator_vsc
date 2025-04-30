/**
 * @file    buzzer.h
 * @brief   Этот файл содержит прототипы функций для файла buzzer.c
 */
#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "main.h"

#include <stdbool.h>
#include <stdint.h>

#define VOLUME_LEVEL_LIMIT                                                     \
  3 ///< Кол-во уровней громкости (от 0 до 3) (volume_t)

/**
 * Уровни громкости (значения в процентах).
 */
typedef enum {
  VOLUME_0 = 0, // Беззвучный режим
  VOLUME_1 = 1, // Минимальный уровень громкости
  VOLUME_2 = 3, // Средний уровень громкости
  VOLUME_3 = 55 // Максимальный уровень громкости
} volume_t;

/**
 * @brief Установка состояния активного бузера. Включение и выключение.
 * @param state: Состояние states_t: TURN_ON, TURN_OFF.
 */
void set_active_buzzer_state(states_t state);

/**
 * @brief  Установка мелодии для пассивного бузера (TEST_MODE).
 * @param  freq_buff: Буфер с частотами для тонов.
 * @param  buff_size: Размер freq_buff.
 * @retval None
 */
void set_passive_buzzer_melody(const uint16_t *freq_buff, uint8_t buff_size);

/**
 * @brief  Старт воспроизведения гонга (1, 2, 3 тона), запуск TIM1 для подсчета
 *         продолжительности тона.
 * @param  bip_counter:   Кол-во тонов.
 * @param  bip_frequency: Частота стартового тона.
 * @param  volume:        Уровень громкости тона.
 * @retval None
 */
void play_gong(uint8_t bip_counter, uint16_t bip_frequency, uint8_t volume);

/**
 * @brief  Старт воспроизведения гонга для MATRIX_STATE_MENU (режим выбора
 *         уровня громкости).
 * @param  is_volume_displayed: Флаг для контроля гонга (воспроизводить 1 раз).
 * @param  volume:              Уровень громкости.
 * @retval None
 */
void play_bip_for_menu(bool *is_volume_displayed, volume_t volume);

#endif /*__ BUTTON_H__ */
