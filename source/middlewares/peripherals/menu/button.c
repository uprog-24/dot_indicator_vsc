/**
 * @file button.c
 */
#include "button.h"

#include "buzzer.h"
#include "config.h"
#include "drawing.h"
#include "tim.h"

#include <stdbool.h>

#define SETTINGS_MODE_VOLUME                                                   \
  "V0L" ///< Строка для отображения режима VOL (уровень громкости)
#define SETTINGS_MODE_ID                                                       \
  "cID" ///< Строка для отображения режима ID (адрес индикатора)

#define SETTINGS_MODE_ESC "ESC" ///< Строка для отображения режима ESC

#define LEVEL_VOLUME_0                                                         \
  "cL0" ///< Строка для отображения режима level_volume = 0 (без звука)
#define LEVEL_VOLUME_1                                                         \
  "cL1" ///< Строка для отображения режима level_volume = 1 (минимальная
        ///< громкость)
#define LEVEL_VOLUME_2                                                         \
  "cL2" ///< Строка для отображения режима level_volume = 2 (средняя громкость)
#define LEVEL_VOLUME_3                                                         \
  "cL3" ///< Строка для отображения режима level_volume = 3 (максимальная
        ///< громкость)

/**
 * Режимы настроек в меню (адрес индикатора, уровенб громкости, выход из меню -
 * сохранение настроек).
 */
typedef enum { ID = 0, LEVEL_VOLUME, ESC } settings_mode_t;

/// Флаг для детектирования первого нажатия кнопки 1 (вход в меню - считывание
/// настроек из flash-памяти).
volatile bool is_first_btn_clicked = true;

/// Счетчик кол-ва нажатий BUTTON_1
volatile uint8_t btn_1_set_mode_counter = 0;

/// Флаг для контроля состояния BUTTON_1
volatile bool is_button_1_pressed = false;

/// Флаг для контроля состояния BUTTON_2
volatile bool is_button_2_pressed = false;

/// Счетчик кол-ва нажатий BUTTON_2
volatile uint8_t btn_2_set_value_counter = 0;

/// Флаг для контроля проигрывания гонга прибытия для level_volume_0
static bool is_level_volume_0_displayed = false;

/// Флаг для контроля проигрывания гонга прибытия для level_volume_1
static bool is_level_volume_1_displayed = false;

/// Флаг для контроля проигрывания гонга прибытия для level_volume_2
static bool is_level_volume_2_displayed = false;

/// Флаг для контроля проигрывания гонга прибытия для level_volume_3
static bool is_level_volume_3_displayed = false;

/**
 * @brief Сброс флагов контроля проигрывания гонга прибытия для всех уровней
 *        громкости.
 */
static void reset_volume_flags() {
  is_level_volume_0_displayed = false;
  is_level_volume_1_displayed = false;
  is_level_volume_2_displayed = false;
  is_level_volume_3_displayed = false;
}

/**
 * @brief  Обработка прерываний для кнопок.
 *         BUTTON_1_Pin и BUTTON_2_Pin используются для меню.
 * @param  GPIO_Pin: Пины кнопок на EXTI линии.
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  /* Текущее состояние матрицы: MATRIX_STATE_START,
   MATRIX_STATE_WORKING, MATRIX_STATE_MENU */
  extern matrix_state_t matrix_state;

  /// Счетчик прошедшего в мс времени между последними нажатиями кнопок.
  extern uint32_t time_since_last_press_ms;

  /* Нажатие кнопки 1 */
  if (GPIO_Pin == BUTTON_1_Pin) {
    is_button_1_pressed = true;
    matrix_state = MATRIX_STATE_MENU; // Переход индикатора в режим меню
    btn_1_set_mode_counter++; // Счетчик для определения текущего режима меню
    is_interface_connected = false; // Сброс флага подключения интерфейса
  }

  /* Нажатие кнопки 2 */
  if (GPIO_Pin == BUTTON_2_Pin) {
    is_button_2_pressed = true;

    if (btn_1_set_mode_counter == 0) {
      btn_2_set_value_counter = 0;
    } else {
      btn_2_set_value_counter++;
    }
  }

  /* Если нажата кнопка 1 или кнопка 2, то остаемся в режиме меню, сбрасываем
   * счетчик времени пребывания в режиме меню */
  if (GPIO_Pin == BUTTON_1_Pin || GPIO_Pin == BUTTON_2_Pin) {
    time_since_last_press_ms = 0;
  }

#if PROTOCOL_NKU_SD7
  /// Flag to control is start bit is received (state DATA_Pin from 1 to 0)
  extern bool is_start_bit_received;

  /// Buffer with timings for reading data bits
  extern const uint16_t nku_sd7_timings[];

  extern volatile uint8_t bit_index;

  extern volatile uint8_t current_byte;

  extern volatile bool is_tim3_period_elapsed;

  extern volatile bool is_time_start;

  if (GPIO_Pin == DATA_Pin) {
    if (matrix_state == MATRIX_STATE_WORKING) {
      if (!is_start_bit_received) {
        if (HAL_GPIO_ReadPin(DATA_GPIO_Port, DATA_Pin) == GPIO_PIN_RESET) {
          is_start_bit_received = true;
          HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
          TIM3_Start(PRESCALER_FOR_US, (nku_sd7_timings[0]) / 2);
        }
      }
    }
  }

#endif
}

/// Текущий режим меню
static settings_mode_t btn_1_settings_mode = ID;

/// Уровень громкости из flash-памяти
static uint8_t level_volume = 1;

/// Адрес индикатора  из flash-памяти
static uint8_t id = 1;

/// Выбранный в меню vol (уровень громкости) для звуковых оповещений
static uint8_t selected_level_volume;

/// Выбранный в меню id (адрес) для индикатора
static uint8_t selected_id;

/// Структура с данными для отображения
static drawing_data_t drawing_data = {0, 0};

/**
 * @brief  Обработка нажатий BUTTON_1 и BUTTON_2.
 * @note   Когда BUTTON_1 нажата 1 раз, то индикатор переходит в состояние меню
 *         matrix_state = MATRIX_STATE_MENU,
 *         BUTTON_1 позволяет выбирать режим меню: ID (адрес индикатора), VOLUME
 *                  (уровень громкости), ESCAPE (выход из меню С сохранением
 *                  выбранных значений).
 *         BUTTON_2 позволяет выбрать значение для ID, VOLUME.
 * @param  None
 * @retval None
 */
void press_button() {
  /// Текущее состояние меню: MENU_STATE_OPEN, MENU_STATE_WORKING,
  /// MENU_STATE_CLOSE
  extern menu_state_t menu_state;

  /* При первом нажатии на кнопку 1 считываем значения настроек из flash-памяти
   */
  if (is_first_btn_clicked) {
    reset_volume_flags();

    is_first_btn_clicked = false;

    btn_2_set_value_counter = 0;

    bool is_id_from_flash_valid = matrix_settings.addr_id >= ADDR_ID_MIN &&
                                  matrix_settings.addr_id <= ADDR_ID_LIMIT;

    id = is_id_from_flash_valid ? matrix_settings.addr_id : MAIN_CABIN_ID;

    if (matrix_settings.volume != VOLUME_0 &&
        matrix_settings.volume != VOLUME_1 &&
        matrix_settings.volume != VOLUME_2 &&
        matrix_settings.volume != VOLUME_3) {
      level_volume = 1;
    } else {
      switch (matrix_settings.volume) {
      case VOLUME_0:
        level_volume = 0;
        break;
      case VOLUME_1:
        level_volume = 1;
        break;
      case VOLUME_2:
        level_volume = 2;
        break;
      case VOLUME_3:
        level_volume = 3;
        break;
      }
    }

    selected_level_volume = level_volume;
    selected_id = id;
  }

  /* Нажатие кнопки 1: переключение режимов меню */
  if (is_button_1_pressed) {
    is_button_1_pressed = false;

    switch (btn_1_set_mode_counter) {
    case 1:

      /* Режим VOL (уровень громкости) */
      while (btn_1_set_mode_counter == 1 && btn_2_set_value_counter == 0) {
        draw_string_on_matrix(SETTINGS_MODE_VOLUME);
        btn_1_settings_mode = LEVEL_VOLUME;
      }

      break;
    case 2:

      /* Режим ID (адрес индмкатора) */
      while (btn_1_set_mode_counter == 2 && btn_2_set_value_counter == 0) {
        draw_string_on_matrix(SETTINGS_MODE_ID);
        btn_1_settings_mode = ID;
      }

      /* Для режима LEVEL_VOLUME: возврат к режиму LEVEL_VOLUME
       * ПОСЛЕ выбора значения уровня громкости */
      while (btn_1_settings_mode == LEVEL_VOLUME &&
             btn_1_set_mode_counter == 2 && btn_2_set_value_counter == 1) {
        draw_string_on_matrix(SETTINGS_MODE_VOLUME);
      }

      /* Возврат к выбору значения уровня громкости в режиме LEVEL_VOLUME */
      if (btn_2_set_value_counter == 2) {
        btn_1_set_mode_counter = 1;
        btn_2_set_value_counter = 1;
        level_volume = selected_level_volume;

        reset_volume_flags();
      }

      break;

    case 3:

      /* Для режима LEVEL_VOLUME: возврат к режиму LEVEL_VOLUME */
      if (btn_1_settings_mode == LEVEL_VOLUME) {
        btn_2_set_value_counter = 0;
        level_volume = selected_level_volume;

        reset_volume_flags();
      }

      while (btn_1_set_mode_counter == 3 && btn_2_set_value_counter == 0) {
        draw_string_on_matrix(SETTINGS_MODE_ESC);
        btn_1_settings_mode = ESC;
      }

      if (btn_2_set_value_counter == 0) {
        btn_1_set_mode_counter = 1;
      } else {
        /* Для режима ID: возврат к режиму ID
         * ПОСЛЕ выбора значения адреса */
        while (btn_1_settings_mode == ID && btn_1_set_mode_counter == 3 &&
               btn_2_set_value_counter == 1) {
          draw_string_on_matrix(SETTINGS_MODE_ID);
        }

        /* Возврат к режиму ID */
        if (btn_1_set_mode_counter == 4) {
          btn_1_set_mode_counter = 1;
          btn_2_set_value_counter = 0;
          id = selected_id;
        }

        /* Возврат к выбору значений в режиме ID */
        if (btn_2_set_value_counter == 2) {
          btn_1_set_mode_counter = 2;
          btn_2_set_value_counter = 1;
          id = selected_id;
        }
      }

      break;
    }
  }

  /* Нажатие кнопки 2 */
  if (is_button_2_pressed) {
    is_button_2_pressed = false;

    switch (btn_2_set_value_counter) {
    case 1:

      if (btn_1_set_mode_counter == 0) {
        btn_2_set_value_counter = 0;
      } else {
        switch (btn_1_settings_mode) {
        case LEVEL_VOLUME: /* Выбор значения для уровня громкости */
          selected_level_volume = level_volume;
          while (btn_1_settings_mode == LEVEL_VOLUME &&
                 btn_2_set_value_counter == 1 && btn_1_set_mode_counter == 1) {
            if (level_volume == 0) {
              draw_string_on_matrix(LEVEL_VOLUME_0);
              play_bip_for_menu(&is_level_volume_0_displayed, VOLUME_0);

              is_level_volume_3_displayed = false;
            }

            if (level_volume == 1) {
              draw_string_on_matrix(LEVEL_VOLUME_1);
              play_bip_for_menu(&is_level_volume_1_displayed, VOLUME_1);

              is_level_volume_0_displayed = false;
            }
            if (level_volume == 2) {
              draw_string_on_matrix(LEVEL_VOLUME_2);
              play_bip_for_menu(&is_level_volume_2_displayed, VOLUME_2);

              is_level_volume_1_displayed = false;
            }
            if (level_volume == 3) {
              draw_string_on_matrix(LEVEL_VOLUME_3);
              play_bip_for_menu(&is_level_volume_3_displayed, VOLUME_3);

              is_level_volume_2_displayed = false;
            }
          }

          level_volume++;
          if (level_volume > VOLUME_LEVEL_LIMIT) {
            level_volume = 0;
          }
          break;

        case ID: /* Выбор значения адреса для индикатора */

          if (id == MAIN_CABIN_ID) {
            matrix_string[DIRECTION] = 'c';
            matrix_string[MSB] = 'K';
            matrix_string[LSB] = 'c';
          } else {
            drawing_data.floor = id;
            setting_symbols(matrix_string, &drawing_data, ADDR_ID_LIMIT, NULL,
                            0);
          }

          selected_id = id;
          while (btn_1_settings_mode == ID && btn_2_set_value_counter == 1 &&
                 btn_1_set_mode_counter == 2) {
            draw_string_on_matrix(matrix_string);
          }

          /* Установка следующего за отображенным id */
          if (id == ADDR_ID_LIMIT) {
            id = ADDR_ID_MIN;
          } else if (id == MAX_POSITIVE_NUMBER_FLOOR) {
            id = MAIN_CABIN_ID;
          } else {
            id++;
          }

          break;

        case ESC:

          /* Выход С СОХРАНЕНИЕМ выбранных значений */
          switch (selected_level_volume) {
          case 0:
            update_structure(&matrix_settings, VOLUME_0, selected_id);
            break;
          case 1:
            update_structure(&matrix_settings, VOLUME_1, selected_id);
            break;
          case 2:
            update_structure(&matrix_settings, VOLUME_2, selected_id);
            break;
          case 3:
            update_structure(&matrix_settings, VOLUME_3, selected_id);
            break;
          }

          btn_1_set_mode_counter = 0;
          btn_2_set_value_counter = 0;
          is_first_btn_clicked = true;
          matrix_string[DIRECTION] = 'c';
          matrix_string[MSB] = 'c';
          matrix_string[LSB] = 'c';
          menu_state = MENU_STATE_CLOSE;
          break;
        }

        if (btn_1_settings_mode != ESC) {
          btn_2_set_value_counter = 1;
        }

        break;
      }
    }
  }
}
