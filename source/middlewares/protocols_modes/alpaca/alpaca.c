/**
 * @file alpaca.c
 */
#include "alpaca.h"

#include "buzzer.h"
#include "config.h"
#include "drawing.h"
#include "tim.h"

#include <stdbool.h>

#define SPECIAL_SYMBOLS_BUFF_SIZE 6 ///< Number of special symbols
#define GONG_BUZZER_FREQ 3000       ///< Frequency of bip for ARRIVAL gong
#define BUZZER_FREQ_CABIN_OVERLOAD                                             \
  3000 ///< Frequency of bip for VOICE_CABIN_OVERLOAD
#define BUZZER_FREQ_FIRE_DANGER                                                \
  BUZZER_FREQ_CABIN_OVERLOAD ///< Frequency of bip for FIRE_DANGER

/**
 * Stores direction of movement (ALPACA)
 */
typedef enum {
  ALPACA_MOVE_UP = PR_IM_AR_UP,
  ALPACA_MOVE_DOWN = PR_IM_AR_DN,
  ALPACA_NO_MOVE = PR_IM_AR_NA
} direction_alpaca_t;

typedef struct {
  uint8_t byte1;
  uint8_t byte2;
} msg_alpaca_t;

typedef enum {
  MESSAGE_ARROW,
  MESSAGE_FLOOR,
  MESSAGE_MODE,
  MESSAGE_GONG,
  MESSAGE_NONE
} message_type_t;

typedef struct alpaca_parametrs {
  msg_alpaca_t rx_data_alpaca;
  message_type_t message_type;
  bool is_messge_received;
} nku_parametrs_struct;

nku_parametrs_struct alpaca_parametrs = {.rx_data_alpaca =
                                             {
                                                 0x00,
                                             },
                                         .message_type = MESSAGE_NONE,
                                         .is_messge_received = false};

/// Buffer with code location and it's symbols
static const code_location_symbols_t
    special_symbols_code_location[SPECIAL_SYMBOLS_BUFF_SIZE] = {
        {.code_location = PR_IM_LD_ON, .symbols = "pg"},
        {.code_location = PR_IM_ERR_PRESS, .symbols = "A"},
        {.code_location = PR_IM_EVQ_PRESS, .symbols = "E"},
        {.code_location = PR_IM_FL_NA, .symbols = "EF"},
        {.code_location = PR_IM_FRA, .symbols = "F"},
        {.code_location = PR_IM_OVL_1, .symbols = "Kg"}};

/// Structure for data that will be displayed on matrix
static drawing_data_t drawing_data = {0, NO_DIRECTION};

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

/** Stores previous and current state of bit to control buzzer (the front of the
    "Arrival" signal (bit W[3].2) from 0 to 1)
 */
static uint8_t gong[2] = {
    0,
};

/**
 * @brief  Setting gong depend on direction bits in byte W3
 * @note   1. Read Arrival bit using ARRIVAL_MASK and direction using
 *            ARROW_MASK;
 * 		   2. Along the front of the "Arrival" signal (bit W[3].2) from
 *            0 to 1 set play_gong depend on direction. Check previous (gong[0])
 *            and current gong state (gong[1]).
 * @param  direction_byte_w_3: Byte W3
 * @retval None
 */
static void setting_gong(direction_alpaca_t direction_data, uint8_t volume) {

  switch (drawing_data.direction) {
  case DIRECTION_UP:
    play_gong(1, GONG_BUZZER_FREQ, volume);
    break;
  case DIRECTION_DOWN:
    play_gong(2, GONG_BUZZER_FREQ, volume);
    break;
  case NO_DIRECTION:
    play_gong(3, GONG_BUZZER_FREQ, volume);
    break;
  default:
    __NOP();
    play_gong(3, GONG_BUZZER_FREQ, volume);
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
static uint16_t previous_mode = 0;
static uint16_t current_mode = 0;

static void process_code_location(uint16_t current_location) {
  bool is_disable_mode_signal = false;

  switch (alpaca_parametrs.message_type) {
  case MESSAGE_ARROW:
    // if (matrix_string[DIRECTION] != '-' && matrix_string[DIRECTION] != 'p') {
    transform_direction_to_common(data);
    set_direction_symbol(matrix_string, drawing_data.direction);
    // } else { // periodic change direction and symbol "p"/"-"
    // }
    break;

  case MESSAGE_FLOOR:
    /* Этаж неопределен, строка EF */
    if (data == PR_IM_FL_NA) {
      drawing_data.floor = PR_IM_FL_NA;
      is_drawing_data_floor_special = true;
    }

    /* Этажи с 1 по 64 */
    if (data >= PR_IM_FL_01 && data <= PR_IM_FL_64) {
      drawing_data.floor = data % 100 - 3;
      is_drawing_data_floor_special = false;
    }

    break;

  case MESSAGE_MODE:

    // Если Кабинный индикатор, то обрабатываем спец. режимы
    /* Режим погрузки, строка ПГ -> PR_IM_LD_ON */
    /* Неисправность лифта, символ A -> PR_IM_ERR_PRESS */
    if (matrix_settings.addr_id == MAIN_CABIN_ID) {
      is_drawing_data_floor_special = true;

      bool is_previous_mode_higher_priority = false;

      switch (current_location) {

        /* Сигнал о начале режима погрузки, строка ПГ */
      case PR_IM_LD_ON:
        is_previous_mode_higher_priority =
            (previous_mode == PR_IM_ERR_PRESS || previous_mode == PR_IM_OVL_1 ||
             previous_mode == PR_IM_EVQ_PRESS || previous_mode == PR_IM_FRA)
                ? true
                : false;
        break;

        /* Сигнал о завершении режима погрузки, пустая строка "ccc" */
      case PR_IM_LD_OFF:
        // is_disable_mode_signal = true;
        break;

        /* Сигнал о неисправности лифта, символ A */
      case PR_IM_ERR_PRESS:
        is_previous_mode_higher_priority =
            (previous_mode == PR_IM_OVL_1 || previous_mode == PR_IM_EVQ_PRESS ||
             previous_mode == PR_IM_FRA)
                ? true
                : false;
        break;

        /* Сигнал о наличиии перегрузки кабины, строка КГ */
      case PR_IM_OVL_1:
        is_previous_mode_higher_priority =
            (previous_mode == PR_IM_EVQ_PRESS || previous_mode == PR_IM_FRA)
                ? true
                : false;
        break;

        /* Сигнал о завершении перегрузки кабины, пустая строка "ccc" */
      case PR_IM_OVL_0:
        is_disable_mode_signal = true;
        break;

        /* Сигнал о начале режима эвакуации, символ E */
      case PR_IM_EVQ_PRESS:
        is_previous_mode_higher_priority =
            (previous_mode == PR_IM_FRA) ? true : false;
        break;

        /* Сигнал о начале пожарной опасности/сейсмической активности,
         * символ F */
      case PR_IM_FRA:
        is_previous_mode_higher_priority = false;
        break;
      }

      /* Если ПРЕДЫДУЩИЙ режим имеет бОльший приоритет, то отображаем его,
       * иначе отображаем ТЕКУЩИЙ режим (он имеет бОльший приотритет) */
      if (is_previous_mode_higher_priority) {
        current_mode = previous_mode;
        drawing_data.floor = current_mode;
      } else {
        drawing_data.floor = current_location;
      }
    }

    break;

  case MESSAGE_GONG:

    break;

  default:
    break;
  }

  //================================ Настройка matrix_string для отображения
  /* Если спец. режим */
  if (is_drawing_data_floor_special) {

    if (is_disable_mode_signal) {
      matrix_string[DIRECTION] = 'c';
      matrix_string[MSB] = 'c';
      matrix_string[LSB] = 'c';
    } else {
      set_floor_symbols(
          matrix_string, drawing_data.floor, MAX_POSITIVE_NUMBER_LOCATION,
          special_symbols_code_location, SPECIAL_SYMBOLS_BUFF_SIZE);
    }

  } else {
    /* Если этаж */
    if (matrix_settings.group_id <= MAX_P_FLOOR_SHIFT_INDEX) {

      /* Если текущий этаж от СУЛ больше установленного значения сдвига
       * matrix_settings.group_id, то из значения этажа вычетаем значение сдвига
       */
      if (drawing_data.floor > matrix_settings.group_id) {
        shifted_floor = drawing_data.floor - matrix_settings.group_id;
        set_floor_symbols(
            matrix_string, shifted_floor, MAX_POSITIVE_NUMBER_LOCATION,
            special_symbols_code_location, SPECIAL_SYMBOLS_BUFF_SIZE);

      } else {

        /* Если текущий этаж меньше или равен сдвигу
         * matrix_settings.group_id, то из значения сдвига вычетаем значение
         * этажа от СУЛ и прибавляем единицу.
         * Например, этаж 1, сдвиг 3 -> отображаемый этаж: -3/П3.
         */
        shifted_floor = matrix_settings.group_id - drawing_data.floor + 1;

        /* Этажи П1...П9 и П10 */
        matrix_string[MSB] = (shifted_floor <= MAX_P_FLOOR_SHIFT_INDEX - 1)
                                 ? 'p'
                                 : convert_int_to_char(shifted_floor / 10);
        matrix_string[LSB] = (shifted_floor <= 9)
                                 ? convert_int_to_char(shifted_floor)
                                 : convert_int_to_char(shifted_floor % 10);
      }

      /* Если сдвиг в диапазоне от MIN_MINUS_FLOOR_SHIFT_INDEX = 11
       * до ADDR_ID_LIMIT = 73 -> -1..-63 */
    } else if (matrix_settings.group_id >= MIN_MINUS_FLOOR_SHIFT_INDEX &&
               matrix_settings.group_id <= ADDR_ID_LIMIT) {

      if (drawing_data.floor >
          matrix_settings.group_id - MAX_P_FLOOR_SHIFT_INDEX) {
        shifted_floor = drawing_data.floor -
                        (matrix_settings.group_id - MAX_P_FLOOR_SHIFT_INDEX);
        set_floor_symbols(
            matrix_string, shifted_floor, MAX_POSITIVE_NUMBER_LOCATION,
            special_symbols_code_location, SPECIAL_SYMBOLS_BUFF_SIZE);
      } else {

        shifted_floor = matrix_settings.group_id - drawing_data.floor -
                        MAX_P_FLOOR_SHIFT_INDEX + 1;

        if (shifted_floor <= 9) {
          // matrix_string[DIRECTION] = 'c';
          matrix_string[MSB] = '-';
          matrix_string[LSB] = convert_int_to_char(shifted_floor);
        } else {
          // matrix_string[DIRECTION] = '-';
          matrix_string[MSB] = convert_int_to_char(shifted_floor / 10);
          matrix_string[LSB] = convert_int_to_char(shifted_floor % 10);
        }
      }
    }
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

uint32_t lading_edge[2] = {
    0,
};
uint32_t overload_edge[2] = {
    0,
};
uint32_t gong_edge[2] = {
    0,
};

static void setting_sound_alpaca(uint16_t current_location) {

  if (matrix_settings.volume == VOLUME_0) {
    return;
  }

  switch (alpaca_parametrs.message_type) {

  case MESSAGE_GONG:
    if (!is_fire_danger_sound) {
      setting_gong(current_location, matrix_settings.volume);
    }
    break;

  case MESSAGE_MODE:

#if 1
    /* Начало режима Перегрузка кабины */
    if (current_location == PR_IM_OVL_1) {
      is_cabin_overload_sound = true;
      TIM2_Start_bip(BUZZER_FREQ_CABIN_OVERLOAD, VOLUME_1);
    }

    /* Завершение режима Перегрузка кабины */
    if (current_location == PR_IM_OVL_0) {
      is_cabin_overload_sound = false;
      stop_buzzer_sound();
    }
#endif

#if 0

    if (current_location == PR_IM_OVL_1) {
      overload_edge[0] = 1;
    } else if (current_location == PR_IM_OVL_0) {
      overload_edge[0] = 0;
    }
    if (overload_edge[0] && !overload_edge[1]) {
      /* Перегруз */
      TIM2_Start_bip(BUZZER_FREQ_CABIN_OVERLOAD, matrix_settings.volume);
    } else if (!overload_edge[0] && overload_edge[1]) {
      /* Нет перегруза */
      stop_buzzer_sound();
    }
    overload_edge[1] = overload_edge[0];
#endif

    /* Пожарная опасность/сейсмическая активность или эвакуация */
    if (current_location == PR_IM_FRA || current_location == PR_IM_EVQ_PRESS) {
      TIM2_Start_bip(BUZZER_FREQ_FIRE_DANGER, VOLUME_2);
      is_fire_danger_sound = true;
    }

#if 0
    else if (is_fire_danger_sound) {
      is_fire_danger_sound = false;
      stop_buzzer_sound();
    }
#endif

    break;

  default:
    break;
  }
}

void process_data_alpaca() {

  //========== Запись новых полученных данных в alpaca_parametrs.rx_data_alpaca
  if (is_can_data_received()) {
    reset_value_data_received();

    CAN_Data_Message_t *received_msg = get_received_data_by_can();
    uint8_t shift_value = matrix_settings.group_id;

    // Копируем 8 байт из массива received_msg.rx_data_can в структуру
    // alpaca_parametrs.rx_data_alpaca
    memcpy(&alpaca_parametrs.rx_data_alpaca, received_msg->rx_data_can,
           sizeof(msg_alpaca_t));
  }
  /// Flag to control is data received by CAN
  extern volatile bool is_data_received;

  //=============================== Обработка полученных данных
  uint8_t first_byte = alpaca_parametrs.rx_data_alpaca.byte1;  // MSB
  uint8_t second_byte = alpaca_parametrs.rx_data_alpaca.byte2; // LSB

  data = (first_byte << 8) | second_byte;

  previous_mode = current_mode;
  current_mode = data;

  /* Определяем тип данных */
  switch (data) {
  case PR_IM_AR_DN ... PR_IM_AR_UP:
    alpaca_parametrs.message_type = MESSAGE_ARROW;
    break;

  case PR_IM_FL_NA ... PR_IM_FL_64:
    alpaca_parametrs.message_type = MESSAGE_FLOOR;
    break;

  case PR_IM_LD_ON ... PR_IM_EVQ_PRESS:
    alpaca_parametrs.message_type = MESSAGE_MODE;
    break;

  case PR_CM_BP_00:
    alpaca_parametrs.message_type = MESSAGE_GONG;
    break;

    /* Если пришли некорректные данные/данных нет (data = 0),
     * то отображаем -- */
  default:
    alpaca_parametrs.message_type = MESSAGE_NONE;
    break;
  }

  /* Если пришли некорректные данные/данных нет (data = 0),
   * то отображаем -- */
  if (alpaca_parametrs.message_type == MESSAGE_NONE) {
    draw_string_on_matrix("c--");
  } else {
    /* Если данные корректны */

    /* Обработка символов для отображения */
    process_code_location(data);

    /* Обработка звуковых сигналов */
    /* Кабинный индикатор */
    if (matrix_settings.addr_id == MAIN_CABIN_ID) {
      setting_sound_alpaca(data);
    } else {
      /*  Этажный индикатор */
      // if (matrix_settings.addr_id == drawing_data.floor) {
      if (matrix_settings.addr_id == drawing_data.floor) {
        if (matrix_settings.volume != VOLUME_0) {
          if (alpaca_parametrs.message_type == MESSAGE_GONG) {
            setting_gong(data, matrix_settings.volume);
          }
        }
      }
    }

    while (is_data_received == false && is_interface_connected == true) {
      draw_string_on_matrix(matrix_string);
    }
  }
}
