/**
 * @file buzzer.c
 */
#include "buzzer.h"

#include "tim.h"

#define BIP_DURATION_MS                                                        \
  1000 ///< Продолжительнось 1 тона в мс для гонга (в реальности 1000 мс)

/**
 * @brief Установка состояния активного бузера. Включение и выключение.
 * @param state: Состояние states_t: TURN_ON, TURN_OFF.
 */
void set_active_buzzer_state(states_t state) {
  switch (state) {
  case TURN_OFF:
    HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_RESET);
    break;

  case TURN_ON:
    HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, GPIO_PIN_SET);
    break;
  }
}

/**
 * @brief  Старт воспроизведения гонга (1, 2, 3 тона), TIM4 используется для
 *         подсчета продолжительности тона.
 * @param  bip_counter:   Кол-во тонов.
 * @param  bip_frequency: Частота стартового тона.
 * @param  volume:        Уровень громкости тона.
 * @retval None
 */
void play_gong(uint8_t bip_counter, uint16_t bip_frequency, uint8_t volume) {
  TIM2_Set_pwm_sound(bip_frequency, bip_counter, BIP_DURATION_MS, volume);
}

/**
 * @brief  Старт воспроизведения гонга для MATRIX_STATE_MENU (режим выбора
 *         уровня громкости).
 * @param  is_volume_displayed: Флаг для контроля гонга (воспроизводить 1 раз).
 * @param  volume:              Уровень громкости.
 * @retval None
 */
void play_bip_for_menu(bool *is_volume_displayed, volume_t volume) {
  if (*is_volume_displayed == false) {
    *is_volume_displayed = true;
    stop_buzzer_sound();
    if (volume != VOLUME_0) {
      play_gong(3, 1000, volume);
    }
  }
}
