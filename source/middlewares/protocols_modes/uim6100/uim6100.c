/**
 * @file uim6100.c
 */
#include "uim6100.h"

#include "buzzer.h"
#include "config.h"
#include "drawing.h"
#include "tim.h"

#include <stdbool.h>

#define ARROW_MASK                                                             \
  0x03 ///< Mask for direction of movement (0 and 1st bits of byte W3)
#define ARRIVAL_MASK 0b100 ///< Mask for bit arrival (2nd bit of byte W3)
#define ARRIVAL_VALUE 4    ///< Bit value of arrival (bits: 100)
#define CODE_MESSAGE_W_1_MASK 0b00111111

#define CODE_FLOOR_W_2_MASK 0x3F

#define GONG_BUZZER_FREQ                                                       \
  1000 ///< Frequency of bip for start UPWARD, DOWNWARD and ARRIVAL
#define BUZZER_FREQ_CABIN_OVERLOAD                                             \
  3000 ///< Frequency of sound for VOICE_CABIN_OVERLOAD
#define BUZZER_FREQ_FIRE_DANGER                                                \
  BUZZER_FREQ_CABIN_OVERLOAD ///< Frequency of sound for VOICE_FIRE_DANGER

#define SPECIAL_SYMBOLS_BUFF_SIZE 19 ///< Number of special symbols

/**
 * Stores indexes of bytes of UIM6100 protocol
 */
typedef enum UIM_PACKET_BYTES {
  BYTE_CODE_OPERATION_0 = 0,
  BYTE_CODE_OPERATION_1,
  BYTE_W_0,
  BYTE_W_1,
  BYTE_W_2,
  BYTE_W_3
} uim6100_packet_bytes_t;

/// Index of byte_0 for extern in can.c
uint8_t byte_code_operation_0 = BYTE_CODE_OPERATION_0;

/// Index of byte_1 for extern in can.c
uint8_t byte_code_operation_1 = BYTE_CODE_OPERATION_1;

/**
 * Stores direction of movement (UIM6100)
 */
typedef enum {
  UIM_6100_MOVE_UP = 2,
  UIM_6100_MOVE_DOWN = 1,
  UIM_6100_NO_MOVE = 0
} direction_uim_6100_t;

/**
 * Stores values of byte W1 (code message), without floors
 */
typedef enum CODE_MSG {
  VOICE_LIFT_IS_NOT_WORK = 55,
  VOICE_FIRE_DANGER = 56,
  VOICE_CABIN_OVERLOAD = 57,
  VOICE_DOORS_CLOSING = 58,
  VOICE_DOORS_OPENING = 59,
  SOUND_DOORS_CLOSING = 60,
  SOUND_DOORS_OPENING = 61,
  BUTTON_SOUND_SHORT = 62,
  MUTE_SOUND = 63
} code_msg_t;

/**
 * Stores values of byte W2 (code floor)
 */
typedef enum CODE_FLOOR {
  FLOOR_MINUS_1 = 41,
  FLOOR_MINUS_2 = 42,
  FLOOR_MINUS_3 = 43,
  FLOOR_MINUS_4 = 44,
  FLOOR_MINUS_5 = 45,
  FLOOR_MINUS_6 = 46,
  FLOOR_MINUS_7 = 47,
  FLOOR_MINUS_8 = 48,
  FLOOR_MINUS_9 = 49,
  RESERVE = 50,
  SEISMIC_DANGER = 51,
  LIFT_NOT_WORK = 52,
  TRANSFER_FIREFIGHTERS = 53,
  CODE_FLOOR_54 = 54,
  SERVICE = 55,
  EVACUATION = 56,
  FIRE_DANGER = 57,
  FAULT_IBP = 58,
  LOADING = 59
} code_floor_t;

/// Buffer with code location and it's symbols
static const code_location_symbols_t
    special_symbols_code_location[SPECIAL_SYMBOLS_BUFF_SIZE] = {
        {.code_location = RESERVE, .symbols = "b"},
        {.code_location = SEISMIC_DANGER, .symbols = "L"},
        {.code_location = LIFT_NOT_WORK, .symbols = "A"},
        {.code_location = TRANSFER_FIREFIGHTERS, .symbols = "P"},
        {.code_location = CODE_FLOOR_54, .symbols = "H"},
        {.code_location = SERVICE, .symbols = "C"},
        {.code_location = EVACUATION, .symbols = "E"},
        {.code_location = FIRE_DANGER, .symbols = "F"},
        {.code_location = FAULT_IBP, .symbols = "U"},
        {.code_location = LOADING, .symbols = "p"},
        {.code_location = FLOOR_MINUS_1, .symbols = "-1"},
        {.code_location = FLOOR_MINUS_2, .symbols = "-2"},
        {.code_location = FLOOR_MINUS_3, .symbols = "-3"},
        {.code_location = FLOOR_MINUS_4, .symbols = "-4"},
        {.code_location = FLOOR_MINUS_5, .symbols = "-5"},
        {.code_location = FLOOR_MINUS_6, .symbols = "-6"},
        {.code_location = FLOOR_MINUS_7, .symbols = "-7"},
        {.code_location = FLOOR_MINUS_8, .symbols = "-8"},
        {.code_location = FLOOR_MINUS_9, .symbols = "-9"},
};

/// Flag to control is button pressed
static bool is_button_pressed = false;

/// Counter for number received data (order button is pressed)
static uint8_t order_button_cnt = 0;

/// Counter for number received data (order button is disable sound)
static uint8_t button_disable_cnt = 0;

/// Flag to control is cabin overloaded
static bool is_cabin_overload = false;

/// Flag to control fire danger voice
static bool is_fire_danger = false;

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
static void setting_gong(uint8_t direction_byte_w_3, uint8_t volume) {
  direction_uim_6100_t direction = direction_byte_w_3 & ARROW_MASK;
  uint8_t arrival = direction_byte_w_3 & ARRIVAL_MASK;

  // if signal 0 is changing to signal 1, then arrival on floor
  gong[0] = (arrival == ARRIVAL_VALUE) != 0 ? 1 : 0;

  if (gong[0] && !gong[1]) {

    switch (direction) {
    case UIM_6100_MOVE_UP:
      play_gong(1, GONG_BUZZER_FREQ, volume);
      break;
    case UIM_6100_MOVE_DOWN:
      play_gong(2, GONG_BUZZER_FREQ, volume);
      break;
    case UIM_6100_NO_MOVE:
      play_gong(3, GONG_BUZZER_FREQ, volume);
      break;
    default:
      __NOP();
      play_gong(3, GONG_BUZZER_FREQ, volume);
      break;
    }
  }
  gong[1] = gong[0];
}

/**
 * @brief  Process code message from byte W1, turn on/off buzzer
 * @param  code_msg_byte_w_1: Byte W1
 * @retval None
 */
static void process_code_msg(uint8_t code_msg_byte_w_1, volume_t level_volume) {
  if ((code_msg_byte_w_1 & CODE_MESSAGE_W_1_MASK) == VOICE_CABIN_OVERLOAD) {

    if (matrix_settings.volume != VOLUME_0) {
      is_cabin_overload = true;
      TIM2_Start_bip(BUZZER_FREQ_CABIN_OVERLOAD, VOLUME_3);
    }
    matrix_string[DIRECTION] = 'c';
    matrix_string[MSB] = 'K';
    matrix_string[LSB] = 'g';

  }
  // next received bytes by CAN
  else if (is_cabin_overload) {
    TIM2_Stop_bip();
    is_cabin_overload = false;
  }

  if ((code_msg_byte_w_1 & CODE_MESSAGE_W_1_MASK) == VOICE_FIRE_DANGER) {

    if (matrix_settings.volume != VOLUME_0) {
      is_fire_danger = true;
      TIM2_Start_bip(BUZZER_FREQ_CABIN_OVERLOAD, VOLUME_3);
    }
  }
  // next received bytes by CAN
  else if (is_fire_danger) {
    TIM2_Stop_bip();
    is_fire_danger = false;
  }
}

/**
 * @brief  Process code message from byte W1, turn on/off buzzer
 * @param  code_msg_byte_w_1: Byte W1
 * @retval None
 */
static void setting_sound_uim(msg_t *msg, volume_t level_volume) {

  uint8_t code_msg_byte_w_1 = msg->w1;

  /* Нажатие кнопки приказа */
  if ((code_msg_byte_w_1 & CODE_MESSAGE_W_1_MASK) == BUTTON_SOUND_SHORT) {

    if (button_disable_cnt == 0) {
      if (matrix_settings.volume != VOLUME_0) {
        is_button_pressed = true;
        play_gong(1, 1000, matrix_settings.volume);
      }
    }

    /** Не воспроизводить последующие нажатия, если кнопка уже нажата;
     * продолжительность BIP_DURATION_MS 500
     */
    if (is_button_pressed) {
      button_disable_cnt++;
      if (button_disable_cnt == 3) {
        button_disable_cnt = 0;
        is_button_pressed = false;
      }
    }
  }

  /* Гонг прибытия */
  if (matrix_settings.volume != VOLUME_0) {
    setting_gong(msg->w3, matrix_settings.volume);
  }

  /* Перегруз кабины */
  if ((code_msg_byte_w_1 & CODE_MESSAGE_W_1_MASK) == VOICE_CABIN_OVERLOAD) {

    if (matrix_settings.volume != VOLUME_0) {
      is_cabin_overload = true;
      TIM2_Start_bip(BUZZER_FREQ_CABIN_OVERLOAD, VOLUME_3);
    }

    matrix_string[DIRECTION] = 'c';
    matrix_string[MSB] = 'K';
    matrix_string[LSB] = 'g';
  }
  // next received bytes by CAN
  else if (is_cabin_overload) {
    TIM2_Stop_bip();
    is_cabin_overload = false;
  }

  /* Пожарная опасность */
  if ((code_msg_byte_w_1 & CODE_MESSAGE_W_1_MASK) == VOICE_FIRE_DANGER) {
    is_fire_danger = true;

    if (matrix_settings.volume != VOLUME_0) {
      TIM2_Start_bip(BUZZER_FREQ_FIRE_DANGER, VOLUME_3);
    }

  }
  // next received bytes by CAN
  else if (is_fire_danger) {
    TIM2_Stop_bip();
    is_fire_danger = false;
  }
}

/// Structure for data that will be displayed on matrix
static drawing_data_t drawing_data = {0, 0};

/**
 * @brief  Transform UIM6100 values of direction to common directionType that
 *         defined in drawing.h
 * @param  direction: Value from enum direction_uim_6100_t:
 *                    UIM_6100_MOVE_UP/UIM_6100_MOVE_DOWN/UIM_6100_NO_MOVE
 * @retval None
 */
static void transform_direction_to_common(direction_uim_6100_t direction) {
  switch (direction) {
  case UIM_6100_MOVE_UP:
    drawing_data.direction = DIRECTION_UP;
    break;
  case UIM_6100_MOVE_DOWN:
    drawing_data.direction = DIRECTION_DOWN;
    break;
  case UIM_6100_NO_MOVE:
    drawing_data.direction = NO_DIRECTION;
    break;

  default:
    drawing_data.direction = NO_DIRECTION;
    break;
  }
}

/**
 * @brief  Process data using UIM6100 protocol
 * @note   1. Set drawing_data structure, process code message, setting gong
 *            and symbols;
 *         2. Display matrix_string while next data is not received and
 *            interface is connected.
 * @param  rx_data_can: Pointer to the buffer with received data by CAN
 * @retval None
 */
void process_data_uim(msg_t *msg) {
  /// Flag to control is data received by CAN
  extern volatile bool is_data_received;

  uint8_t code_msg = msg->w1;
  drawing_data.floor = msg->w2 & CODE_FLOOR_W_2_MASK;

  transform_direction_to_common(msg->w3 & ARROW_MASK);

  setting_symbols(matrix_string, &drawing_data, MAX_POSITIVE_NUMBER_LOCATION,
                  special_symbols_code_location, SPECIAL_SYMBOLS_BUFF_SIZE);

  // Кабинный индикатор
  if (matrix_settings.addr_id == MAIN_CABIN_ID) {
#if 1
    /* Проверено на испытаниях */
    setting_gong(msg->w3, matrix_settings.volume);
    process_code_msg(code_msg, matrix_settings.volume);
#elif
    /* Функция для обработки всех звукрвых оповещений была составлена после
     * испытаний, не проверена */
    setting_sound_uim(msg, matrix_settings.volume);
#endif
  } else {
    // Этажный индикатор
    if (matrix_settings.addr_id == drawing_data.floor ||
        matrix_settings.addr_id == 47) {
      if (matrix_settings.volume != VOLUME_0) {
        setting_gong(msg->w3, matrix_settings.volume);
      }
    }
  }

  // while new 6 data bytes are not received, draw str
  while (is_data_received == false && is_interface_connected == true) {
    draw_string_on_matrix(matrix_string);
  }
}
