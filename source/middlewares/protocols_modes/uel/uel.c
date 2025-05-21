/**
 * @file uel.c
 */
#include "uel.h"

#include "buzzer.h"
#include "config.h"
#include "drawing.h"
#include "tim.h"

#include "string.h"

#define NINE_BITS_MASK 0x1FF    ///< Mask for 9 bits with data by UEL protocol
#define CODE_LOCATION_MASK 0x3F ///< Mask for code location bits
#define CONTROL_BITS_MASK 0x1C0 ///< Mask for control bits
#define SPECIAL_SYMBOLS_BUFF_SIZE 13 ///< Number of special symbols
#define GONG_BUZZER_FREQ 3000        ///< Frequency of bip for ARRIVAL gong
#define BUZZER_FREQ_CABIN_OVERLOAD                                             \
  3000 ///< Frequency of bip for VOICE_CABIN_OVERLOAD
#define BUZZER_FREQ_FIRE_DANGER                                                \
  BUZZER_FREQ_CABIN_OVERLOAD ///< Frequency of bip for FIRE_DANGER

/**
 * Stores values of direction of the movement (UEL)
 */
typedef enum {
  UEL_MOVE_UP = 0x140,
  UEL_MOVE_DOWN = 0x180,
  UEL_NO_MOVE = 0x100
} direction_uel_t;

/**
 * Stores values of control bits
 */
typedef enum CONTROL_BITS_STATES {
  NUMBER_CLICKED_BTN = 0x1C0,
  MOVE_UP_AFTER_STOP = 0x40,
  MOVE_DOWN_AFTER_STOP = 0x80,
  MOVE_UP_OR_DOWN_AFTER_STOP = 0xC0,
  SPECIAL_FORMAT = 0x0 //
} control_bits_states_t;

/**
 * Stores values of code locations
 */
typedef enum CODE_LOCATION_UEL {
  LOCATION_DASH = 41,
  LOCATION_P2 = 44,
  LOCATION_P1 = 45,
  LOCATION_P = 46,
  LOCATION_MINUS_4 = 47,
  LOCATION_MINUS_3 = 48,
  LOCATION_MINUS_2 = 49,
  LOCATION_MINUS_1 = 50,
  LOCATION_C1 = 52,
  LOCATION_C2 = 53,
  LOCATION_REVISION = 55,
  LOCATION_NORMAL_WORK = 56,
  LOCATION_LOADING = 57,

  // repeated values
  CABIN_OVERLOAD = 1,
  GONG_ARRIVAL = 8,
  FARE_DANGER_SYMBOL = 2,
  FARE_DANGER_SOUND = 32,
  EVACUATION = 4,
  SEISMIC_DANGER = 16
} code_location_uel_t;

/// Buffer with code location and it's symbols
static const code_location_symbols_t
    special_symbols_code_location[SPECIAL_SYMBOLS_BUFF_SIZE] = {
        {.code_location = LOCATION_DASH, .symbols = "--"},
        {.code_location = LOCATION_P2, .symbols = "p2"},
        {.code_location = LOCATION_P1, .symbols = "p1"},
        {.code_location = LOCATION_P, .symbols = "p"},
        {.code_location = LOCATION_C1, .symbols = "C1"},
        {.code_location = LOCATION_C2, .symbols = "C2"},
        {.code_location = LOCATION_REVISION, .symbols = "PE"},
        {.code_location = LOCATION_NORMAL_WORK, .symbols = "HP"},
        {.code_location = LOCATION_LOADING, .symbols = "pg"},

        {.code_location = LOCATION_MINUS_4, .symbols = "-4"},
        {.code_location = LOCATION_MINUS_3, .symbols = "-3"},
        {.code_location = LOCATION_MINUS_2, .symbols = "-2"},
        {.code_location = LOCATION_MINUS_1, .symbols = "-1"},
};

/// Structure for data that will be displayed on matrix
static drawing_data_t drawing_data = {0, 0};

/// Flag to control if gong is playing
static volatile bool is_gong_play = false;

/// Flag to control if cabin is overloaded
static bool is_cabin_overload_sound = false;

/// Flag to control fire danger
static bool is_fire_danger_sound = false;

/// Counter for number received data (fire danger)
static uint8_t fire_danger_cnt = 0;

/// Counter for number received data (fire danger is disable sound)
static uint8_t fire_disable_cnt = 0;

/// Flag to control evacuation sound
static bool is_evacuation_sound = false;

/// Flag to control seismic danger sound
static bool is_seismic_danger_sound = false;

/// Received 2 bytes by UART
static uint16_t data = 0;

/**
 * @brief  Setting symbols for UEL protocol
 * @param  str:              Pointer to the output string with symbols
 * @param  current_location: Code of current location
 * @param  control_bits:     Control bits
 * @retval None
 */
static void setting_sound_uel(char *matrix_string, uint8_t current_location,
                              control_bits_states_t control_bits) {
  switch (control_bits) {
  case SPECIAL_FORMAT:

    if ((current_location & EVACUATION) == EVACUATION) {
      matrix_string[MSB] = 'E';
      matrix_string[LSB] = 'c';
      stop_buzzer_sound();
    }

    if ((current_location & SEISMIC_DANGER) == SEISMIC_DANGER) {
      matrix_string[MSB] = 'L';
      matrix_string[LSB] = 'c';
      stop_buzzer_sound();
    }

#if 1
    if ((current_location & FARE_DANGER_SYMBOL) == FARE_DANGER_SYMBOL) {
      matrix_string[MSB] = 'F';
      matrix_string[LSB] = 'c';
      stop_buzzer_sound();
    }
#endif

#if 0

    if ((current_location & GONG_ARRIVAL) == GONG_ARRIVAL) {
      if (!is_gong_play) {
        stop_buzzer_sound();
#if 1
        if (matrix_settings.volume != VOLUME_0) {
          play_gong(3, GONG_BUZZER_FREQ, matrix_settings.volume);
        }

#endif
        is_gong_play = true;
      }
    } else {
      is_gong_play = false;
    }

#endif

#if 0
    if ((current_location & CABIN_OVERLOAD) == CABIN_OVERLOAD) {
#if 1
      if (matrix_settings.volume != VOLUME_0 &&
          matrix_settings.addr_id == MAIN_CABIN_ID) {
        start_buzzer_sound(BUZZER_FREQ_CABIN_OVERLOAD, VOLUME_3);
      }
#endif
      is_cabin_overload_sound = true;

      matrix_string[DIRECTION] = 'c';
      matrix_string[MSB] = 'K';
      matrix_string[LSB] = 'g';

    } else if (is_cabin_overload_sound) {
      stop_buzzer_sound();
      is_cabin_overload_sound = false;
    }

#endif

    if ((current_location & FARE_DANGER_SOUND) == FARE_DANGER_SOUND) {
      fire_danger_cnt++;
      if (fire_danger_cnt > 5U) {
        stop_buzzer_sound();
        fire_danger_cnt = 0;
#if 1
        if (matrix_settings.volume != VOLUME_0 &&
            matrix_settings.addr_id == MAIN_CABIN_ID) {
          start_buzzer_sound(BUZZER_FREQ_FIRE_DANGER, VOLUME_3);
        }
//          play_gong(1, BUZZER_FREQ_FIRE_DANGER, VOLUME_3);
#endif
        is_fire_danger_sound = true;
      }

      if (is_fire_danger_sound) {
        fire_disable_cnt++;
        if (fire_disable_cnt == 3) {
          fire_disable_cnt = 0;
          is_fire_danger_sound = false;
          stop_buzzer_sound();
        }
      }

    } else {
      is_fire_danger_sound = false;
      fire_disable_cnt = 0;
      fire_danger_cnt = 0;
    }
    break;

  default:
    //      stop_buzzer_sound();
    break;
  }
}

static uint8_t gong[2] = {
    0,
};

// Гонг
static void setting_gong(control_bits_states_t control_bits,
                         uint8_t current_location, uint8_t volume) {
  uint8_t arrival = current_location & GONG_ARRIVAL;

  /*  Если сигнал Гонг из 0 меняется на 1 и быты Движение Вверх и Вниз равны 0,
   * тогда детектируем прибытие на этаж */
  gong[0] = ((arrival == GONG_ARRIVAL) == 1) ? 1 : 0;

  if (gong[0] && !gong[1]) {

    switch (control_bits) {
    case UEL_MOVE_UP:
      play_gong(1, GONG_BUZZER_FREQ, volume);
      break;
    case UEL_MOVE_DOWN:
      play_gong(1, GONG_BUZZER_FREQ, volume);
      break;
    case UEL_NO_MOVE:
      play_gong(3, GONG_BUZZER_FREQ, volume);
      break;
    default:
      //  play_gong(1, GONG_BUZZER_FREQ, volume);
      break;
    }
  }
  gong[1] = gong[0];
}

/// Флаг для контроля перегруза кабины
bool is_cabin_overload = false;

bool is_overload_sound_on = false; // состояние звука

extern uint16_t overload_sound_ms;
extern volatile bool is_time_ms_for_overload_elapsed;

// Режим Перегрузка (символы и звук)
static void set_cabin_overload_symbol_sound(uint8_t current_location) {

  if ((current_location & CABIN_OVERLOAD) == CABIN_OVERLOAD) {

    if (matrix_settings.volume != VOLUME_0) {

      // Первый вход в режим перегрузки
      if (!is_cabin_overload) {
        is_cabin_overload = true;
        overload_sound_ms = 0;
        is_overload_sound_on = true;
        start_buzzer_sound(BUZZER_FREQ_CABIN_OVERLOAD, VOLUME_3);
      }

      if (is_time_ms_for_overload_elapsed) {
        is_time_ms_for_overload_elapsed = false;

        if (is_overload_sound_on) {
          stop_buzzer_sound();
          is_overload_sound_on = false;
        } else {
          start_buzzer_sound(BUZZER_FREQ_CABIN_OVERLOAD, VOLUME_3);
          is_overload_sound_on = true;
        }
      }
    }

    matrix_string[DIRECTION] = 'c';
    matrix_string[MSB] = 'K';
    matrix_string[LSB] = 'g';
  } else if (is_cabin_overload) {
    stop_buzzer_sound();
    is_cabin_overload = false;
    is_overload_sound_on = false;
    overload_sound_ms = 0;
  }
}

static void cabin_indicator_special_regime(char *matrix_string,
                                           uint8_t current_location,
                                           control_bits_states_t control_bits) {

  // Гонг
  if (matrix_settings.volume != VOLUME_0) {
    setting_gong(control_bits, current_location, matrix_settings.volume);
  }

  // Погрузка
  // set_loading_symbol(control_byte_first);

  // Перегруз
  // set_cabin_overload_symbol_sound(current_location);

  // Авария
  // set_accident_symbol(control_byte_second);

  // Пожар, символ F
  // set_fire_danger_symbol(control_byte_first);

  // Пожар, звук по фронту
  if (matrix_settings.volume != VOLUME_0) {
    // set_fire_danger_sound(control_byte_second);
  }
}

static void floor_indicator_special_regime(uint8_t direction_code,
                                           uint8_t control_byte_first,
                                           uint8_t control_byte_second) {

  // Гонг, прибытие на этаж с номером matrix_settings.addr_id
  if (matrix_settings.addr_id == drawing_data.floor) {
    if (matrix_settings.volume != VOLUME_0) {
      setting_gong(direction_code, control_byte_first, matrix_settings.volume);
    }
  }

  // Авария
  // set_accident_symbol(control_byte_second);

  // Пожар, символ F
  // set_fire_danger_symbol(control_byte_first);
}

/**
 * @brief  Transform UEL values of direction to common directionType that
 *         defined in drawing.h
 * @param  direction: Value from enum direction_uel_t:
 *                    UEL_MOVE_UP/UEL_MOVE_DOWN/UEL_NO_MOVE
 * @retval None
 */
static void transform_direction_to_common(direction_uel_t direction) {
  switch (direction) {
  case UEL_MOVE_UP:
    drawing_data.direction = DIRECTION_UP;
    break;
  case UEL_MOVE_DOWN:
    drawing_data.direction = DIRECTION_DOWN;
    break;
  case UEL_NO_MOVE:
    drawing_data.direction = NO_DIRECTION;
    break;

  default:
    drawing_data.direction = NO_DIRECTION;
    break;
  }
}

/// Received control bits
static control_bits_states_t control_bits;

/**
 * @brief  Process data using UEL protocol
 *         1. Get 9 bits from 16 received bits;
 *         2. Set drawing_data structure, setting symbols and sound;
 *         3. Display matrix_string while next data is not received and
 *            interface is connected.
 * @param  received_data: Pointer to the buffer with received data by CAN
 * @retval None
 */

static uint8_t dash_error_cnt = 0;
static uint8_t dash_error_disable = 0;
static const uint8_t TIME_FLOOR_DURING_DASH = 8;
static const uint8_t TIME_DASH_DURING_DASH = 10;
static bool is_dash_error_displayed = false;

void process_data_uel(uint16_t *received_data) {
  /// Flag to control is receiving data completed by UART
  extern bool is_rx_data_completed;

  data = ~(*received_data) & NINE_BITS_MASK;
  control_bits = data & CONTROL_BITS_MASK;

  transform_direction_to_common(control_bits);
  drawing_data.floor = data & CODE_LOCATION_MASK;

  if (control_bits !=
      SPECIAL_FORMAT) { // due to repeated values for SPECIAL_FORMAT and FLOORS

    if (drawing_data.floor == LOCATION_DASH) {
      dash_error_cnt++;
      if (dash_error_cnt == TIME_FLOOR_DURING_DASH) {
        dash_error_cnt = 0;
        is_dash_error_displayed = true;

        setting_symbols(
            matrix_string, &drawing_data, MAX_POSITIVE_NUMBER_LOCATION,
            special_symbols_code_location, SPECIAL_SYMBOLS_BUFF_SIZE);
      }
    } else {
      if (is_dash_error_displayed) {
        dash_error_disable++;
        if (dash_error_disable == TIME_DASH_DURING_DASH) { // 690 ms, STOP
          dash_error_disable = 0;
          is_dash_error_displayed = false;
          setting_symbols(
              matrix_string, &drawing_data, MAX_POSITIVE_NUMBER_LOCATION,
              special_symbols_code_location, SPECIAL_SYMBOLS_BUFF_SIZE);
        }
      } else {
        setting_symbols(
            matrix_string, &drawing_data, MAX_POSITIVE_NUMBER_LOCATION,
            special_symbols_code_location, SPECIAL_SYMBOLS_BUFF_SIZE);
      }
    }
  }

  // Кабинный индикатор
  if (matrix_settings.addr_id == MAIN_CABIN_ID) {
    // Спец. режимы для кабинного индикатора
    cabin_indicator_special_regime(matrix_string, drawing_data.floor,
                                   control_bits);
  } else {
    // Этажный индикатор
    /* Установка drawing_data.floor для гонга */
    // Этажи 0, с 1 по 9
    // if (first_symbol_code == 0 || first_symbol_code == SYMBOL_EMPTY) {
    //   drawing_data.floor = second_symbol_code;
    // }

    // // Этажи с 10 по 99
    // if (first_symbol_code >= 1 && first_symbol_code <= 9 &&
    //     second_symbol_code >= 0 && second_symbol_code <= 9) {
    //   drawing_data.floor = first_symbol_code * 10 + second_symbol_code;
    // }

    // Спец. режимы для этажного индикатора
    floor_indicator_special_regime(matrix_string, drawing_data.floor,
                                   control_bits);
  }

  setting_sound_uel(matrix_string, drawing_data.floor, control_bits);

  while (is_rx_data_completed == false && is_interface_connected == true) {
#if DOT_PIN
    draw_string_on_matrix(matrix_string);
#elif DOT_SPI
    display_symbols_spi(matrix_string);
#endif
  }
}
