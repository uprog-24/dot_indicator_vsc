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

#define SETTINGS_MODE_ID_GROUP                                                 \
  "IDG" ///< String that displayed in settings_mode_t = ESC

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
 * Режимы настроек в меню (адрес индикатора, уровень громкости, выход из меню,
 * адрес группы (НКУ-CAN, Alpaca) - сохранение настроек).
 */
typedef enum { ID = 0, LEVEL_VOLUME, ESC, GROUP_ID } settings_mode_t;

/// Флаг для детектирования первого нажатия кнопки 1 (вход в меню - считывание
/// настроек из flash-памяти).
volatile bool is_first_btn_clicked = true;

/// Флаг для контроля состояния BUTTON_1
volatile bool is_button_1_pressed = false;

/// Флаг для контроля состояния BUTTON_2
volatile bool is_button_2_pressed = false;

/// Флаг для контроля проигрывания гонга прибытия для level_volume_0
static bool is_level_volume_0_displayed = false;

/// Флаг для контроля проигрывания гонга прибытия для level_volume_1
static bool is_level_volume_1_displayed = false;

/// Флаг для контроля проигрывания гонга прибытия для level_volume_2
static bool is_level_volume_2_displayed = false;

/// Флаг для контроля проигрывания гонга прибытия для level_volume_3
static bool is_level_volume_3_displayed = false;

/// Flag to control BUTTON_1 state
volatile bool is_button_2_pressed_first = true;

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
 * @brief Открытие меню настроек.
 *
 * @param matrix_state Указатель на состояние индикатора.
 */
static void go_to_menu(matrix_state_t *matrix_state) {
  *matrix_state = MATRIX_STATE_MENU; // Переход индикатора в режим меню
  is_button_1_pressed =
      true; // Нажатие кнопки (обработка в функции press_button())
  is_interface_connected = false;
}

extern volatile bool is_time_sec_for_settings_elapsed;
extern matrix_state_t matrix_state;

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
    matrix_state = MATRIX_STATE_MENU; // Переход индикатора в режим меню
    is_button_1_pressed = true;
    is_interface_connected = false; // Сброс флага подключения интерфейса
  }

  /* Нажатие кнопки 2 */
  if (GPIO_Pin == BUTTON_2_Pin) {
    is_button_2_pressed = true;
  }

  /* Если нажата кнопка 1 или кнопка 2, то остаемся в режиме меню, сбрасываем
   * счетчик времени пребывания в режиме меню */
  if (GPIO_Pin == BUTTON_1_Pin || GPIO_Pin == BUTTON_2_Pin) {
    time_since_last_press_ms = 0;
    is_time_sec_for_settings_elapsed = false;
  }

#if PROTOCOL_NKU_SD7
  /// Flag to control is start bit is received (state DATA_Pin from 1 to 0)
  extern bool is_start_bit_received;

  extern const uint16_t nku_sd7_timing;

  // Детектируем спад по фронту из 1 в 0 (старт-бит для NKU_SD7)
  if (GPIO_Pin == DATA_Pin) {
    if (matrix_state == MATRIX_STATE_WORKING) {
      if (!is_start_bit_received) {
        if (HAL_GPIO_ReadPin(DATA_GPIO_Port, DATA_Pin) == GPIO_PIN_RESET) {
          is_start_bit_received = true;
          HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
          TIM3_Start(PRESCALER_FOR_US, nku_sd7_timing / 2);
        }
      }
    }
  }

#endif
}

/**
 * @brief Обновление настроек матрицы (адрес и громкость).
 *
 * @param selected_level_volume Выбранная громкость звуковых оповещений.
 * @param selected_id Выбранный адрес индикатора.
 */
void update_matrix_settings(uint8_t *selected_level_volume,
                            uint8_t *selected_id, uint8_t *selected_group_id) {
  switch (*selected_level_volume) {
  case 0:
    update_structure(&matrix_settings, VOLUME_0, *selected_id,
                     *selected_group_id);
    break;
  case 1:
    update_structure(&matrix_settings, VOLUME_1, *selected_id,
                     *selected_group_id);
    break;
  case 2:
    update_structure(&matrix_settings, VOLUME_2, *selected_id,
                     *selected_group_id);
    break;
  case 3:
    update_structure(&matrix_settings, VOLUME_3, *selected_id,
                     *selected_group_id);
    break;
  }
}

/// Текущий режим меню
static settings_mode_t btn_1_settings_mode = ID;

/// Уровень громкости из flash-памяти
static uint8_t flash_level_volume = 1;

/// Адрес индикатора  из flash-памяти
static uint8_t flash_id = 1;

/* Адрес группы из flash */
static uint8_t flash_group_id;

/// Выбранный в меню vol (уровень громкости) для звуковых оповещений
static uint8_t selected_level_volume;

/// Выбранный в меню id (адрес) для индикатора
static uint8_t selected_id;

static uint8_t selected_group_id;

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

typedef enum {
  BUTTON_NONE = 0,
  BUTTON_1_PRESSED,
  BUTTON_2_PRESSED
} button_state_t;

typedef enum {
  MENU_STATE_IDLE = 0, // Ожидание
  MENU_STATE_VOLUME,   // Уровень громкости
  MENU_STATE_ID,       // Настройка ID
  MENU_STATE_GROUP_ID, // Настройка группы
  MENU_STATE_EXIT      // Выход
} menu_state_regimes_t;

/**
 * @brief  Действия при выходе из меню.
 */
typedef enum {
  NOT_SAVE_SETTINGS, ///< Не сохранять настройки
  SAVE_SETTINGS      ///< Сохранить настройки
} menu_exit_actions_t;

typedef struct {
  menu_state_regimes_t current_state; // Текущее состояние меню
  button_state_t button_1;            // Состояние кнопки 1
  button_state_t button_2;            // Состояние кнопки 2
} menu_context_t;

static menu_context_t menu = {.current_state = MENU_STATE_IDLE,
                              .button_1 = BUTTON_NONE,
                              .button_2 = BUTTON_NONE};

void set_new_selected_id(uint8_t *selected_id) {
#if PROTOCOL_UIM_6100
  if (*selected_id == 47) {
    *selected_id = 1;
  } else if (*selected_id == 40) {
    *selected_id = MAIN_CABIN_ID;
  } else {
    (*selected_id)++;
  }

#elif PROTOCOL_UKL
  if (*selected_id == ADDR_ID_LIMIT) {
    *selected_id = ADDR_ID_MIN;
  } else if (*selected_id == 55) {
    *selected_id = 57;
  } else {
    (*selected_id)++;
  }
#elif PROTOCOL_NKU || PROTOCOL_ALPACA || PROTOCOL_NKU_SD7
  if (*selected_id == ADDR_ID_LIMIT) {
    *selected_id = ADDR_ID_MIN;
  } else {
    (*selected_id)++;
  }
#elif PROTOCOL_ALPACA
  // selected_id++;
  // if (selected_id > ADDR_ID_LIMIT) {
  //   selected_id = ADDR_ID_MIN;
  // }
#endif
}

void set_symbols_id(uint8_t *selected_id, drawing_data_t *drawing_data) {
#if PROTOCOL_UKL
  if (*selected_id >= 57 && *selected_id <= 59) {
    matrix_string[DIRECTION] = 'c';
    matrix_string[MSB] = 'p';
    matrix_string[LSB] = (*selected_id == 57)
                             ? 'c'
                             : convert_int_to_char((*selected_id) % 10 - 7);
  } else if (*selected_id >= 60 && *selected_id <= 63) {
    matrix_string[DIRECTION] = 'c';
    matrix_string[MSB] = '-';
    matrix_string[LSB] = convert_int_to_char((*selected_id) % 10 + 1);
  } else

#elif PROTOCOL_ALPACA

#elif PROTOCOL_NKU_SD7
  if (*selected_id == MAIN_CABIN_ID) {
    set_symbols(SYMBOL_EMPTY, SYMBOL_K, SYMBOL_EMPTY);
  } else {
    set_symbols(SYMBOL_EMPTY,
                (*selected_id < 10) ? (*selected_id) % 10 : (*selected_id) / 10,
                (*selected_id < 10) ? SYMBOL_EMPTY : (*selected_id) % 10);
  }
#endif
}

void set_symbols_extra_mode(uint8_t *selected_group_id,
                            drawing_data_t *drawing_data) {
#if PROTOCOL_ALPACA
  if (*selected_group_id == ADDR_ID_MIN) {
    matrix_string[DIRECTION] = 'c';
    matrix_string[MSB] = '0';
    matrix_string[LSB] = 'c';
  } else if (*selected_group_id <= MAX_P_FLOOR_SHIFT_INDEX) {
    /* shift = group_id = 1...9 и 10 -> П1..П9 и П10 */
    matrix_string[DIRECTION] =
        (*selected_group_id <= MAX_P_FLOOR_SHIFT_INDEX) ? 'c' : 'p';
    matrix_string[MSB] = (*selected_group_id <= MAX_P_FLOOR_SHIFT_INDEX)
                             ? 'p'
                             : convert_int_to_char((*selected_group_id) / 10);
    matrix_string[LSB] = (*selected_group_id <= MAX_P_FLOOR_SHIFT_INDEX)
                             ? convert_int_to_char(*selected_group_id)
                             : convert_int_to_char((*selected_group_id) % 10);
  } else if (*selected_group_id >= MIN_MINUS_FLOOR_SHIFT_INDEX) {
    /* shift = group_id = 11... -> вычитаем MAX_P_FLOOR_SHIFT_INDEX
     * (тк от 0 до 10 идут этажи П1..П10) -> -1... */

    matrix_string[DIRECTION] = '-';
    matrix_string[MSB] =
        (*selected_group_id <= 19)
            ? convert_int_to_char((*selected_group_id) -
                                  MAX_P_FLOOR_SHIFT_INDEX)
            : convert_int_to_char(
                  ((*selected_group_id) - MAX_P_FLOOR_SHIFT_INDEX) / 10);
    matrix_string[LSB] =
        (*selected_group_id <= 19)
            ? 'c'
            : convert_int_to_char(
                  ((*selected_group_id) - MAX_P_FLOOR_SHIFT_INDEX) % 10);
  }
#elif PROTOCOL_NKU
  drawing_data->floor = *selected_group_id;
  setting_symbols(matrix_string, drawing_data, ADDR_ID_LIMIT, NULL, 0);
#endif
}

/// Current menu state: MENU_STATE_OPEN, MENU_STATE_WORKING,
/// MENU_STATE_CLOSE
extern menu_state_t menu_state;

extern uint32_t time_since_last_press_ms;

/**
 * @brief Выход из меню с сохранением/без сохранения настроек в памяти
 * устройства
 *
 * @param menu_state
 * @param menu_exit_action
 */
void menu_exit(menu_state_t *menu_state, menu_exit_actions_t menu_exit_action) {

  switch (menu_exit_action) {
  case NOT_SAVE_SETTINGS:

    is_first_btn_clicked = true;
    time_since_last_press_ms = 0;

    matrix_state = MATRIX_STATE_START;
    *menu_state = MENU_STATE_OPEN;
    menu.current_state = MENU_STATE_IDLE;
    break;

  case SAVE_SETTINGS:
/* Обновление структуры с данными о настройках: громкость, адрес, адрес группы
 */
#if PROTOCOL_NKU || PROTOCOL_ALPACA
    update_matrix_settings(&selected_level_volume, &selected_id,
                           &selected_group_id);
#else
    /* Обновление структуры с данными о настройках: громкость, адрес, адрес
     * группы не нужен */
    update_matrix_settings(&selected_level_volume, &selected_id, 0);
#endif

    /* Сброс флагов, изменение состояний */

    is_first_btn_clicked = true;
    time_since_last_press_ms = 0;

    *menu_state = MENU_STATE_CLOSE;

    break;
  }
}

void handle_button_press(menu_context_t *menu, button_state_t button) {

  switch (menu->current_state) {
    /* ===== Состояние ожидания ===== */
  case MENU_STATE_IDLE:
    if (button == BUTTON_1_PRESSED) {
    }
    break;

    /*===== Режим меню VOL (Уровень громкости) =====*/
  case MENU_STATE_VOLUME:
    if (button == BUTTON_1_PRESSED) {
      stop_buzzer_sound();
      is_button_2_pressed_first = true;

      while (is_button_1_pressed == false && is_button_2_pressed == false &&
             is_time_sec_for_settings_elapsed != true) {
        draw_string(SETTINGS_MODE_VOLUME);
      }

      /* Переход к следующему режиму меню (к адресу) */
      if (is_button_1_pressed) {
        menu->current_state = MENU_STATE_ID;
      }

    } else if (button == BUTTON_2_PRESSED) { // Изменение VOL

      /* Если кнопка нажата 1-ый раз, то отображаем уровень из flash-памяти;
       * если кнопка нажата НЕ 1-ый раз, то изменяем счетчик текущего выбранного
       * уровня. */
      if (is_button_2_pressed_first) {
        selected_level_volume =
            flash_level_volume; // Показываем сохранённый уровень
      } else {
        selected_level_volume++; // Увеличиваем уровень
        if (selected_level_volume > VOLUME_LEVEL_LIMIT) {
          selected_level_volume = 0; // Сброс до 0 при превышении лимита
        }
      }

      /* Отображаем текущий выбранный/сохраненный уровень */
      while (is_button_1_pressed == false && is_button_2_pressed == false &&
             is_time_sec_for_settings_elapsed != true) {
        switch (selected_level_volume) {
        case 0:

          // draw_symbols(SYMBOL_EMPTY, SYMBOL_L, SYMBOL_ZERO);
          draw_string(LEVEL_VOLUME_0);

          play_bip_for_menu(&is_level_volume_0_displayed, VOLUME_0);
          is_level_volume_3_displayed = false;
          break;

        case 1:
          draw_string(LEVEL_VOLUME_1);

          play_bip_for_menu(&is_level_volume_1_displayed, VOLUME_1);
          is_level_volume_0_displayed = false;
          break;

        case 2:
          draw_string(LEVEL_VOLUME_2);

          play_bip_for_menu(&is_level_volume_2_displayed, VOLUME_2);
          is_level_volume_1_displayed = false;
          break;

        case 3:
          draw_string(LEVEL_VOLUME_3);

          play_bip_for_menu(&is_level_volume_3_displayed, VOLUME_3);
          is_level_volume_2_displayed = false;
          break;

        default:
          break;
        }
      }

      if (!is_button_2_pressed_first) {
        flash_level_volume =
            selected_level_volume; // Сохраняем выбранное значение
      }
      is_button_2_pressed_first = false;
      reset_volume_flags();
    }
    break;
    /*===== Завершение: Режим меню VOL (Уровень громкости) =====*/

    /*===== Режим меню ID (Адрес) =====*/
  case MENU_STATE_ID:
    if (button == BUTTON_1_PRESSED) {

      is_button_2_pressed_first = true;

      // set_symbols(SYMBOL_EMPTY, SYMBOL_I, SYMBOL_D);

      while (is_button_1_pressed == false && is_button_2_pressed == false &&
             is_time_sec_for_settings_elapsed != true) {
        // draw_symbols(SYMBOL_EMPTY, SYMBOL_I, SYMBOL_D);
        draw_string(SETTINGS_MODE_ID);
      }

      /* Переход к следующему режиму меню */
      if (is_button_1_pressed) {
        /* Для НКУ_CAN следующий режим: Адрес группы; для Альпака - Сдвиг */
#if PROTOCOL_NKU || PROTOCOL_ALPACA
        menu->current_state = MENU_STATE_GROUP_ID;
#else /* Для остальных протоколов следующий режим: Выход */
        menu->current_state = MENU_STATE_EXIT;
#endif
      }
    } else if (button == BUTTON_2_PRESSED) { // Изменение ID

      /* Если кнопка нажата 1-ый раз, то отображаем адрес из flash-памяти;
       * если кнопка нажата НЕ 1-ый раз, то изменяем счетчик текущего выбранного
       * адреса. */
      if (is_button_2_pressed_first) {
        selected_id = flash_id; // Показываем сохранённый адрес
      } else {
        /* Сохраняем новое значение адреса в переменную selected_id */
        set_new_selected_id(&selected_id);
      }

      /* Настройка символов для отображения ID */
      set_symbols_id(&selected_id, &drawing_data);

      /* Отображаем текущий выбранный/сохраненный адрес */
      while (is_button_1_pressed == false && is_button_2_pressed == false &&
             is_time_sec_for_settings_elapsed != true) {
        draw_symbols();
      }

      if (!is_button_2_pressed_first) {
        flash_id = selected_id; // Сохраняем выбранное значение
      }
      is_button_2_pressed_first = false;
    }
    break;
    /*===== Завершение: Режим меню ID (Адрес) =====*/

    /*===== Режим меню GROUP_ID (Адрес группы; для НКУ_CAN, Alpaca) =====*/
  case MENU_STATE_GROUP_ID:
    if (button == BUTTON_1_PRESSED) {

      is_button_2_pressed_first = true;

      /* Отображаем текущий выбранный/сохраненный адрес группы */
      while (is_button_1_pressed == false && is_button_2_pressed == false &&
             is_time_sec_for_settings_elapsed != true) {
#if PROTOCOL_NKU
        draw_string_on_matrix(SETTINGS_MODE_ID_GROUP);
#elif PROTOCOL_ALPACA
        draw_string_on_matrix(SETTINGS_MODE_SHFT);
#endif
      }

      /* Переход к следующему режиму меню (к выходу из меню) */
      if (is_button_1_pressed) {
        menu->current_state = MENU_STATE_EXIT;
      }
    } else if (button == BUTTON_2_PRESSED) { // Изменение адреса группы

      /* Если кнопка нажата 1-ый раз, то отображаем адрес из flash-памяти;
       * если кнопка нажата НЕ 1-ый раз, то изменяем счетчик текущего выбранного
       * адреса. */
      if (is_button_2_pressed_first) {
        selected_group_id =
            flash_group_id; // Показываем сохранённый адрес группы
      } else {

#if PROTOCOL_NKU || PROTOCOL_ALPACA
        if (selected_group_id == GROUP_ID_MAX) {
          selected_group_id = GROUP_ID_MIN;
        } else {
          selected_group_id++;
        }
#endif
      }

      /* Настройка символов для отображения GROUP_ID (НКУ), SHIFT (Альпака) */
      set_symbols_extra_mode(&selected_group_id, &drawing_data);

      /* Отображаем текущий выбранный/сохраненный адрес группы */
      while (is_button_1_pressed == false && is_button_2_pressed == false &&
             is_time_sec_for_settings_elapsed != true) {
        // draw_string_on_matrix(matrix_string);
      }

      if (!is_button_2_pressed_first) {
        flash_group_id = selected_group_id; // Сохраняем выбранное значение
      }
      is_button_2_pressed_first = false;
    }
    break;
    /*===== Завершение: Режим меню GROUP_ID (Адрес группы) =====*/

  /*===== Режим меню ESC (Выход) =====*/
  case MENU_STATE_EXIT:
    if (button == BUTTON_1_PRESSED) {

      while (is_button_1_pressed == false && is_button_2_pressed == false &&
             is_time_sec_for_settings_elapsed != true) {
        draw_string(SETTINGS_MODE_ESC);
      }

      /* Переход к следующему режиму меню (к уровню громкости) */
      if (is_button_1_pressed) {
        menu->current_state = MENU_STATE_VOLUME;
      }
    } else if (button ==
               BUTTON_2_PRESSED) { // Выход из меню с сохранением настроек
      menu_exit(&menu_state, SAVE_SETTINGS);
    }

    break;
    /*===== Завершение: Режим меню ESC (Выход) =====*/
  }
}

void press_button() {

#if !DEMO_MODE && !TEST_MODE

  if (is_first_btn_clicked) {

    menu.current_state = MENU_STATE_VOLUME;

    reset_volume_flags();

    is_first_btn_clicked = false;

    /* Инициализация переменных для Адреса */
    bool is_id_from_flash_valid = matrix_settings.addr_id >= ADDR_ID_MIN &&
                                  matrix_settings.addr_id <= ADDR_ID_LIMIT;

    flash_id = is_id_from_flash_valid ? matrix_settings.addr_id : MAIN_CABIN_ID;

    /* Инициализация переменных для Уровня громкости */
    if (matrix_settings.volume != VOLUME_0 &&
        matrix_settings.volume != VOLUME_1 &&
        matrix_settings.volume != VOLUME_2 &&
        matrix_settings.volume != VOLUME_3) {
      flash_level_volume = 1;
    } else {
      switch (matrix_settings.volume) {
      case VOLUME_0:
        flash_level_volume = 0;
        break;
      case VOLUME_1:
        flash_level_volume = 1;
        break;
      case VOLUME_2:
        flash_level_volume = 2;
        break;
      case VOLUME_3:
        flash_level_volume = 3;
        break;
      }
    }

    selected_level_volume = flash_level_volume;
    selected_id = flash_id;
    selected_group_id = flash_group_id;
  }

#if 1
  /* Выход из меню по истечении PERIOD_SEC_FOR_SETTINGS секунд бездействия в
   * меню (БЕЗ сохранения настроек) */
  if (is_time_sec_for_settings_elapsed) {
    is_time_sec_for_settings_elapsed = false;

    menu_exit(&menu_state, NOT_SAVE_SETTINGS);
  }
#endif

  /*===== Нажатие кнопки 1 =====*/
  if (is_button_1_pressed) {
    is_button_1_pressed = false;
    handle_button_press(&menu, BUTTON_1_PRESSED);
  }

  /*===== Нажатие кнопки 2 =====*/
  if (is_button_2_pressed) {
    is_button_2_pressed = false;
    handle_button_press(&menu, BUTTON_2_PRESSED);
  }

#endif
}
