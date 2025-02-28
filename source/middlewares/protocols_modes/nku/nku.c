/**
 * @file uim6100.c
 */
#include "nku.h"

#include "buzzer.h"
#include "config.h"
#include "drawing.h"
#include "tim.h"

#include "nku.h"
#include <stdbool.h>

#define ARROW_MASK                                                             \
  0x03 ///< Mask for direction of movement (0 and 1st bits of byte DATA7)

#define SYMBOL_MASK 0x3F
#define FIRE_DANGER_MASK 0x40
#define BIT_6_MASK 0x40
#define BIT_7_MASK 0x80

#define ARRIVAL_MASK 0b100 ///< Mask for bit arrival (2nd bit of byte W3)
#define ARRIVAL_VALUE 4    ///< Bit value of arrival (bits: 100)
#define CODE_MESSAGE_W_1_MASK 0b00111111

#define CODE_FLOOR_W_2_MASK 0x3F

#define GONG_BUZZER_FREQ                                                       \
  1000 ///< Frequency of bip for start UPWARD, DOWNWARD and ARRIVAL
#define BUZZER_FREQ_CABIN_OVERLOAD                                             \
  3000 ///< Frequency of bip for VOICE_CABIN_OVERLOAD

#define SPECIAL_SYMBOLS_BUFF_SIZE 19 ///< Number of special symbols

typedef struct {
  uint8_t data1;
  uint8_t data2;
  uint8_t data3;
  uint8_t data4;
  uint8_t data5;
  uint8_t data6;
  uint8_t data7;
  uint8_t data8;
} msg_nku_t;

typedef enum {
  PACKAGE_TYPE_1,
  PACKAGE_TYPE_2,
  PACKAGE_TYPE_3,
  PACKAGE_TYPE_4,
  PACKAGE_TYPE_5
} package_type_t;

typedef struct nku_parametrs {
  msg_nku_t rx_data_nku;
  package_type_t package_type;
  bool is_package_received;
} nku_parametrs_struct;

nku_parametrs_struct nku_parametrs = {.rx_data_nku =
                                          {
                                              0x00,
                                          },
                                      .package_type = PACKAGE_TYPE_1,
                                      .is_package_received = false};

/**
 * Stores direction of movement (NKU)
 */
typedef enum {
  NKU_MOVE_UP = 1,
  NKU_MOVE_DOWN = 2,
  NKU_NO_MOVE = 0
} direction_nku_t;

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

typedef enum SYMBOLS {
  SYMBOL_A = 10,                       // Символ A
  SYMBOL_b = 11,                       // Символ b
  SYMBOL_C = 12,                       // Символ C
  SYMBOL_d = 13,                       // Символ d
  SYMBOL_E = 14,                       // Символ E
  SYMBOL_F = 15,                       // Символ F
  SYMBOL_EMPTY = 16,                   // Символ Пусто
  SYMBOL_UNDERGROUND_FLOOR_BIG = 17,   // Символ П
  SYMBOL_P = 18,                       // Символ P
  SYMBOL_UNDERGROUND_FLOOR_SMALL = 19, // Символ п
  SYMBOL_H = 20,                       // Символ H
  SYMBOL_U_BIG = 21,                   // Символ U
  SYMBOL_MINUS = 22,                   // Символ -
  SYMBOL_UNDERSCORE = 23,              // Символ _
  SYMBOL_U_SMALL = 24,                 // Символ u
  SYMBOL_L = 25,                       // Символ L
  SYMBOL_Y_RU = 26,                    // Символ У
  SYMBOL_B_RU = 27,                    // Символ Б
  SYMBOL_G_RU = 28,                    // Символ Г
  SYMBOL_R = 29,                       // Символ R
  SYMBOL_V = 30,                       // Символ V
  SYMBOL_N = 31,                       // Символ N
  SYMBOL_S = 32,                       // Символ S
  SYMBOL_K = 33,                       // Символ K
  SYMBOL_Y = 34,                       // Символ Y
  SYMBOL_G = 35,                       // Символ G
  SYMBOL_B = 36,                       // Символ B
  SYMBOL_T = 37                        // Символ T
} symbols_t;

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
        {.code_location = SYMBOL_A, .symbols = "A"},
        {.code_location = SYMBOL_b, .symbols = "b"},
        {.code_location = SYMBOL_C, .symbols = "C"},
        {.code_location = SYMBOL_d, .symbols = "d"},
        {.code_location = SYMBOL_E, .symbols = "E"},
        {.code_location = SYMBOL_F, .symbols = "F"},
        {.code_location = SYMBOL_EMPTY, .symbols = "c"},
        {.code_location = SYMBOL_UNDERGROUND_FLOOR_BIG, .symbols = "p"},
        {.code_location = SYMBOL_P, .symbols = "P"},

        // {.code_location = SYMBOL_UNDERGROUND_FLOOR_SMALL, .symbols = "P"},

        {.code_location = SYMBOL_H, .symbols = "H"},
        {.code_location = SYMBOL_U_BIG, .symbols = "U"},
        {.code_location = SYMBOL_MINUS, .symbols = "-"},
        // {.code_location = SYMBOL_UNDERSCORE, .symbols = "_"},
        // {.code_location = SYMBOL_U_SMALL, .symbols = "u"},
        {.code_location = SYMBOL_L, .symbols = "L"},
        // {.code_location = SYMBOL_Y_RU, .symbols = "Y"},
        // {.code_location = SYMBOL_B_RU, .symbols = "B"},
        {.code_location = SYMBOL_G_RU, .symbols = "g"},
        // {.code_location = SYMBOL_R, .symbols = "R"},
        {.code_location = SYMBOL_V, .symbols = "V"},
        {.code_location = SYMBOL_N, .symbols = "N"},
        {.code_location = SYMBOL_S, .symbols = "S"},
        {.code_location = SYMBOL_K, .symbols = "K"},
        // {.code_location = SYMBOL_Y, .symbols = "Y"},
        // {.code_location = SYMBOL_G, .symbols = "G"},
        // {.code_location = SYMBOL_B, .symbols = "B"},
        {.code_location = SYMBOL_T, .symbols = "T"}};

/// Flag to control is cabin overloaded
static bool is_cabin_overload = false;

static bool is_fire_danger = false;

/// Structure for data that will be displayed on matrix
static drawing_data_t drawing_data = {0, 0};

/**
 * @brief  Transform UIM6100 values of direction to common directionType that
 *         defined in drawing.h
 * @param  direction: Value from enum direction_nku_t:
 *                    NKU_MOVE_UP/NKU_MOVE_DOWN/NKU_NO_MOVE
 * @retval None
 */
static void transform_direction_to_common(direction_nku_t direction) {
  switch (direction) {
  case NKU_MOVE_UP:
    drawing_data.direction = DIRECTION_UP;
    break;
  case NKU_MOVE_DOWN:
    drawing_data.direction = DIRECTION_DOWN;
    break;
  case NKU_NO_MOVE:
    drawing_data.direction = NO_DIRECTION;
    break;

  default:
    drawing_data.direction = NO_DIRECTION;
    break;
  }
}

/**
 * @brief  Process data using NKU protocol
 * @note   1. Set drawing_data structure, process code message, setting gong
 *            and symbols;
 *         2. Display matrix_string while next data is not received and
 *            interface is connected.
 * @param  msg: Pointer to the structure with received data by CAN
 * @retval None
 */

uint8_t first_symbol_code = 0;
uint8_t second_symbol_code = 0;
uint8_t third_symbol_code = 0;

static uint8_t gong[2] = {
    0,
};

static uint8_t bip_num = 0;
static void setting_gong(msg_nku_t *rx_data_nku, uint8_t volume) {
  direction_nku_t direction = rx_data_nku->data7 & ARROW_MASK;
  uint16_t arrival = rx_data_nku->data4 & BIT_7_MASK;

  // if signal 0 is changing to signal 1, then arrival on floor
  gong[0] = (arrival == BIT_7_MASK) != 0 ? 1 : 0;

  if (gong[0] && !gong[1]) {

    switch (direction) {
    case NKU_MOVE_UP:
      play_gong(1, GONG_BUZZER_FREQ, volume);
      // bip_num = 1;
      break;
    case NKU_MOVE_DOWN:
      play_gong(2, GONG_BUZZER_FREQ, volume);
      // bip_num = 2;
      break;
    case NKU_NO_MOVE:
      play_gong(3, GONG_BUZZER_FREQ, volume);
      // bip_num = 3;
      break;
    default:
      // __NOP();
      // play_gong(3, GONG_BUZZER_FREQ, volume);
      break;
    }
  }
  gong[1] = gong[0];
}

// msg_nku_t msg_nku = {0, 0, 0, 0};

void process_data_nku() {

  if (is_can_data_received()) {
    reset_value_data_received();

    CAN_Data_Package_t received_msg = get_received_data_by_can();

    // Проверяем ID
    switch (received_msg.std_id) {
    case 0x506:
      nku_parametrs.package_type = PACKAGE_TYPE_3;
      break;
    case 0x508:
      nku_parametrs.package_type = PACKAGE_TYPE_4;
      break;
    case 0x50B:
      nku_parametrs.package_type = PACKAGE_TYPE_5;
      break;

    default:
      break;
    }

    // Копируем 8 байт из массива received_msg.rx_data_can в структуру
    // nku_parametrs.rx_data_nku
    memcpy(&nku_parametrs.rx_data_nku, received_msg.rx_data_can,
           sizeof(msg_nku_t));

    switch (nku_parametrs.package_type) {
    case PACKAGE_TYPE_1:

      break;
    case PACKAGE_TYPE_2:

      break;
    case PACKAGE_TYPE_3:
      /* Индикация стрелки дисплея */
      transform_direction_to_common(nku_parametrs.rx_data_nku.data7 &
                                    ARROW_MASK);
      set_direction_symbol(matrix_string, drawing_data.direction);

      /* Гонг прибытия */
      setting_gong(&nku_parametrs.rx_data_nku, matrix_settings.volume);

      break;
    case PACKAGE_TYPE_4:
      first_symbol_code = nku_parametrs.rx_data_nku.data5 & SYMBOL_MASK;
      second_symbol_code = nku_parametrs.rx_data_nku.data6 & SYMBOL_MASK;
      third_symbol_code = nku_parametrs.rx_data_nku.data7 & SYMBOL_MASK;

      is_fire_danger = nku_parametrs.rx_data_nku.data6 & FIRE_DANGER_MASK;

      if (is_fire_danger) {
        matrix_string[MSB] = 'F';
        matrix_string[LSB] = 'c';
      } else

          if (second_symbol_code == SYMBOL_UNDERGROUND_FLOOR_BIG ||
              third_symbol_code == SYMBOL_UNDERGROUND_FLOOR_BIG) {
        if (second_symbol_code == SYMBOL_EMPTY) {
          matrix_string[MSB] = 'p';
          matrix_string[LSB] = 'c';
        } else {
          matrix_string[MSB] = 'p';
          matrix_string[LSB] = convert_int_to_char(third_symbol_code);
        }

        set_direction_symbol(matrix_string, drawing_data.direction);
      } else if (second_symbol_code == SYMBOL_MINUS) {
        matrix_string[MSB] = '-';
        matrix_string[LSB] = convert_int_to_char(third_symbol_code);
        set_direction_symbol(matrix_string, drawing_data.direction);
      } else {
        /* Однозначные значения этажей */
        if (second_symbol_code == SYMBOL_EMPTY) {
          drawing_data.floor = third_symbol_code;
        } else {
          /* Двузначные значения этажей 10..99 и 0 этаж */
          drawing_data.floor = second_symbol_code * 10 + third_symbol_code;
        }
        setting_symbols(
            matrix_string, &drawing_data, MAX_POSITIVE_NUMBER_LOCATION,
            special_symbols_code_location, SPECIAL_SYMBOLS_BUFF_SIZE);
      }
      break;
    case PACKAGE_TYPE_5:
      is_cabin_overload = nku_parametrs.rx_data_nku.data6 & BIT_6_MASK;

      if (is_cabin_overload) {
        matrix_string[DIRECTION] = 'c';
        matrix_string[MSB] = 'K';
        matrix_string[LSB] = 'g';
      }
      break;

    default:
      break;
    }

    // while new 8 data bytes are not received, draw str
    while (is_can_data_received() == false && is_interface_connected == true) {
#if DOT_PIN
      draw_string_on_matrix(matrix_string);
#elif DOT_SPI
      display_symbols_spi(matrix_string);
#endif
    }
  }
}