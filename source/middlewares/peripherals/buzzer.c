/**
 * @file buzzer.c
 */
#include "buzzer.h"

#include "tim.h"

#define BIP_DURATION_MS 500   ///< Duration of 1 bip for gong
#define GONG_BUZZER_FREQ 2500 ///< Frequency for buzzer's sound

/**
 * @brief Setting the state of an active buzzer.
 * @note  When state = TURN_ON - buzzer is turning on,
 *        when state = TURN_OFF - buzzer is turning off
 * @param state: Type states_t: TURN_ON, TURN_OFF
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
 * @brief  Setting the melody of an passive buzzer
 * @param  freq_buff: Buffer with frequencies for buzzer's melody
 * @param  buff_size: Size of freq_buff
 * @retval None
 */
void set_passive_buzzer_melody(uint16_t *freq_buff, uint8_t buff_size) {
  for (uint8_t ind_freq = 0; ind_freq < buff_size; ind_freq++) {
    TIM2_Start_bip(freq_buff[ind_freq], VOLUME_1);
    TIM3_Delay_ms(250);
  }
  TIM2_Stop_bip();
}

/**
 * @brief  Play gong (1, 2, 3 bips), start TIM1 to count duration of the bip
 * @param  bip_counter:   Number of bips
 * @param  bip_frequency: Frequency bip
 * @param  volume: Level of volume for bip
 * @retval None
 */
void play_gong(uint8_t bip_counter, uint16_t bip_frequency, uint8_t volume) {
  TIM1_Start();
  TIM2_Set_pwm_sound(bip_frequency, bip_counter, BIP_DURATION_MS, volume);
}

/**
 * @brief  Play 1 bip for MATRIX_STATE_MENU (during selection of LEVEL_VOLUME)
 * @param  is_volume_displayed: Flag to control bip (play 1 time in while loop)
 * @param  volume:              Level of volume for bip
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
