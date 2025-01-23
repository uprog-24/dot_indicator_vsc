/**
 * @file alpaca.c
 */
#include "alpaca.h"

#include "buzzer.h"
#include "config.h"
#include "drawing.h"
#include "tim.h"

#include <stdbool.h>
//==============================================================================
//                       ИНДИКАЦИЯ МЕСТОПОЛОЖЕНИЯ
//==============================================================================
// ------------------------ Направление движения -------------------------------
#define PR_IM_AR_NA 10000 // Нет направления, стрелка неактивна
#define PR_IM_AR_UP 10001 // Направление вверх, стрелка вверх
#define PR_IM_AR_DN 10002 // Направление вниз, стрелка вниз
// -------------------------- Индикация этажа ----------------------------------
#define PR_IM_FL_NA 10003 // Этаж не определен
#define PR_IM_FL_01 10004 // Индикация этажа №1
#define PR_IM_FL_02 10005 // Индикация этажа №2
#define PR_IM_FL_03 10006 // Индикация этажа №3
#define PR_IM_FL_04 10007 // Индикация этажа №4
#define PR_IM_FL_05 10008 // Индикация этажа №5
#define PR_IM_FL_06 10009 // Индикация этажа №6
#define PR_IM_FL_07 10010 // Индикация этажа №7
#define PR_IM_FL_08 10011 // Индикация этажа №8
#define PR_IM_FL_09 10012 // Индикация этажа №9
#define PR_IM_FL_10 10013 // Индикация этажа №10
#define PR_IM_FL_11 10014 // Индикация этажа №11
#define PR_IM_FL_12 10015 // Индикация этажа №12
#define PR_IM_FL_13 10016 // Индикация этажа №13
#define PR_IM_FL_14 10017 // Индикация этажа №14
#define PR_IM_FL_15 10018 // Индикация этажа №15
#define PR_IM_FL_16 10019 // Индикация этажа №16
#define PR_IM_FL_17 10020 // Индикация этажа №17
#define PR_IM_FL_18 10021 // Индикация этажа №18
#define PR_IM_FL_19 10022 // Индикация этажа №19
#define PR_IM_FL_20 10023 // Индикация этажа №20
#define PR_IM_FL_21 10024 // Индикация этажа №21
#define PR_IM_FL_22 10025 // Индикация этажа №22
#define PR_IM_FL_23 10026 // Индикация этажа №23
#define PR_IM_FL_24 10027 // Индикация этажа №24
#define PR_IM_FL_25 10028 // Индикация этажа №25
#define PR_IM_FL_26 10029 // Индикация этажа №26
#define PR_IM_FL_27 10030 // Индикация этажа №27
#define PR_IM_FL_28 10031 // Индикация этажа №28
#define PR_IM_FL_29 10032 // Индикация этажа №29
#define PR_IM_FL_30 10033 // Индикация этажа №30
#define PR_IM_FL_31 10034 // Индикация этажа №31
#define PR_IM_FL_32 10035 // Индикация этажа №32
#define PR_IM_FL_33 10036 // Индикация этажа №33
#define PR_IM_FL_34 10037 // Индикация этажа №34
#define PR_IM_FL_35 10038 // Индикация этажа №35
#define PR_IM_FL_36 10039 // Индикация этажа №36
#define PR_IM_FL_37 10040 // Индикация этажа №37
#define PR_IM_FL_38 10041 // Индикация этажа №38
#define PR_IM_FL_39 10042 // Индикация этажа №39
#define PR_IM_FL_40 10043 // Индикация этажа №40
#define PR_IM_FL_41 10044 // Индикация этажа №41
#define PR_IM_FL_42 10045 // Индикация этажа №42
#define PR_IM_FL_43 10046 // Индикация этажа №43
#define PR_IM_FL_44 10047 // Индикация этажа №44
#define PR_IM_FL_45 10048 // Индикация этажа №45
#define PR_IM_FL_46 10049 // Индикация этажа №46
#define PR_IM_FL_47 10050 // Индикация этажа №47
#define PR_IM_FL_48 10051 // Индикация этажа №48
#define PR_IM_FL_49 10052 // Индикация этажа №49
#define PR_IM_FL_50 10053 // Индикация этажа №50
#define PR_IM_FL_51 10054 // Индикация этажа №51
#define PR_IM_FL_52 10055 // Индикация этажа №52
#define PR_IM_FL_53 10056 // Индикация этажа №53
#define PR_IM_FL_54 10057 // Индикация этажа №54
#define PR_IM_FL_55 10058 // Индикация этажа №55
#define PR_IM_FL_56 10059 // Индикация этажа №56
#define PR_IM_FL_57 10060 // Индикация этажа №57
#define PR_IM_FL_58 10061 // Индикация этажа №58
#define PR_IM_FL_59 10062 // Индикация этажа №59
#define PR_IM_FL_60 10063 // Индикация этажа №60
#define PR_IM_FL_61 10064 // Индикация этажа №61
#define PR_IM_FL_62 10065 // Индикация этажа №62
#define PR_IM_FL_63 10066 // Индикация этажа №63
#define PR_IM_FL_64 10067 // Индикация этажа №64

//==============================================================================
//                       ГОЛОСОВЫЕ СООБЩЕНИЯ И ИНДИКАЦИЯ
//==============================================================================
#define PR_IM_LD_ON 10735 // MM посылает сигнал в IM о начале режима погрузки
#define PR_IM_LD_OFF                                                           \
  10736 // MM посылает сигнал в IM об окончании режима погрузки
//------------------------------------------------------------------------------
#define PR_IM_OVL_1 10737 // ММ посылает сигнал в IM о наличии перегрузки кабины
#define PR_IM_OVL_0                                                            \
  10738 // ММ посылает сигнал в IM об окончании перегрузки кабины
#define PR_IM_GNG_1 10739 // ММ посылает сигнал в IM сигнал прибытия
#define PR_IM_GNG_0                                                            \
  10740 // ММ посылает сигнал в IM прекращение сигнала прибытия
//------------------------------------------------------------------------------
// ММ посылает сигнал в IM сигнал о начале пожара или сейсмической активности
#define PR_IM_FRA 10741
//------------------------------------------------------------------------------
#define PR_IM_OPD 10742 // ММ посылает сигнал в IM сигнал начале открытия дверей
#define PR_IM_CLD 10743 // ММ посылает сигнал в IM сигнал начале закрытия дверей
//------------------------------------------------------------------------------
#define PR_IM_ABT_PRESS                                                        \
  10744 // ММ посылает сигнал в IM сигнал о нажатии кнопки "Отмена"
//------------------------------------------------------------------------------
#define PR_IM_ERR_PRESS                                                        \
  10745 // ММ посылает сигнал в IM сигнал о неисправности лифта
#define PR_IM_EVQ_PRESS                                                        \
  10746 // ММ посылает сигнал в IM сигнал о начале режима эвакуации
//------------------------------------------------------------------------------

#define SPECIAL_SYMBOLS_BUFF_SIZE 4 ///< Number of special symbols
#define GONG_BUZZER_FREQ 3000       ///< Frequency of bip for ARRIVAL gong
#define BUZZER_FREQ_CABIN_OVERLOAD                                             \
  5000 ///< Frequency of bip for VOICE_CABIN_OVERLOAD
#define BUZZER_FREQ_FIRE_DANGER                                                \
  BUZZER_FREQ_CABIN_OVERLOAD ///< Frequency of bip for FIRE_DANGER

/**
 * Stores indexes of bytes of UIM6100 protocol
 */
typedef enum ALPACA_PACKET_BYTES {
  BYTE_0,
  BYTE_1,
} alpaca_packet_bytes_t;

/**
 * Stores direction of movement (ALPACA)
 */
typedef enum {
  ALPACA_MOVE_UP = PR_IM_AR_UP,
  ALPACA_MOVE_DOWN = PR_IM_AR_DN,
  ALPACA_NO_MOVE = PR_IM_AR_NA
} direction_alpaca_t;

/// Buffer with code location and it's symbols
static const code_location_symbols_t
    special_symbols_code_location[SPECIAL_SYMBOLS_BUFF_SIZE] = {
        {.code_location = PR_IM_LD_ON, .symbols = "pg"},
        {.code_location = PR_IM_ERR_PRESS, .symbols = "A"},
        {.code_location = PR_IM_EVQ_PRESS, .symbols = "E"},
        {.code_location = PR_IM_FL_NA, .symbols = "EFL"}};

/// Structure for data that will be displayed on matrix
static drawing_data_t drawing_data = {0, 0};

/**
 * @brief  Transform UIM6100 values of direction to common directionType that
 *         defined in drawing.h
 * @param  direction: Value from enum direction_uim_6100_t:
 *                    UIM_6100_MOVE_UP/UIM_6100_MOVE_DOWN/UIM_6100_NO_MOVE
 * @retval None
 */
static void transform_direction_to_common(direction_alpaca_t direction) {
  switch (direction) {
  case ALPACA_MOVE_UP:
    drawing_data.direction = DIRECTION_UP;
    break;
  case ALPACA_MOVE_DOWN:
    drawing_data.direction = DIRECTION_DOWN;
    break;
  case ALPACA_NO_MOVE:
    drawing_data.direction = NO_DIRECTION;
    break;

  default:
    drawing_data.direction = NO_DIRECTION;
    break;
  }
}

static uint16_t data;

// static uint16_t current_floor = 0;
static direction_alpaca_t direction = ALPACA_NO_MOVE;

static uint8_t dash_error_cnt = 0;
static uint8_t dash_error_disable = 0;
static const uint8_t TIME_FLOOR_DURING_DASH = 8;
static const uint8_t TIME_DASH_DURING_DASH = 10;
static bool is_dash_error_displayed = false;
static uint8_t shifted_floor = 0;
static bool is_drawing_data_floor_special = false;

static void process_code_location() {
  if (data == PR_IM_FL_NA) {
    drawing_data.floor = PR_IM_FL_NA;
    is_drawing_data_floor_special = true;
  }

  if (data >= PR_IM_FL_01 && data <= PR_IM_FL_64) {
    drawing_data.floor = data % 100 - 3;
    is_drawing_data_floor_special = false;
  }

  switch (data) {
  case PR_IM_AR_NA:
    direction = ALPACA_NO_MOVE;
    break;

  case PR_IM_AR_UP:
    direction = ALPACA_MOVE_UP;
    break;

  case PR_IM_AR_DN:
    direction = ALPACA_MOVE_DOWN;
    break;

    // special symbols
  case PR_IM_LD_ON:
    drawing_data.floor = PR_IM_LD_ON;
    is_drawing_data_floor_special = true;
    break;

  case PR_IM_ERR_PRESS:
    drawing_data.floor = PR_IM_ERR_PRESS;
    is_drawing_data_floor_special = true;
    break;

  case PR_IM_EVQ_PRESS:
    drawing_data.floor = PR_IM_EVQ_PRESS;
    is_drawing_data_floor_special = true;
    break;

  default:
    // current_floor = 0;
    break;
  }

  if (is_drawing_data_floor_special) {
    set_floor_symbols(matrix_string, drawing_data.floor,
                      MAX_POSITIVE_NUMBER_LOCATION,
                      special_symbols_code_location, SPECIAL_SYMBOLS_BUFF_SIZE);
  } else {
    if (matrix_settings.addr_id <= 10
        // MAX_P_FLOOR_ID
    ) {

      if (drawing_data.floor > matrix_settings.addr_id) {
        set_floor_symbols(
            matrix_string, drawing_data.floor - matrix_settings.addr_id,
            MAX_POSITIVE_NUMBER_LOCATION, special_symbols_code_location,
            SPECIAL_SYMBOLS_BUFF_SIZE);

      } else {

        shifted_floor = matrix_settings.addr_id - drawing_data.floor + 1;
        if (shifted_floor <= 9) {
          matrix_string[DIRECTION] = 'c';
          matrix_string[MSB] = 'p';
          matrix_string[LSB] = convert_int_to_char(shifted_floor);
        } else {
          matrix_string[DIRECTION] = 'p';
          matrix_string[MSB] = convert_int_to_char(shifted_floor / 10);
          matrix_string[LSB] = convert_int_to_char(shifted_floor % 10);
        }
      }

    } else if (matrix_settings.addr_id >= 11
               // MIN_MINUS_FLOOR_ID
               && matrix_settings.addr_id <= ADDR_ID_LIMIT) {

      if (drawing_data.floor > matrix_settings.addr_id - 10) {
        set_floor_symbols(
            matrix_string, drawing_data.floor - (matrix_settings.addr_id - 10),
            MAX_POSITIVE_NUMBER_LOCATION, special_symbols_code_location,
            SPECIAL_SYMBOLS_BUFF_SIZE);
      } else {

        shifted_floor = matrix_settings.addr_id - drawing_data.floor - 10 + 1;

        if (shifted_floor <= 9) {
          matrix_string[DIRECTION] = 'c';
          matrix_string[MSB] = '-';
          matrix_string[LSB] = convert_int_to_char(shifted_floor);
        } else {
          matrix_string[DIRECTION] = '-';
          matrix_string[MSB] = convert_int_to_char(shifted_floor / 10);
          matrix_string[LSB] = convert_int_to_char(shifted_floor % 10);
        }
      }
    }
  }

#if 0
  if (matrix_string[DIRECTION] != '-') {
    dash_error_cnt++;
    if (dash_error_cnt == TIME_FLOOR_DURING_DASH) {
      dash_error_cnt = 0;
      is_dash_error_displayed = true;

      // setting_symbols(matrix_string, &drawing_data,
      //                 MAX_POSITIVE_NUMBER_LOCATION,
      //                 special_symbols_code_location,
      //                 SPECIAL_SYMBOLS_BUFF_SIZE);
      transform_direction_to_common(direction);
      set_direction_symbol(matrix_string, drawing_data.direction);
    }
  } else {
    if (is_dash_error_displayed) {
      dash_error_disable++;
      if (dash_error_disable == TIME_DASH_DURING_DASH) { // 690 ms, STOP
        dash_error_disable = 0;
        is_dash_error_displayed = false;
        // setting_symbols(
        //     matrix_string, &drawing_data, MAX_POSITIVE_NUMBER_LOCATION,
        //     special_symbols_code_location, SPECIAL_SYMBOLS_BUFF_SIZE);
      }
    } else {
      // setting_symbols(matrix_string, &drawing_data,
      //                 MAX_POSITIVE_NUMBER_LOCATION,
      //                 special_symbols_code_location,
      //                 SPECIAL_SYMBOLS_BUFF_SIZE);
    }
  }
#endif

  if (matrix_string[DIRECTION] != '-' && matrix_string[DIRECTION] != 'p') {
    transform_direction_to_common(direction);
    set_direction_symbol(matrix_string, drawing_data.direction);
  } else { // periodic change direction and symbol "p"/"-"
  }
}

/// Flag to control if cabin is overloaded
static bool is_cabin_overload_sound = false;

/// Counter for number received data (fire danger)
static uint8_t fire_danger_cnt = 0;

/// Flag to control fire danger
static bool is_fire_danger_sound = false;

/// Counter for number received data (fire danger is disable sound)
static uint8_t fire_disable_cnt = 0;

static void setting_sound_alpaca(uint16_t current_location) {
  if (current_location == PR_IM_OVL_1) {
    TIM2_Start_bip(BUZZER_FREQ_CABIN_OVERLOAD, matrix_settings.volume);
  }

  if (current_location == PR_IM_OVL_0 || current_location == PR_IM_GNG_0) {
    TIM2_Stop_bip();
  }

  if (current_location == PR_IM_GNG_1) {
    // play_gong(3, GONG_BUZZER_FREQ, matrix_settings.volume);
    TIM2_Start_bip(GONG_BUZZER_FREQ, matrix_settings.volume);
  }

  if (current_location == PR_IM_FRA) {
    matrix_string[MSB] = 'F';
    matrix_string[LSB] = 'c';
    fire_danger_cnt++;
    // if (fire_danger_cnt > 10U) { // START
    // stop_buzzer_sound();
    fire_danger_cnt = 0;
#if 1
    TIM2_Start_bip(BUZZER_FREQ_FIRE_DANGER, VOLUME_3);
#endif
    is_fire_danger_sound = true;
    // }

#if 0
    if (is_fire_danger_sound) {
      fire_disable_cnt++;
      if (fire_disable_cnt == 8) { // STOP
        fire_disable_cnt = 0;
        is_fire_danger_sound = false;
        stop_buzzer_sound();
      }
    }
#endif
  } else {
    is_fire_danger_sound = false;
    fire_disable_cnt = 0;
    fire_danger_cnt = 0;
  }

  if (current_location == PR_IM_OPD) {
    play_gong(1, GONG_BUZZER_FREQ, matrix_settings.volume);
  }

  if (current_location == PR_IM_OPD || current_location == PR_IM_CLD ||
      current_location == PR_IM_ABT_PRESS) {
    play_gong(1, GONG_BUZZER_FREQ, matrix_settings.volume);
  }
}

void process_data_alpaca(uint8_t *rx_data_can) {

  /// Flag to control is data received by CAN
  extern volatile bool is_data_received;

  uint8_t first_byte = rx_data_can[BYTE_0];
  uint8_t second_byte = rx_data_can[BYTE_1];

  data = (first_byte << 8) | second_byte;

  process_code_location();

  // if (matrix_settings.volume != VOLUME_0) {
  setting_sound_alpaca(data);
  // }

  //  while (is_data_received == false && is_interface_connected == true) {
  draw_string_on_matrix(matrix_string);
  // }
}