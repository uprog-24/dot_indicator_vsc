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
  VOLUME_1 = 1, // 80
  VOLUME_2 = 3, // 75
  VOLUME_3 = 55 // 55
} volume_t;

typedef enum {
  SOUND_NONE = 0,
  SOUND_ORRDER_BUTTON,
  SOUND_GONG,
  SOUND_CABIN_OVERLOAD,
  SOUND_FIRE_SIREN
} sound_types_t;

typedef struct {
  sound_types_t current_sound;
  bool is_gong_sound_playing;
  bool is_button_touched_sound_playing;
} buzzer_status_struct;

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
void set_passive_buzzer_melody(uint16_t *freq_buff, uint8_t buff_size);

/**
 * @brief  Play gong using timers for start buzzer sound and count duration
 * @param  bip_counter:   Number of bips
 * @param  bip_frequency: Frequency bip
 * @retval None
 */
void play_gong(uint8_t bip_counter, uint16_t bip_frequency, volume_t volume);

/**
 * @brief  Play 1 bip for MATRIX_STATE_MENU (during selection of LEVEL_VOLUME)
 * @param  is_volume_displayed: Flag to control bip (play 1 time in while loop)
 * @param  volume:              Level of volume for bip
 * @retval None
 */
void play_bip_for_menu(bool *is_volume_displayed, volume_t volume);

void start_buzzer_sound();

/**
 * @brief  Остановка звука (PWM TIM2 и TIM1 для длительности бипов).
 * @retval None
 */
void stop_buzzer_sound();

#endif /*__ BUZZER_H__ */
