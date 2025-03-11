/**
 * @file buzzer.c
 */
#include "buzzer.h"

#include "tim.h"

#if DOT_PIN
#define BIP_DURATION_MS 500 ///< Duration of 1 bip for gong

#elif DOT_SPI
#define BIP_DURATION_MS 350 ///< Duration of 1 bip for gong

#endif

#if DOT_PIN
#define BIP_OFFSET_MS 0
#elif DOT_SPI
#define BIP_OFFSET_MS 200
#endif

#define START_GONG_BUZZER_FREQ 1000 ///< Frequency for start buzzer's sound

static uint16_t gong_frequencies_buff[3] = {1000, 900, 800};

volatile buzzer_status_struct buzzer_status = {0};

/// Value of bip frequency for HAL_TIM_OC_DelayElapsedCallback
static uint16_t _bip_freq = 0;

/// Value of bip counter for HAL_TIM_OC_DelayElapsedCallback
static uint8_t _bip_counter = 0;

/// Value of bip duration for HAL_TIM_OC_DelayElapsedCallback
static uint32_t _bip_duration_ms = 0;

/// Value of bip volume for HAL_TIM_OC_DelayElapsedCallback
static volume_t _bip_volume = 0;

extern volatile uint32_t tim1_elapsed_ms;

/**
 * @brief  Колбэк для сравнения выхода, управление длительностью бипов для
 * гонга.
 * @param  htim: Структура TIM.
 * @retval None
 */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {

  if (htim->Instance == TIM1) {

    tim1_elapsed_ms++;

    if (tim1_elapsed_ms == _bip_duration_ms) { // stop bip 1

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
      set_active_buzzer_state(TURN_ON);
    }
#endif

    if (tim1_elapsed_ms == BIP_OFFSET_MS + 2 * _bip_duration_ms) { // stop bip 2

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
      set_active_buzzer_state(TURN_ON);
    }
#endif

    if (tim1_elapsed_ms ==
        2 * BIP_OFFSET_MS + 3 * _bip_duration_ms) { // stop bip 3

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

/**
 * @brief Установка состояния активного бузера.
 * @note  Когда state = TURN_ON - бузер включается,
 *        когда state = TURN_OFF - бузер выключается.
 * @param state: Тип states_t: TURN_ON, TURN_OFF.
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
 * @brief  Установка мелодии пассивного бузера.
 * @param  freq_buff: Буфер с частотами для мелодии бузера.
 * @param  buff_size: Размер freq_buff.
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
 * @brief  Воспроизведение гонга (1, 2, 3 бипа), запуск TIM1 для отсчета
 * длительности бипа.
 * @param  bip_counter:   Количество бипов.
 * @param  bip_frequency: Частота бипа.
 * @param  volume: Уровень громкости для бипа.
 * @retval None
 */
void play_gong(uint8_t bip_counter, uint16_t bip_frequency, volume_t volume) {
  TIM1_Start();
  TIM2_Set_pwm_sound(bip_frequency, bip_counter, BIP_DURATION_MS, volume);
}

/**
 * @brief  Запуск бипа для гонга.
 * @note   Установка частоты, количества бипов, длительности бипа и громкости.
 * @param  frequency:       Частота звука бузера.
 * @param  bip_counter:     Количество бипов.
 * @param  bip_duration_ms: Длительность бипа.
 * @retval None
 */
void TIM2_Set_pwm_sound(uint16_t frequency, uint16_t bip_counter,
                        uint16_t bip_duration_ms, volume_t volume) {
  _bip_freq = frequency;
  _bip_counter = bip_counter;
  _bip_duration_ms = bip_duration_ms;
  _bip_volume = volume;

// start bip 1
#if DOT_PIN
  TIM2_Start_PWM();
#endif
  TIM2_Start_bip(_bip_freq, volume);
}

/**
 * @brief  Воспроизведение 1 бипа для MATRIX_STATE_MENU (во время выбора
 * LEVEL_VOLUME).
 * @param  is_volume_displayed: Флаг для управления бипом (воспроизведение 1 раз
 * в цикле while).
 * @param  volume:              Уровень громкости для бипа.
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

void start_buzzer_sound() {}

/**
 * @brief  Запуск бипа для бузера.
 * @param  frequency: Частота бипа.
 * @param  volume:    Уровень громкости бипа.
 * @retval None
 */
void TIM2_Start_bip(uint16_t frequency, volume_t volume) {

#if DOT_PIN

  TIM2_Start_PWM();
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

    break;

  default:
    k = 1.0;
    break;
  }

  TIM2->CCR2 = ((TIM2->ARR / 100) * volume * k);

#elif DOT_SPI
  set_active_buzzer_state(TURN_ON);
#endif
}

/**
 * @brief  Выключение звука бузера.
 * @note   Остановка бипа с использованием предделителя TIM2.
 * @retval None
 */
void TIM2_Stop_bip() {
#if DOT_PIN
  TIM2_Stop_PWM();
#elif DOT_SPI
  set_active_buzzer_state(TURN_OFF);
#endif
}

/**
 * @brief  Остановка звука (PWM TIM2 и TIM1 для длительности бипов).
 * @retval None
 */
void stop_buzzer_sound() {
  TIM1_Stop();
#if DOT_PIN
  TIM2_Stop_bip();

#if PROTOCOL_NKU
  if (buzzer_status.current_sound == SOUND_GONG) {
    buzzer_status.is_gong_sound_playing = false;
    // buzzer_status.current_sound = SOUND_NONE;
  }
  if (buzzer_status.is_button_touched_sound_playing == SOUND_ORRDER_BUTTON) {
    buzzer_status.is_button_touched_sound_playing = false;
  }

#endif
#elif DOT_SPI
  set_active_buzzer_state(TURN_OFF);
#endif
  _bip_counter = 0;
}
