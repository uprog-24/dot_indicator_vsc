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

typedef enum {
  BUTTON_NONE = 0,
  BUTTON_1_PRESSED,
  BUTTON_2_PRESSED
} button_state_t;

// Подсостояния меню (пункты меню)
typedef enum {
  MENU_SUBSTATE_INIT = 0, // Первый вход
  MENU_SUBSTATE_VOLUME,   // Уровень громкости
  MENU_SUBSTATE_ID,       // Настройка ID
  MENU_SUBSTATE_GROUP_ID, // Доп. настройка (настройка группы)
  MENU_SUBSTATE_EXIT // Выход
} menu_substate_e;

/**
 * @brief  Действия при выходе из меню.
 */
typedef enum {
  NOT_SAVE_SETTINGS, ///< Не сохранять настройки
  SAVE_SETTINGS      ///< Сохранить настройки
} menu_exit_actions_e;

typedef struct {
  uint8_t flash_level_volume; /// Уровень громкости из flash-памяти
  uint8_t flash_id; /// Адрес индикатора  из flash-памяти
  uint8_t flash_group_id; /// Адрес группы из flash (доп.)

  uint8_t selected_level_volume; /// Выбранный в меню vol (уровень громкости)
  uint8_t selected_id; /// Выбранный в меню id (адрес) для индикатора
  uint8_t selected_group_id; /// Выбраннвй доп. параметр

} menu_settings_t;

typedef struct {
  menu_substate_e curr_substate; // Текущее состояние меню
  button_state_t current_button;
  button_state_t previous_button;
  bool value_edited; // Было ли изменено значение в этом пункте

  menu_settings_t settings;
} menu_context_t;

static menu_context_t menu = {.curr_substate = MENU_SUBSTATE_INIT,
                              .current_button = BUTTON_NONE,
                              .previous_button = BUTTON_NONE,
                              .value_edited = false,
                              .settings = {0}};

static void go_to_menu(matrix_state_t *matrix_state);

static void menu_init(menu_settings_t *settings);

static void handle_menu_init(menu_context_t *menu, button_state_t button);
static void handle_menu_volume(menu_context_t *menu, button_state_t button);
static void handle_menu_id(menu_context_t *menu, button_state_t button);
static void handle_menu_escape(menu_context_t *menu, button_state_t button);

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

extern volatile bool is_time_sec_for_settings_elapsed;
extern matrix_state_t matrix_state;

/**
 * @brief  Обработка прерываний для кнопок.
 *         BUTTON_1_Pin и BUTTON_2_Pin используются для меню.
 * @param  GPIO_Pin: Пины кнопок на EXTI линии.
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

  /// Счетчик прошедшего в мс времени между последними нажатиями кнопок.
  extern uint32_t time_since_last_press_ms;

  /* Нажатие кнопки 1 */
  if (GPIO_Pin == BUTTON_1_Pin) {
    go_to_menu(&matrix_state);
  }

  /* Нажатие кнопки 2 */
  if (GPIO_Pin == BUTTON_2_Pin) {
    /* Детектируем нажатие кнопки 2, если уже вошли в режиме Меню (была нажата
     * кнопка 1) */
    if (matrix_state == MATRIX_STATE_MENU) {
      is_button_2_pressed = true;
    }
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
 * @brief Открытие меню настроек
 *
 * @param matrix_state Указатель на состояние индикатора
 */
static void go_to_menu(matrix_state_t *matrix_state) {
  *matrix_state = MATRIX_STATE_MENU; // Переход индикатора в режим меню
  is_button_1_pressed =
      true; // Нажатие кнопки (обработка в функции handle_menu())
  is_interface_connected = false; // Сброс флага подключения интерфейса
}

/**
 * @brief Обновление настроек матрицы (адрес и громкость).
 *
 * @param selected_level_volume Выбранная громкость звуковых оповещений.
 * @param selected_id Выбранный адрес индикатора.
 */
static void update_matrix_settings(settings_t *matrix_settings,
                                   menu_settings_t *menu_settings) {

  volume_t volume_code;

  switch (menu_settings->selected_level_volume) {
  case 0:
    volume_code = VOLUME_0;
    break;
  case 1:
    volume_code = VOLUME_1;
    break;
  case 2:
    volume_code = VOLUME_2;
    break;
  case 3:
    volume_code = VOLUME_3;
    break;
  default:
    break;
  }

  update_structure(matrix_settings, volume_code, menu_settings->selected_id,
                   menu_settings->selected_group_id);
}

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

/**
 * @brief Настройка значения адреса
 *
 * @param selected_id: Выбранное значение адреса
 */
static void set_new_selected_id(uint8_t *selected_id) {
  if (*selected_id == ADDR_ID_LIMIT) {
    *selected_id = ADDR_ID_MIN;
  } else {
    (*selected_id)++;
  }
}

/**
 * @brief Настройка символов адреса
 *
 * @param selected_id:  Выбранное значение адреса
 */
static void set_symbols_id(uint8_t selected_id) {

  if (selected_id == MAIN_CABIN_ID) {
    set_symbols(SYMBOL_EMPTY, SYMBOL_K, SYMBOL_EMPTY);
  } else {
    set_symbols(SYMBOL_EMPTY,
                (selected_id < 10) ? (selected_id) % 10 : (selected_id) / 10,
                (selected_id < 10) ? SYMBOL_EMPTY : (selected_id) % 10);
  }
}

static void reset_menu(menu_context_t *menu_context) {
  menu_context->curr_substate = MENU_SUBSTATE_INIT;
  menu_context->current_button = BUTTON_NONE;
  menu_context->previous_button = BUTTON_NONE;
  menu_context->value_edited = false;
}

extern menu_state_t menu_state;
extern uint32_t time_since_last_press_ms;

/**
 * @brief Выход из меню с сохранением/без сохранения настроек в памяти
 * устройства
 *
 * @param menu_state
 * @param menu_exit_action
 */
void menu_exit(menu_state_t *menu_state, menu_exit_actions_e menu_exit_action) {

  reset_menu(&menu);

  switch (menu_exit_action) {
  case NOT_SAVE_SETTINGS:

    time_since_last_press_ms = 0;

    matrix_state = MATRIX_STATE_START;
    *menu_state = MENU_STATE_OPEN;

    break;

  case SAVE_SETTINGS:
    /* Обновление структуры с данными о настройках: громкость, адрес, адрес
     * группы
     */
    update_matrix_settings(&matrix_settings, &menu.settings);

    /* Сброс флагов, изменение состояний */

    time_since_last_press_ms = 0;
    *menu_state = MENU_STATE_CLOSE;

    break;
  }
}

/**
 * @brief Обработка нажатий кнопок в режиме VOLUME
 *
 * @param menu
 * @param button
 */
static void handle_menu_volume(menu_context_t *menu, button_state_t button) {
  switch (button) {
  case BUTTON_1_PRESSED:
    stop_buzzer_sound();

    set_string(SETTINGS_MODE_VOLUME);
    // Подтверждаем выбор, остаёмся на VOL
    /* Возврат к пункту меню после выбора значения */
    if (menu->value_edited) {
      menu->value_edited = false;
      menu->settings.flash_level_volume =
          menu->settings.selected_level_volume; // Сохраняем выбранное значение
    } else {
      // Переход к следующему режиму меню (к адресу)
      menu->curr_substate = MENU_SUBSTATE_ID;
      set_string(SETTINGS_MODE_ID);
    }
    break;

  case BUTTON_2_PRESSED:
    menu->value_edited = true;

    /* Если кнопка нажата 1-ый раз, то отображаем уровень из flash-памяти;
     * если кнопка нажата НЕ 1-ый раз, то изменяем счетчик текущего выбранного
     * уровня. */
    if (menu->previous_button == BUTTON_1_PRESSED) {
      menu->settings.selected_level_volume =
          menu->settings.flash_level_volume; // Показываем сохранённый уровень
    } else if (menu->previous_button == BUTTON_2_PRESSED) {
      menu->settings.selected_level_volume++; // Увеличиваем уровень
      if (menu->settings.selected_level_volume > VOLUME_LEVEL_LIMIT) {
        menu->settings.selected_level_volume =
            0; // Сброс до 0 при превышении лимита
      }
    }

    /* Отображаем текущий выбранный/сохраненный уровень */
    switch (menu->settings.selected_level_volume) {
    case 0:
      set_string(LEVEL_VOLUME_0);

      play_bip_for_menu(&is_level_volume_0_displayed, VOLUME_0);
      is_level_volume_3_displayed = false;
      break;

    case 1:
      set_string(LEVEL_VOLUME_1);

      play_bip_for_menu(&is_level_volume_1_displayed, VOLUME_1);
      is_level_volume_0_displayed = false;
      break;

    case 2:
      set_string(LEVEL_VOLUME_2);

      play_bip_for_menu(&is_level_volume_2_displayed, VOLUME_2);
      is_level_volume_1_displayed = false;
      break;

    case 3:
      set_string(LEVEL_VOLUME_3);

      play_bip_for_menu(&is_level_volume_3_displayed, VOLUME_3);
      is_level_volume_2_displayed = false;
      break;

    default:
      break;
    }

    reset_volume_flags();
    break;

  default:
    break;
  }
}

/**
 * @brief Обработка нажатий кнопок в режиме ID
 *
 * @param menu
 * @param button
 */
static void handle_menu_id(menu_context_t *menu, button_state_t button) {
  switch (button) {
  case BUTTON_1_PRESSED:
    /* Возврат к пункту меню после выбора значения */
    // Подтверждаем выбор, остаёмся на ID
    if (menu->value_edited) {
      menu->value_edited = false;
      menu->settings.flash_id =
          menu->settings.selected_id; // Сохраняем выбранное значение
      set_string(SETTINGS_MODE_ID); // Можно показать "Сохранено"
    } else {
      // Переход к следующему режиму меню
      menu->curr_substate = MENU_SUBSTATE_EXIT;
      set_string(SETTINGS_MODE_ESC);
    }
    break;

  case BUTTON_2_PRESSED:
    // Изменение ID
    menu->value_edited = true;

    /* Если кнопка нажата 1-ый раз, то отображаем адрес из flash-памяти;
     * если кнопка нажата НЕ 1-ый раз, то изменяем счетчик текущего выбранного
     * адреса. */
    if (menu->previous_button == BUTTON_1_PRESSED) {
      menu->settings.selected_id =
          menu->settings.flash_id; // Показываем сохранённый адрес
    } else if (menu->previous_button == BUTTON_2_PRESSED) {
      /* Сохраняем новое значение адреса в переменную selected_id */
      set_new_selected_id(&menu->settings.selected_id);
    }

    /* Настройка символов для отображения ID */
    set_symbols_id(menu->settings.selected_id);
    break;

  default:
    break;
  }
}

/**
 * @brief Обработка нажатий кнопок в режиме ESCAPE
 *
 * @param menu
 * @param button
 */
static void handle_menu_escape(menu_context_t *menu, button_state_t button) {
  switch (button) {
  case BUTTON_1_PRESSED:
    /* Переход к следующему режиму меню (к уровню громкости) */
    menu->curr_substate = MENU_SUBSTATE_VOLUME;
    set_string(SETTINGS_MODE_VOLUME);
    break;

  case BUTTON_2_PRESSED:
    // Выход из меню с сохранением настроек
    menu->curr_substate = MENU_SUBSTATE_INIT;
    menu_exit(&menu_state, SAVE_SETTINGS);
    break;

  default:
    break;
  }
}

/**
 * @brief Обработка нажатий кнопок в режиме INIT (первый вход)
 *
 * @param menu
 * @param button
 */
static void handle_menu_init(menu_context_t *menu, button_state_t button) {

  // Вход по нажатию кнопки 1
  if (button == BUTTON_1_PRESSED) {
    menu_init(&menu->settings);
    menu->curr_substate = MENU_SUBSTATE_VOLUME;
    set_string(SETTINGS_MODE_VOLUME);
  }
}

/**
 * @brief Обработка кнопок в зависимости от состояния меню
 *
 * @param menu
 * @param button
 */
static void handle_button_press(menu_context_t *menu,
                                button_state_t curr_button) {

  menu->previous_button = menu->current_button;
  menu->current_button = curr_button;

  switch (menu->curr_substate) {
    /* ===== Состояние инициализации (Вход) ===== */
  case MENU_SUBSTATE_INIT:
    handle_menu_init(menu, menu->current_button);
    break;

    /*===== Режим меню VOL (Уровень громкости) =====*/
  case MENU_SUBSTATE_VOLUME:
    handle_menu_volume(menu, menu->current_button);
    break;

    /*===== Режим меню ID (Адрес) =====*/
  case MENU_SUBSTATE_ID:
    handle_menu_id(menu, menu->current_button);
    break;

  /*===== Режим меню ESC (Выход) =====*/
  case MENU_SUBSTATE_EXIT:
    handle_menu_escape(menu, menu->current_button);
    break;
  }
}

/**
 * @brief Вход в меню
 *
 */
static void menu_init(menu_settings_t *settings) {

  reset_volume_flags();

  /* Инициализация переменных для Адреса */
  bool is_id_from_flash_valid = matrix_settings.addr_id >= ADDR_ID_MIN &&
                                matrix_settings.addr_id <= ADDR_ID_LIMIT;

  settings->flash_id =
      is_id_from_flash_valid ? matrix_settings.addr_id : MAIN_CABIN_ID;

  /* Инициализация переменных для Уровня громкости */

  bool is_volume_from_flash_valid = (matrix_settings.volume == VOLUME_0 ||
                                     matrix_settings.volume == VOLUME_1 ||
                                     matrix_settings.volume == VOLUME_2 ||
                                     matrix_settings.volume == VOLUME_3);

  if (!is_volume_from_flash_valid) {
    settings->flash_level_volume = 1;
  } else {
    switch (matrix_settings.volume) {
    case VOLUME_0:
      settings->flash_level_volume = 0;
      break;
    case VOLUME_1:
      settings->flash_level_volume = 1;
      break;
    case VOLUME_2:
      settings->flash_level_volume = 2;
      break;
    case VOLUME_3:
      settings->flash_level_volume = 3;
      break;
    }
  }

  settings->selected_level_volume = settings->flash_level_volume;
  settings->selected_id = settings->flash_id;
  settings->selected_group_id = settings->flash_group_id;
}

/**
 * @brief Обработка меню
 *
 */
void handle_menu() {

#if !DEMO_MODE && !TEST_MODE

  /* Выход из меню по истечении PERIOD_SEC_FOR_SETTINGS секунд бездействия в
   * меню (БЕЗ сохранения настроек) */
  if (is_time_sec_for_settings_elapsed) {
    is_time_sec_for_settings_elapsed = false;

    menu_exit(&menu_state, NOT_SAVE_SETTINGS);
  }

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
