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
  "V0L" ///< String that displayed in settings_mode_t = VOL
#define SETTINGS_MODE_ID                                                       \
  "cID" ///< String that displayed in settings_mode_t = ID
#define SETTINGS_MODE_SFT                                                      \
  "SHF" ///< String that displayed in mode ID when shift for floor location
        ///< (Alpaca)
#define SETTINGS_MODE_ESC                                                      \
  "ESC" ///< String that displayed in settings_mode_t = ESC
#define SETTINGS_MODE_ID_GROUP                                                 \
  "IDG" ///< String that displayed in settings_mode_t = ESC

#define LEVEL_VOLUME_0 "cL0" ///< String that displayed when level_volume = 0
#define LEVEL_VOLUME_1 "cL1" ///< String that displayed when level_volume = 1
#define LEVEL_VOLUME_2 "cL2" ///< String that displayed when level_volume = 2
#define LEVEL_VOLUME_3 "cL3" ///< String that displayed when level_volume = 3

#define SHIFT_P_FLOOR "pFL"
#define SHIFT_MINUS_FLOOR "-FL"

#if DOT_SPI
#define DEBOUNCE_DELAY 150 // Задержка в миллисекундах для фильтрации дребезга
#define LONG_PRESS_TIME 1000 // 1000 мс - долгое нажатие

uint8_t button_state = 1; // Состояние кнопки (1 - отпущена, 0 - нажата)
volatile uint32_t button_press_time =
    0; // Время удержания кнопки (в миллисекундах)

uint32_t last_time = 0; // Время последнего изменения состояния

volatile bool is_long_press_detected = false;
volatile bool is_short_press_detected = false;

volatile bool is_button_pressed = false; // Flag for button state
#endif

/**
 * Stores modes of settings
 */
typedef enum { ID = 0, LEVEL_VOLUME, ESC, GROUP_ID } settings_mode_t;

/// Flag to control first btn1 click
volatile bool is_first_btn_clicked = true;

/// Counter for pressing of BUTTON_1
volatile uint8_t btn_1_set_mode_counter = 0;

/// Flag to control BUTTON_1 state
volatile bool is_button_1_pressed = false;

/// Flag to control BUTTON_2 state
volatile bool is_button_2_pressed = false;

/// Counter for pressing of BUTTON_2
volatile uint8_t btn_2_set_value_counter = 0;

/// Flag to control bip for level_volume_0
static bool is_level_volume_0_displayed = false;

/// Flag to control bip for level_volume_1
static bool is_level_volume_1_displayed = false;

/// Flag to control bip for level_volume_2
static bool is_level_volume_2_displayed = false;

/// Flag to control bip for level_volume_3
static bool is_level_volume_3_displayed = false;

/// Flag to control BUTTON_1 state
volatile bool is_button_2_pressed_first = true;

/**
 * @brief Сброс флагов для отработки гонгов при выборе уровня громкости
 * (L0-L3) в меню
 *
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
  btn_1_set_mode_counter++;
  is_interface_connected = false;
}

/**
 * @brief  Handle Interrupt by EXTI line, setting the state when button is
 *         pressed.
 *         1. BUTTON_1_Pin and BUTTON_2_Pin are used for MENU;
 *         2. If (GPIO_Pin == DATA_Pin) then read the pin state until the bit
 *            changes from 1 to 0, set is_start_bit_received;
 *         3. Start TIM3 with timing for first data bit and set disable IRQ.
 * @param  GPIO_Pin: Button's pins connected EXTI line
 * @retval None
 */
/* Counter for elapsed time in seconds between pressing of buttons */
extern uint32_t time_since_last_press_sec;
/* Current matrix state: MATRIX_STATE_INIT, MATRIX_STATE_START,
  MATRIX_STATE_WORKING, MATRIX_STATE_MENU */
extern matrix_state_t matrix_state;
extern volatile bool is_time_sec_for_settings_elapsed;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  // /* Current matrix state: MATRIX_STATE_INIT, MATRIX_STATE_START,
  //  MATRIX_STATE_WORKING, MATRIX_STATE_MENU */
  // extern matrix_state_t matrix_state;

  if (
  /**
   * @brief DOT_PIN: Обработка нажатия BUTTON_1_Pin: вход в меню,
   * переключение режимов ID, VOL, ESC.
   *        DOT_SPI: Обработка нажатия SW_IN_3_Pin: вход в режим
   * выбора адреса - короткое нажатие,
   * вход в режим выбора режима звукового оповещения
   * (З - звук есть/Б - звука нет) - длинное нажатие
   * (переключение - короткое нажатие).
   */
#if DOT_PIN
      GPIO_Pin == BUTTON_1_Pin
#elif DOT_SPI
      GPIO_Pin == SW_IN_3_Pin
#endif
  ) {

#if DOT_SPI
    uint32_t current_time = HAL_GetTick(); // Текущее время в миллисекундах
    if (current_time - last_time > DEBOUNCE_DELAY) {
      uint8_t current_button_state =
          HAL_GPIO_ReadPin(SW_IN_3_GPIO_Port, SW_IN_3_Pin);

      if (current_button_state != button_state) {
        button_state = current_button_state;
        last_time = current_time;

        /* Кнопка нажата (состояние лог. 0)*/
        if (current_button_state == GPIO_PIN_RESET) {
          current_time = HAL_GetTick();
          button_press_time = 0;
          if (!is_button_pressed) { // Чтобы не запускать таймер на каждое
            // прерывание
            is_button_pressed = true; // Устанавливаем флаг нажатия
            button_press_time = 0; // Сброс времени нажатия
            TIM3_Start(PRESCALER_FOR_MS, 1);
          }
        } else {
          /* Кнопка отпущена (состояние лог. 1)*/
          is_button_pressed = false; // Кнопка отпущена
          button_state = 1;
          TIM3_Stop();

          // Проверяем время, в течении которого была нажата кнопка
          if (button_press_time >=
              LONG_PRESS_TIME) { // Если время удержания больше или равно
                                 // LONG_PRESS_TIME в мс
            is_long_press_detected = true;
          } else {
            is_short_press_detected = true;
          }
          button_press_time = 0;
          go_to_menu(&matrix_state);
        }
      }
    }

#elif DOT_PIN
    matrix_state = MATRIX_STATE_MENU;
    is_button_1_pressed = true;
    // matrix_state = MATRIX_STATE_MENU;
    // btn_1_set_mode_counter++;
    is_interface_connected = false;
#endif
  }

  /* Обработка нажатия BUTTON_2_Pin, выбор значения для ID, VOL, ESC */
#if DOT_PIN
  if (GPIO_Pin == BUTTON_2_Pin) {
    is_button_2_pressed = true;
  }
#endif

  /* Сброс времени последнего нажатия кнопки меню */
  if (
#if DOT_PIN
      GPIO_Pin == BUTTON_1_Pin || GPIO_Pin == BUTTON_2_Pin
#elif DOT_SPI
      GPIO_Pin == SW_IN_3_Pin
#endif
  ) {
    time_since_last_press_sec = 0;
    is_time_sec_for_settings_elapsed = false;
  }

#if PROTOCOL_UKL
  /// Flag to control is start bit is received (state DATA_Pin from 1 to 0)
  extern bool is_start_bit_received;

  /// Buffer with timings for reading data bits
  extern const uint16_t ukl_timings[];

  if (GPIO_Pin == DATA_Pin) {
    if (matrix_state == MATRIX_STATE_WORKING) {
      if (!is_start_bit_received) {
        if (HAL_GPIO_ReadPin(DATA_GPIO_Port, DATA_Pin) == GPIO_PIN_RESET) {
          is_start_bit_received = true;
          TIM3_Start(PRESCALER_FOR_US, ukl_timings[0]);
          HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
        }
      }
    }
  }

#endif
}

/// Current (temporary) level of volume
static uint8_t flash_level_volume = 1;

/// Current (temporary) id for interface CAN/UART
static uint8_t flash_id = 1;

/* Адрес группы из flash */
static uint8_t flash_group_id;

/// Selected level of volume
static uint8_t selected_level_volume;

/// Selected id for interface CAN/UART
static uint8_t selected_id;

static uint8_t selected_group_id;

/// Structure for data that will be displayed on matrix
static drawing_data_t drawing_data = {0, 0};

/// Сдвиг для адреса (протокол Alpaca)
uint8_t shift_mode = 1;

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

/**
 * @brief  Handle pressing BUTTON_1 and BUTTON_2.
 * @note   When BUTTON_1 is pressed 1st time - matrix_state = MATRIX_STATE_MENU,
 *         BUTTON_1 allows to select settings_mode_t: ID, VOLUME, ESCAPE.
 *         BUTTON_2 allows to select value for ID, VOLUME.
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
  if (selected_id == 47) {
    selected_id = 1;
  } else if (selected_id == 40) {
    selected_id = MAIN_CABIN_ID;
  } else {
    selected_id++;
  }

#elif PROTOCOL_UKL
  if (selected_id == ADDR_ID_LIMIT) {
    selected_id = ADDR_ID_MIN;
  } else if (selected_id == 55) {
    selected_id = 57;
  } else {
    selected_id++;
  }
#elif PROTOCOL_NKU
  if (*selected_id == ADDR_ID_LIMIT) {
    *selected_id = ADDR_ID_MIN;
  } else {
    (*selected_id)++;
  }
#elif PROTOCOL_ALPACA
  selected_id++;
  if (selected_id > ADDR_ID_LIMIT) {
    selected_id = ADDR_ID_MIN;
  }
#endif
}

void set_symbols_id(uint8_t *selected_id, drawing_data_t *drawing_data) {
#if PROTOCOL_UKL
  if (selected_id >= 57 && selected_id <= 59) {
    matrix_string[DIRECTION] = 'c';
    matrix_string[MSB] = 'p';
    matrix_string[LSB] =
        (selected_id == 57) ? 'c' : convert_int_to_char(selected_id % 10 - 7);
  } else if (selected_id >= 60 && selected_id <= 63) {
    matrix_string[DIRECTION] = 'c';
    matrix_string[MSB] = '-';
    matrix_string[LSB] = convert_int_to_char(selected_id % 10 + 1);
  } else

#elif PROTOCOL_ALPACA
  if (selected_id == ADDR_ID_MIN) {
    matrix_string[DIRECTION] = 'c';
    matrix_string[MSB] = '0';
    matrix_string[LSB] = 'c';
  } else if (selected_id <= MAX_P_FLOOR_ID) {
    // id = 1...9
    if (selected_id < MAX_P_FLOOR_ID) {
      matrix_string[DIRECTION] = 'c';
      matrix_string[MSB] = 'p';
      matrix_string[LSB] = convert_int_to_char(selected_id);
    } else {
      // id = 10
      matrix_string[DIRECTION] = 'p';
      matrix_string[MSB] = convert_int_to_char(selected_id / 10);
      matrix_string[LSB] = convert_int_to_char(selected_id % 10);
    }
  } else if (selected_id >= MIN_MINUS_FLOOR_ID &&
             selected_id <= ADDR_ID_LIMIT) {
    // id = 11...19 -> -10 -> 1...9
    if (selected_id <= 19) {
      matrix_string[DIRECTION] = '-';
      matrix_string[MSB] = convert_int_to_char(selected_id - 10);
      matrix_string[LSB] = 'c';
    } else {
      // id = 20...ADDR_ID_LIMIT -> -10 -> 10...63
      matrix_string[DIRECTION] = '-';
      matrix_string[MSB] = convert_int_to_char((selected_id - 10) / 10);
      matrix_string[LSB] = convert_int_to_char((selected_id - 10) % 10);
    }
  }
#endif

      if (*selected_id == MAIN_CABIN_ID) {
    matrix_string[DIRECTION] = 'c';
    matrix_string[MSB] = 'K';
    matrix_string[LSB] = 'c';
  } else {
    drawing_data->floor = *selected_id;
    setting_symbols(matrix_string, drawing_data, ADDR_ID_LIMIT, NULL, 0);
  }
}

/// Current menu state: MENU_STATE_OPEN, MENU_STATE_WORKING,
/// MENU_STATE_CLOSE
extern menu_state_t menu_state;

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
    time_since_last_press_sec = 0;

    matrix_state = MATRIX_STATE_START;
    *menu_state = MENU_STATE_OPEN;
    menu.current_state = MENU_STATE_IDLE;
    break;

  case SAVE_SETTINGS:
/* Обновление структуры с данными о настройках */
#if PROTOCOL_NKU
    update_matrix_settings(&selected_level_volume, &selected_id,
                           &selected_group_id);
#else
    update_matrix_settings(&selected_level_volume, &selected_id, 0);
#endif

/* Сброс флагов, изменение состояний */
#if DOT_SPI
    is_short_press_detected = false;
    is_long_press_detected = false;

    extern uint32_t time_since_last_press_sec;
    time_since_last_press_sec = 0;
    *menu_state =
        MENU_STATE_CLOSE; // Состояние меню для перезаписи настроек индикатора
    btn_1_set_mode_counter = 0;
    btn_2_set_value_counter = 0;
    is_first_btn_clicked = true;
#elif DOT_PIN

    is_first_btn_clicked = true;
    time_since_last_press_sec = 0;

    matrix_string[DIRECTION] = 'c';
    matrix_string[MSB] = 'c';
    matrix_string[LSB] = 'c';
    *menu_state = MENU_STATE_CLOSE;
#endif
    break;
  }
}

void handle_button_press(menu_context_t *menu, button_state_t button) {

  switch (menu->current_state) {
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
        draw_string_on_matrix(SETTINGS_MODE_VOLUME);
      }

      /* Переход к следующему режиму меню */
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
          draw_string_on_matrix(LEVEL_VOLUME_0);
          play_bip_for_menu(&is_level_volume_0_displayed, VOLUME_0);
          is_level_volume_3_displayed = false;
          break;

        case 1:
          draw_string_on_matrix(LEVEL_VOLUME_1);
          play_bip_for_menu(&is_level_volume_1_displayed, VOLUME_1);
          is_level_volume_0_displayed = false;
          break;

        case 2:
          draw_string_on_matrix(LEVEL_VOLUME_2);
          play_bip_for_menu(&is_level_volume_2_displayed, VOLUME_2);
          is_level_volume_1_displayed = false;
          break;

        case 3:
          draw_string_on_matrix(LEVEL_VOLUME_3);
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

      while (is_button_1_pressed == false && is_button_2_pressed == false &&
             is_time_sec_for_settings_elapsed != true) {
        draw_string_on_matrix(SETTINGS_MODE_ID);
      }

      /* Переход к следующему режиму меню */
      if (is_button_1_pressed) {
#if PROTOCOL_NKU /* Для НКУ следующий режим: Адрес группы */
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
        draw_string_on_matrix(matrix_string);
      }

      if (!is_button_2_pressed_first) {
        flash_id = selected_id; // Сохраняем выбранное значение
      }
      is_button_2_pressed_first = false;
    }
    break;
    /*===== Завершение: Режим меню ID (Адрес) =====*/

    /*===== Режим меню GROUP_ID (Адрес группы; для НКУ) =====*/
  case MENU_STATE_GROUP_ID:
    if (button == BUTTON_1_PRESSED) {

      is_button_2_pressed_first = true;

      /* Отображаем текущий выбранный/сохраненный адрес группы */
      while (is_button_1_pressed == false && is_button_2_pressed == false &&
             is_time_sec_for_settings_elapsed != true) {
        draw_string_on_matrix(SETTINGS_MODE_ID_GROUP);
      }

      /* Переход к следующему режиму меню */
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

#if PROTOCOL_NKU
        if (selected_group_id == GROUP_ID_MAX) {
          selected_group_id = GROUP_ID_MIN;
        } else {
          selected_group_id++;
        }
#elif PROTOCOL_ALPACA

#endif
      }

      drawing_data.floor = selected_group_id;
      setting_symbols(matrix_string, &drawing_data, GROUP_ID_MAX, NULL, 0);

      /* Отображаем текущий выбранный/сохраненный адрес группы */
      while (is_button_1_pressed == false && is_button_2_pressed == false &&
             is_time_sec_for_settings_elapsed != true) {
        draw_string_on_matrix(matrix_string);
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
        draw_string_on_matrix(SETTINGS_MODE_ESC);
      }

      /* Переход к следующему режиму меню */
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

  /* Выход из меню по истечении PERIOD_SEC_FOR_SETTINGS секунд бездействия в
   * меню (БЕЗ сохранения настроек) */
  if (is_time_sec_for_settings_elapsed) {
    is_time_sec_for_settings_elapsed = false;

#if DOT_PIN
    menu_exit(&menu_state, NOT_SAVE_SETTINGS);
#elif DOT_SPI
    menu_exit(&menu_state, SAVE_SETTINGS);
#endif
  }
}

void press_button() {
  // /// Current menu state: MENU_STATE_OPEN, MENU_STATE_WORKING,
  // /// MENU_STATE_CLOSE
  // extern menu_state_t menu_state;

#if !DEMO_MODE && !TEST_MODE

  if (is_first_btn_clicked) {

#if DOT_PIN
    menu.current_state = MENU_STATE_VOLUME;
#elif DOT_SPI

#endif
    reset_volume_flags();

    is_first_btn_clicked = false;

    btn_2_set_value_counter = 0;

    /* Инициализация переменных для Адреса */
    bool is_id_from_flash_valid = matrix_settings.addr_id >= ADDR_ID_MIN &&
                                  matrix_settings.addr_id <= ADDR_ID_LIMIT;

    flash_id = is_id_from_flash_valid ? matrix_settings.addr_id : MAIN_CABIN_ID;

    /* Инициализация переменных для Адреса группы */
    bool is_group_id_from_flash_valid =
        matrix_settings.group_id >= GROUP_ID_MIN &&
        matrix_settings.group_id <= 4;

    flash_group_id =
        is_group_id_from_flash_valid ? matrix_settings.group_id : GROUP_ID_MIN;

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

  /*===== Нажатие кнопки 1 =====*/
  if (is_button_1_pressed) {
    is_button_1_pressed = false;

    handle_button_press(&menu, BUTTON_1_PRESSED);
#if 0
#if DOT_PIN

#elif DOT_SPI

    extern bool is_time_sec_for_settings_elapsed;
    // button_press_time = 0;

    /** Настройка звука:
     * длинное нажатие - вход в режим,
     * короткое - переключение между Б (нет звука) и З (есть звук)
     */
    if (is_long_press_detected) {
      selected_level_volume = level_volume;
      while (is_time_sec_for_settings_elapsed != true &&
             is_button_1_pressed == false) {

        switch (level_volume) {
        case 0:
          display_symbols_spi("dcc"); // VOLUME_0 - нет звука
          break;
        case 1:
          display_symbols_spi("Zcc"); // VOLUME_1 - есть звук
          break;
        }
      }

      /* Выход из меню */
      if (is_time_sec_for_settings_elapsed) {
        is_time_sec_for_settings_elapsed = false;

        update_matrix_settings(&selected_level_volume, &selected_id);
        menu_exit(&menu_state, SAVE_SETTINGS);
      }

      level_volume++;
      if (level_volume > 1) {
        level_volume = 0;
      }

    }
    /** Настройка адреса: короткое нажатие - вход в режим и переключение адресов
     */
    else if (is_short_press_detected) {

      if (id == MAIN_CABIN_ID) {
        matrix_string[DIRECTION] = 'c';
        matrix_string[MSB] = 'K';
        matrix_string[LSB] = 'c';
      } else {
        drawing_data.floor = id;
        setting_symbols(matrix_string, &drawing_data, ADDR_ID_LIMIT, NULL, 0);
      }

      selected_id = id;

      while (is_time_sec_for_settings_elapsed != true &&
             is_button_1_pressed == false) {
        display_symbols_spi(matrix_string);
      }

      /* Выход из меню */
      if (is_time_sec_for_settings_elapsed) {
        is_time_sec_for_settings_elapsed = false;

        update_matrix_settings(&selected_level_volume, &selected_id);
        menu_exit(&menu_state, SAVE_SETTINGS);
      }

#if PROTOCOL_UIM_6100
      if (id == ADDR_ID_LIMIT) {
        id = ADDR_ID_MIN;
      } else if (id == MAX_POSITIVE_NUMBER_LOCATION) {
        id = MAIN_CABIN_ID;
      } else {
        id++;
      }
#else
      id++;
#endif
    }
#endif
#endif
  }

  /*===== Нажатие кнопки 2 =====*/
  if (is_button_2_pressed) {
    is_button_2_pressed = false;
    handle_button_press(&menu, BUTTON_2_PRESSED);
  }

#endif
}
