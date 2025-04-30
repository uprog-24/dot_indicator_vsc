/**
 * @file    buzzer.h
 * @brief   This file contains all the function prototypes for
 *          the buzzer.c file
 */
#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "main.h"

#include <stdbool.h>
#include <stdint.h>

#define VOLUME_LEVEL_LIMIT 3 ///< Limit level of volume (volume_t)

/**
 * Stores the values in % for level of buzzer volume
 */
typedef enum {
  VOLUME_0 = 0,
  VOLUME_1 = 1,
  VOLUME_2 = 3,
  VOLUME_3 = 55
} volume_t;

/**
 * @brief Setting the state of an active buzzer.
 * @note  When state = TURN_ON - buzzer is turning on,
 *        when state = TURN_OFF - buzzer is turning off
 * @param state: Type states_t: TURN_ON, TURN_OFF
 */
void set_active_buzzer_state(states_t state);

/**
 * @brief  Setting the melody of an passive buzzer
 * @param  freq_buff: Buffer with frequencies for buzzer's melody
 * @param  buff_size: Size of freq_buff
 * @retval None
 */
void set_passive_buzzer_melody(const uint16_t *freq_buff, uint8_t buff_size);

/**
 * @brief  Play gong using timers for start buzzer sound and count duration
 * @param  bip_counter:   Number of bips
 * @param  bip_frequency: Frequency bip
 * @retval None
 */
void play_gong(uint8_t bip_counter, uint16_t bip_frequency, uint8_t volume);

/**
 * @brief  Play 1 bip for MATRIX_STATE_MENU (during selection of LEVEL_VOLUME)
 * @param  is_volume_displayed: Flag to control bip (play 1 time in while loop)
 * @param  volume:              Level of volume for bip
 * @retval None
 */
void play_bip_for_menu(bool *is_volume_displayed, volume_t volume);

#endif /*__ BUTTON_H__ */
