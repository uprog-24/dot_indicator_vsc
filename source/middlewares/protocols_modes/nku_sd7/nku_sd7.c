/**
 * @file uim6100.c
 */
#include "nku_sd7.h"

#include "buzzer.h"
#include "config.h"
#include "drawing.h"
#include "font.h"
#include "tim.h"

#include <stdbool.h>

#define DATA_BITS_MASK 0x7E
#define ARROW_MASK 0x60 ///< Маска для направления движения

// Маски для control_byte_first
#define LOADING_MASK 0x02
#define FIRE_DANGER_MASK 0x04
#define CABIN_OVERLOAD_MASK 0x08
#define GONG_MASK 0x10

// Маски для control_byte_second
#define ACCIDENT_MASK 0x02
#define CALL_MASK 0x04
#define ORDER_MASK 0x08
#define FIRE_DANGER_SOUND_MASK 0x10

#define GONG_BUZZER_FREQ 1000 ///< Частота первого тона гонга
#define BUZZER_FREQ_CABIN_OVERLOAD                                             \
  3000 ///< Частота для тона бузера при перегрузе кабины VOICE_CABIN_OVERLOAD
#define BUZZER_FREQ_FIRE_DANGER                                                \
  BUZZER_FREQ_CABIN_OVERLOAD ///< Частота для тона бузера при пожарной опасности
                             ///< VOICE_FIRE_DANGER

#define SPECIAL_SYMBOLS_BUFF_SIZE 19 ///< Кол-во спец. символов

#define MESSAGE_BYTES 4 ///< Длина сообщения в байтах

#define FILTER_BUFF_SIZE                                                       \
  5 ///< Size of buffer with received data (width of filter)

uint8_t byte_buf[4] = {0, 0, 0, 0}; ///< Буфер для полученных данных
uint8_t byte_buf_copy[4] = {0, 0, 0, 0}; ///< Буфер для полученных данных

/**
 * Направления движения.
 */
typedef enum {
  NKU_SD7_MOVE_UP = 64,
  NKU_SD7_MOVE_DOWN = 32,
  NKU_SD7_NO_MOVE = 0
} direction_nku_sd7_t;

typedef enum SYMBOLS {
  NKU_SYMBOL_0 = 0,
  NKU_SYMBOL_1 = 1,
  NKU_SYMBOL_2 = 2,
  NKU_SYMBOL_3 = 3,
  NKU_SYMBOL_4 = 4,
  NKU_SYMBOL_5 = 5,
  NKU_SYMBOL_6 = 6,
  NKU_SYMBOL_7 = 7,
  NKU_SYMBOL_8 = 8,
  NKU_SYMBOL_9 = 9,
  NKU_SYMBOL_A = 10,                       // Символ A
  NKU_SYMBOL_b = 11,                       // Символ b
  NKU_SYMBOL_C = 12,                       // Символ C
  NKU_SYMBOL_d = 13,                       // Символ d
  NKU_SYMBOL_E = 14,                       // Символ E
  NKU_SYMBOL_F = 15,                       // Символ F
  NKU_SYMBOL_EMPTY = 16,                   // Символ Пробел
  NKU_SYMBOL_UNDERGROUND_FLOOR_BIG = 17,   // Символ П
  NKU_SYMBOL_P = 18,                       // Символ P
  NKU_SYMBOL_UNDERGROUND_FLOOR_SMALL = 19, // Символ п
  NKU_SYMBOL_H = 20,                       // Символ H
  NKU_SYMBOL_U_BIG = 21,                   // Символ U
  NKU_SYMBOL_MINUS = 22,                   // Символ -
  NKU_SYMBOL_UNDERSCORE = 23,              // Символ _
  NKU_SYMBOL_U_SMALL = 24,                 // Символ u
  NKU_SYMBOL_L = 25,                       // Символ L
  NKU_SYMBOL_Y_RU = 26,                    // Символ У
  NKU_SYMBOL_B_RU = 27,                    // Символ Б
  NKU_SYMBOL_G_RU = 28,                    // Символ Г
  NKU_SYMBOL_R = 29,                       // Символ R
  NKU_SYMBOL_V = 30,                       // Символ V
  NKU_SYMBOL_N = 31,                       // Символ N
  NKU_SYMBOL_S = 32,                       // Символ S
  NKU_SYMBOL_K = 33,                       // Символ K
  NKU_SYMBOL_Y = 34,                       // Символ Y
  NKU_SYMBOL_G = 35,                       // Символ G
  NKU_SYMBOL_B = 36,                       // Символ B
  NKU_SYMBOL_T = 37                        // Символ T
} symbols_t;

// typedef struct {
//   uint16_t code_location;
//   char symbol_char;
// } code_location_char_t;

// static const code_location_char_t char_code_location[10] = {
//     {.code_location = SYMBOL_0, .symbol_char = '0'},
//     {.code_location = SYMBOL_1, .symbol_char = '1'},
//     {.code_location = SYMBOL_2, .symbol_char = '2'},
//     {.code_location = SYMBOL_3, .symbol_char = '3'},
//     {.code_location = SYMBOL_4, .symbol_char = '4'},
//     {.code_location = SYMBOL_5, .symbol_char = '5'},
//     {.code_location = SYMBOL_6, .symbol_char = '6'},
//     {.code_location = SYMBOL_7, .symbol_char = '7'},
//     {.code_location = SYMBOL_8, .symbol_char = '8'},
//     {.code_location = SYMBOL_9, .symbol_char = '9'}};

// /// Буфер со спец. символами
// static const code_location_symbols_t
//     special_symbols_code_location[SPECIAL_SYMBOLS_BUFF_SIZE] = {
//         {.code_location = SYMBOL_A, .symbols = "A"},
//         {.code_location = SYMBOL_b, .symbols = "b"},
//         {.code_location = SYMBOL_C, .symbols = "C"},
//         {.code_location = SYMBOL_d, .symbols = "d"},
//         {.code_location = SYMBOL_E, .symbols = "E"},
//         {.code_location = SYMBOL_F, .symbols = "F"},
//         {.code_location = SYMBOL_EMPTY, .symbols = "c"},
//         {.code_location = SYMBOL_UNDERGROUND_FLOOR_BIG, .symbols = "p"},
//         {.code_location = SYMBOL_P, .symbols = "P"},

//         // {.code_location = SYMBOL_UNDERGROUND_FLOOR_SMALL, .symbols = "P"},

//         {.code_location = SYMBOL_H, .symbols = "H"},
//         {.code_location = SYMBOL_U_BIG, .symbols = "U"},
//         {.code_location = SYMBOL_MINUS, .symbols = "-"},
//         // {.code_location = SYMBOL_UNDERSCORE, .symbols = "_"},
//         // {.code_location = SYMBOL_U_SMALL, .symbols = "u"},
//         {.code_location = SYMBOL_L, .symbols = "L"},
//         // {.code_location = SYMBOL_Y_RU, .symbols = "Y"},
//         // {.code_location = SYMBOL_B_RU, .symbols = "B"},
//         {.code_location = SYMBOL_G_RU, .symbols = "g"},
//         // {.code_location = SYMBOL_R, .symbols = "R"},
//         {.code_location = SYMBOL_V, .symbols = "V"},
//         // {.code_location = SYMBOL_N, .symbols = "N"},
//         {.code_location = SYMBOL_S, .symbols = "S"},
//         {.code_location = SYMBOL_K, .symbols = "K"},
//         // {.code_location = SYMBOL_Y, .symbols = "Y"},
//         // {.code_location = SYMBOL_G, .symbols = "G"},
//         // {.code_location = SYMBOL_B, .symbols = "B"},
//         {.code_location = SYMBOL_T, .symbols = "T"}};

/// Структура с данными для отображения (direction, floor).
static drawing_data_t drawing_data = {0, 0};

static void transform_direction_to_common(direction_nku_sd7_t direction) {
  switch (direction) {
  case NKU_SD7_MOVE_UP:
    drawing_data.direction = DIRECTION_UP;
    break;
  case NKU_SD7_MOVE_DOWN:
    drawing_data.direction = DIRECTION_DOWN;
    break;
  case NKU_SD7_NO_MOVE:
    drawing_data.direction = NO_DIRECTION;
    break;

  default:
    drawing_data.direction = NO_DIRECTION;
    break;
  }
}

static inline symbol_e direction_to_symbol(direction_nku_sd7_t direction) {
  switch (direction) {
  case NKU_SD7_MOVE_UP:
    return SYMBOL_ARROW_UP;
  case NKU_SD7_MOVE_DOWN:
    return SYMBOL_ARROW_DOWN;
  default:
    return SYMBOL_EMPTY;
  }
}

static inline symbol_e map_to_common_symbol(uint8_t symbol_code) {
  switch (symbol_code) {
  case NKU_SYMBOL_0:
    return SYMBOL_ZERO;
  case NKU_SYMBOL_1:
    return SYMBOL_ONE;
  case NKU_SYMBOL_2:
    return SYMBOL_TWO;
  case NKU_SYMBOL_3:
    return SYMBOL_THREE;
  case NKU_SYMBOL_4:
    return SYMBOL_FOUR;
  case NKU_SYMBOL_5:
    return SYMBOL_FIVE;
  case NKU_SYMBOL_6:
    return SYMBOL_SIX;
  case NKU_SYMBOL_7:
    return SYMBOL_SEVEN;
  case NKU_SYMBOL_8:
    return SYMBOL_EIGHT;
  case NKU_SYMBOL_9:
    return SYMBOL_NINE;

  case NKU_SYMBOL_A:
    return SYMBOL_A;
  case NKU_SYMBOL_b:
    return SYMBOL_b;
  case NKU_SYMBOL_C:
    return SYMBOL_C;
  case NKU_SYMBOL_d:
    return SYMBOL_d;
  case NKU_SYMBOL_E:
    return SYMBOL_E;
  case NKU_SYMBOL_F:
    return SYMBOL_F;
  case NKU_SYMBOL_EMPTY:
    return SYMBOL_EMPTY;

  case NKU_SYMBOL_UNDERGROUND_FLOOR_BIG:
    return SYMBOL_UNDERGROUND_FLOOR_BIG; // Символ П
  case NKU_SYMBOL_P:
    return SYMBOL_P; // Символ P

  case NKU_SYMBOL_UNDERGROUND_FLOOR_SMALL:
    return SYMBOL_UNDERGROUND_FLOOR_SMALL; // Символ п
  case NKU_SYMBOL_H:
    return SYMBOL_H; // Символ H
  case NKU_SYMBOL_U_BIG:
    return SYMBOL_U_BIG; // Символ U
  case NKU_SYMBOL_MINUS:
    return SYMBOL_MINUS; // Символ -
  case NKU_SYMBOL_UNDERSCORE:
    return SYMBOL_UNDERSCORE; // Символ _
  case NKU_SYMBOL_U_SMALL:
    return SYMBOL_U_SMALL; // Символ u
  case NKU_SYMBOL_L:
    return SYMBOL_L; // Символ L
  case NKU_SYMBOL_Y_RU:
    return SYMBOL_Y_RU; // Символ У
  case NKU_SYMBOL_B_RU:
    return SYMBOL_B_RU; // Символ Б
  case NKU_SYMBOL_G_RU:
    return SYMBOL_G_RU; // Символ Г
  case NKU_SYMBOL_R:
    return SYMBOL_R; // Символ R
  case NKU_SYMBOL_V:
    return SYMBOL_V; // Символ V
  case NKU_SYMBOL_N:
    return SYMBOL_N; // Символ N
  case NKU_SYMBOL_S:
    return SYMBOL_S; // Символ S
  case NKU_SYMBOL_K:
    return SYMBOL_K; // Символ K
  case NKU_SYMBOL_Y:
    return SYMBOL_Y; // Символ Y
  case NKU_SYMBOL_G:
    return SYMBOL_G; // Символ G
  case NKU_SYMBOL_B:
    return SYMBOL_B; // Символ B
  case NKU_SYMBOL_T:
    return SYMBOL_T; // Символ T

  default:
    return SYMBOL_EMPTY;
  }
}

static void set_loading_symbol(uint8_t control_byte_first) {
  if ((control_byte_first & LOADING_MASK) == LOADING_MASK) {
    symbols.symbol_code_1 = map_to_common_symbol(NKU_SYMBOL_EMPTY);
    symbols.symbol_code_2 =
        map_to_common_symbol(NKU_SYMBOL_UNDERGROUND_FLOOR_BIG);
    symbols.symbol_code_3 = map_to_common_symbol(NKU_SYMBOL_G_RU);
  }
}

/// Флаг для контроля перегруза кабины
static bool is_cabin_overload = false;

static void set_cabin_overload_symbol_sound(uint8_t control_byte_first) {
  if ((control_byte_first & CABIN_OVERLOAD_MASK) == CABIN_OVERLOAD_MASK) {

    if (matrix_settings.volume != VOLUME_0) {
      is_cabin_overload = true;
      start_buzzer_sound(BUZZER_FREQ_CABIN_OVERLOAD, VOLUME_3);
    }

    symbols.symbol_code_1 = map_to_common_symbol(NKU_SYMBOL_EMPTY);
    symbols.symbol_code_2 = map_to_common_symbol(NKU_SYMBOL_K);
    symbols.symbol_code_3 = map_to_common_symbol(NKU_SYMBOL_G_RU);
  } else if (is_cabin_overload) {
    stop_buzzer_sound();
    is_cabin_overload = false;
  }
}

static void set_accident_symbol(uint8_t control_byte_second) {
  if ((control_byte_second & ACCIDENT_MASK) == ACCIDENT_MASK) {
    symbols.symbol_code_2 = map_to_common_symbol(NKU_SYMBOL_A);
    symbols.symbol_code_3 = map_to_common_symbol(NKU_SYMBOL_EMPTY);
  }
}

static bool is_fire_danger_symbol = false;

static void set_fire_danger_symbol(uint8_t control_byte_first) {
  if ((control_byte_first & FIRE_DANGER_MASK) == FIRE_DANGER_MASK) {
    is_fire_danger_symbol = true;
    symbols.symbol_code_2 = map_to_common_symbol(NKU_SYMBOL_F);
    symbols.symbol_code_3 = map_to_common_symbol(NKU_SYMBOL_EMPTY);
  }
}

static uint8_t fire_sound_edge[2] = {
    0,
};

static void set_fire_danger_sound(uint8_t control_byte_second) {
  uint8_t fire_danger_sound_bit = control_byte_second & FIRE_DANGER_SOUND_MASK;

  fire_sound_edge[0] =
      (fire_danger_sound_bit == FIRE_DANGER_SOUND_MASK) != 0 ? 1 : 0;

  if (fire_sound_edge[0] && !fire_sound_edge[1]) {
    play_gong(1, BUZZER_FREQ_FIRE_DANGER, VOLUME_3);
  }
  fire_sound_edge[1] = fire_sound_edge[0];
}

static uint8_t gong[2] = {
    0,
};

static void setting_gong(uint8_t direction_code, uint8_t control_byte_first,
                         uint8_t volume) {
  uint8_t arrival = control_byte_first & GONG_MASK;

  // Если сигнал из 0 меняется на 1, тогда детектируем прибытие на этаж
  gong[0] = (arrival == GONG_MASK) != 0 ? 1 : 0;

  if (gong[0] && !gong[1]) {

    switch (direction_code) {
    case NKU_SD7_MOVE_UP:
      play_gong(1, GONG_BUZZER_FREQ, volume);
      break;
    case NKU_SD7_MOVE_DOWN:
      play_gong(2, GONG_BUZZER_FREQ, volume);
      break;
    case NKU_SD7_NO_MOVE:
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

/// Флаг для контроля воспроизведения оповещения при пожарной опасности
static bool is_fire_danger = false;

/// Флаг контроля 4-х байтов
volatile bool is_read_data_completed = false;

static void cabin_indicator_special_regime(uint8_t direction_code,
                                           uint8_t control_byte_first,
                                           uint8_t control_byte_second) {

  // Гонг
  if (matrix_settings.volume != VOLUME_0) {
    setting_gong(direction_code, control_byte_first, matrix_settings.volume);
  }

  // Погрузка
  set_loading_symbol(control_byte_first);

  // Перегруз
  set_cabin_overload_symbol_sound(control_byte_first);

  // Авария
  set_accident_symbol(control_byte_second);

  // Пожар, символ F
  set_fire_danger_symbol(control_byte_first);

  // Пожар, звук по фронту
  if (matrix_settings.volume != VOLUME_0) {
    set_fire_danger_sound(control_byte_second);
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
  set_accident_symbol(control_byte_second);

  // Пожар, символ F
  set_fire_danger_symbol(control_byte_first);
}

/// Flag to control is data filtered before displaying it on matrix
static bool is_data_filtered = false;

/// Number of received data
static uint8_t number_received_data = 0;

/// Current index of the element in filter_buff
static uint8_t current_index_buff = 0;

/**
 * Stores the parameters for filtering received data that will be displayed on
 * matrix: floor, direction, control_bits and counter that save number of
 * repetitions of the data
 */
typedef struct {
  uint8_t buffer[4];
  uint8_t counter;
} floor_counter_t;

/// Buffer that store received data and its repetitions
static floor_counter_t filter_buff[FILTER_BUFF_SIZE];

/// Copy of received_data_ukl (13 bits received by UKL)
static volatile uint16_t received_data_ukl_copy = 1;

/**
 * @brief  Setting structure with type floor_counter_t
 * @param  filter_struct: Pointer to the structure with type floor_counter_t
 * @param  floor:         Received floor
 * @param  direction:     Received direction
 * @param  control_bits:  Received control_bits
 * @param  counter:       Save number of repetitions of the data
 * @retval None
 */
static void set_filter_structure(floor_counter_t *filter_struct,
                                 uint8_t *buffer, uint8_t counter) {
  memcpy(filter_struct->buffer, buffer, sizeof(filter_struct->buffer));
  filter_struct->counter = counter;
}

/**
 * @brief  Sorting filter_buff in descending order using the bubble method.
 * @note   filter_buff[0].counter has maximum value and will be displayed on
 *         matrix
 * @param  filter_buff: Pointer to the buffer with received data by UKL
 * @param  buff_size:   Size of filter_buff
 * @retval None
 */
static void sort_bubble(floor_counter_t *filter_buff, uint8_t buff_size) {
  for (uint8_t i = 0; i < buff_size - 1; i++) {
    for (uint8_t k = 0; k < buff_size - i - 1; k++) {
      if (filter_buff[k].counter < filter_buff[k + 1].counter) {
        floor_counter_t temp = filter_buff[k];
        filter_buff[k] = filter_buff[k + 1];
        filter_buff[k + 1] = temp;
      }
    }
  }
}

uint8_t zero_buffer[4] = {0};

static void filter_data() {
  if (current_index_buff == 0) {
    for (uint8_t i = 0; i < FILTER_BUFF_SIZE; i++) {
      set_filter_structure(&filter_buff[i], zero_buffer, 0);
    }
    current_index_buff = 0;
  }

  // uint8_t floor = received_data_ukl_copy & CODE_FLOOR_MASK;
  // direction_ukl_t direction = received_data_ukl_copy & DIRECTION_MASK;
  // control_bits_states_t control_bits =
  //     received_data_ukl_copy & CONTROL_BITS_MASK;

  bool is_data_found = false;
  number_received_data++;

  for (uint8_t i = 0; i < current_index_buff; i++) {
    if (filter_buff[i].buffer[0] == byte_buf_copy[0] &&
        filter_buff[i].buffer[1] == byte_buf_copy[1] &&
        filter_buff[i].buffer[2] == byte_buf_copy[2] &&
        filter_buff[i].buffer[3] == byte_buf_copy[3]) {
      filter_buff[i].counter++;
      is_data_found = true;
      break;
    }
  }

  if (!is_data_found && current_index_buff < FILTER_BUFF_SIZE - 1) {
    set_filter_structure(&filter_buff[current_index_buff], byte_buf_copy, 1);
    current_index_buff++;
  }

  if (number_received_data == FILTER_BUFF_SIZE) {
    sort_bubble(filter_buff, FILTER_BUFF_SIZE);

    is_data_filtered = true;
    number_received_data = 0;
    current_index_buff = 0;
  }
}

/*
 * Структура для сохранения полученных байтов по последовательному интерфейсу.
 */
typedef struct {
  uint8_t first_symbol_code;
  uint8_t second_symbol_code;
  uint8_t control_byte_first;
  uint8_t control_byte_second;
} msg_t;

msg_t messgae_nku_sd7 = {

};

uint8_t first_symbol_code;
uint8_t second_symbol_code;
uint8_t control_byte_first;
uint8_t control_byte_second;

uint8_t direction_code;

/**
 * @brief  Обработка данных по протоколу NKU_SD7.
 * @param
 * @retval None
 */
void process_data_nku_sd7() {

  filter_data();

  if (is_data_filtered) {
    is_data_filtered = false;

    first_symbol_code = (filter_buff[0].buffer[0] & DATA_BITS_MASK) >> 1;
    second_symbol_code = (filter_buff[0].buffer[1] & DATA_BITS_MASK) >> 1;
    control_byte_first = filter_buff[0].buffer[2];
    control_byte_second = filter_buff[0].buffer[3];

    direction_code = control_byte_second & ARROW_MASK;

    // if (!is_fire_danger_symbol) {

#if 0
    switch (first_symbol_code) {

    // Этаж 0
    case 0:
      matrix_string[MSB] = convert_int_to_char(second_symbol_code);
      matrix_string[LSB] = 'c';
      break;

    // Этажи 1..9
    case SYMBOL_EMPTY:
      matrix_string[MSB] = convert_int_to_char(second_symbol_code);
      matrix_string[LSB] = 'c';

      // Этаж cП
      if (second_symbol_code == SYMBOL_UNDERGROUND_FLOOR_BIG) {
        matrix_string[MSB] = 'p';
        matrix_string[LSB] = 'c';
      }
      break;

      // Этажи -1..-9
    case SYMBOL_MINUS:
      matrix_string[MSB] = '-';
      matrix_string[LSB] = convert_int_to_char(second_symbol_code);
      break;

      // Этажи П1..П9
    case SYMBOL_UNDERGROUND_FLOOR_BIG:
      matrix_string[MSB] = 'p';
      matrix_string[LSB] = convert_int_to_char(second_symbol_code);
      break;

      // Этажи с 10
    default:
      matrix_string[MSB] = convert_int_to_char(first_symbol_code);
      matrix_string[LSB] = convert_int_to_char(second_symbol_code);
      break;
    }
    // }
#endif

    // Настройка кода стрелки
    symbols.symbol_code_1 = direction_to_symbol(direction_code);
    // transform_direction_to_common(direction_code);
    // set_direction_symbol(matrix_string, drawing_data.direction);

    // Настройка кода этажа
    // Этаж 0
    if (first_symbol_code == 0 && second_symbol_code == 0) {
      symbols.symbol_code_2 = map_to_common_symbol(second_symbol_code);
      symbols.symbol_code_3 = map_to_common_symbol(NKU_SYMBOL_EMPTY);
    } else if (first_symbol_code == NKU_SYMBOL_EMPTY) {

      symbols.symbol_code_2 = map_to_common_symbol(second_symbol_code);
      symbols.symbol_code_3 = map_to_common_symbol(NKU_SYMBOL_EMPTY);

      // Этаж cП
      if (second_symbol_code == SYMBOL_UNDERGROUND_FLOOR_BIG) {
        symbols.symbol_code_2 = map_to_common_symbol(second_symbol_code);
        symbols.symbol_code_3 = map_to_common_symbol(NKU_SYMBOL_EMPTY);
      }

    } else {
      symbols.symbol_code_2 = map_to_common_symbol(first_symbol_code);
      symbols.symbol_code_3 = map_to_common_symbol(second_symbol_code);
    }

    // Кабинный индикатор
    if (matrix_settings.addr_id == MAIN_CABIN_ID) {
      // Спец. режимы для кабинного индикатора
      cabin_indicator_special_regime(direction_code, control_byte_first,
                                     control_byte_second);
    } else {
      // Этажный индикатор
      /* Установка drawing_data.floor для гонга */
      // Этажи 0, с 1 по 9
      if (first_symbol_code == 0 || first_symbol_code == SYMBOL_EMPTY) {
        drawing_data.floor = second_symbol_code;
      }

      // Этажи с 10 по 99
      if (first_symbol_code >= 1 && first_symbol_code <= 9 &&
          second_symbol_code >= 0 && second_symbol_code <= 9) {
        drawing_data.floor = first_symbol_code * 10 + second_symbol_code;
      }

      // Спец. режимы для этажного индикатора
      floor_indicator_special_regime(direction_code, control_byte_first,
                                     control_byte_second);
    }
  }

  while (is_read_data_completed == false && is_interface_connected == true) {
    // draw_string_on_matrix(matrix_string);
    // draw_symbols(SYM_EMPTY, SYM_EMPTY, second_symbol_code);
    draw_symbols(&symbols);
  }
}

void process_data_pin() {
  if (is_read_data_completed) {
    is_read_data_completed = false;

    process_data_nku_sd7();
  }
}

/// Flag to control is start bit is received (state DATA_Pin from 1 to 0)
volatile bool is_start_bit_received = false;

/// Buffer with timings for reading data bits
const uint16_t nku_sd7_timings[1] = {2556};

/// Value of the current received bit
volatile uint8_t bit = 1;

/// Number of data bits in received packet
static const uint8_t packet_size = 8;

/// Maximum index of received data
static const uint8_t max_index_packet = packet_size - 1;

volatile uint8_t current_byte = 0;

volatile uint8_t bit_index = 0;
volatile uint8_t byte_count = 0;

static void reset_state() {
  bit_index = 0;
  byte_count = 0;
  current_byte = 0;
  is_start_bit_received = false;
  memset((void *)byte_buf, 0, 4 * sizeof(uint8_t));
  TIM3_Stop();
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

// reset_state()
void stop_sd7_before_menu_mode() {
  bit_index = 0;
  byte_count = 0;
  current_byte = 0;
  is_start_bit_received = false;
  memset((void *)byte_buf, 0, 4 * sizeof(uint8_t));
  TIM3_Stop();
}

void read_data_bit(void) {

  uint8_t bit = HAL_GPIO_ReadPin(DATA_GPIO_Port, DATA_Pin);

  current_byte |= (bit << (7 - bit_index));
  bit_index++;

  if (bit_index == 8) {

    // Приняли 8 бит (start + 6 data + stop)
    byte_buf[byte_count] = ~current_byte;
    byte_count++;

    // Проверка старт и стоп битов
    uint8_t start_bit = (~current_byte >> 7) & 0x01;
    uint8_t stop_bit = ~current_byte & 0x01;
    if (start_bit != 1 || stop_bit != 0) {
      // Ошибка байта
      reset_state();
      return;
    }

    if (byte_count == 4) {
      is_read_data_completed = true;
      memcpy(byte_buf_copy, byte_buf, sizeof(byte_buf));
      memset((void *)byte_buf, 0, 4 * sizeof(uint8_t));
      reset_state();

      // Проверка подключения интерфейса
      alive_cnt[0] = (alive_cnt[0] < UINT32_MAX) ? alive_cnt[0] + 1 : 0;
      is_interface_connected = true;

      return;
    }

    // следующий байт
    current_byte = 0;
    bit_index = 0;
  }

  __HAL_TIM_SET_AUTORELOAD(&htim3, nku_sd7_timings[0]);
}
