/**
 * @file uim6100.c
 */
#include "nku_sd7.h"

#include "buzzer.h"
#include "config.h"
#include "drawing.h"
#include "tim.h"

#include <stdbool.h>

// #define SYMBOL_FLOOR_MASK 0x3F
// #define SYMBOL_FLOOR_MASK 0x7E
#if 0
#define SYMBOL_FLOOR_MASK 0x3F
#define ARROW_MASK 0x30 // 0x60 ///< Маска для направления движения
#endif

#if 1
#define SYMBOL_FLOOR_MASK 0x7E
#define ARROW_MASK 0x60 ///< Маска для направления движения

#endif

#define ARRIVAL_MASK 0b100 ///< Маска для бита прибытия (2-й бит байта W3)

#define GONG_BUZZER_FREQ 1000 ///< Частота первого тона гонга
#define BUZZER_FREQ_CABIN_OVERLOAD                                             \
  3000 ///< Частота для тона бузера при перегрузе кабины VOICE_CABIN_OVERLOAD
#define BUZZER_FREQ_FIRE_DANGER                                                \
  BUZZER_FREQ_CABIN_OVERLOAD ///< Частота для тона бузера при пожарной опасности
                             ///< VOICE_FIRE_DANGER

#define SPECIAL_SYMBOLS_BUFF_SIZE 19 ///< Кол-во спец. символов

#define MESSAGE_BYTES 4 ///< Длина сообщения в байтах

uint8_t byte_buf[4] = {0, 0, 0, 0}; ///< Буфер для полученных данных
uint8_t byte_buf_copy[4] = {0, 0, 0, 0}; ///< Буфер для полученных данных

/*
 * Структура для сохранения полученных байтов по последовательному интерфейсу.
 */
typedef struct {
  uint8_t byte_0;
  uint8_t byte_1;
  uint8_t byte_2;
  uint8_t byte_3;
} msg_t;

/**
 * Направления движения.
 */
typedef enum {
#if 0
  NKU_SD7_MOVE_UP = 32,   // 64,
  NKU_SD7_MOVE_DOWN = 16, // 32,
#endif
  NKU_SD7_MOVE_UP = 64,
  NKU_SD7_MOVE_DOWN = 32,
  NKU_SD7_NO_MOVE = 0
} direction_nku_sd7_t;

typedef enum SYMBOLS {
  SYMBOL_0 = 0,
  SYMBOL_1 = 1,
  SYMBOL_2 = 2,
  SYMBOL_3 = 3,
  SYMBOL_4 = 4,
  SYMBOL_5 = 5,
  SYMBOL_6 = 6,
  SYMBOL_7 = 7,
  SYMBOL_8 = 8,
  SYMBOL_9 = 9,
  SYMBOL_A = 10,                       // Символ A
  SYMBOL_b = 11,                       // Символ b
  SYMBOL_C = 12,                       // Символ C
  SYMBOL_d = 13,                       // Символ d
  SYMBOL_E = 14,                       // Символ E
  SYMBOL_F = 15,                       // Символ F
  SYMBOL_EMPTY = 16,                   // Символ Пробел
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

typedef struct {
  uint16_t code_location;
  char symbol_char;
} code_location_char_t;

static const code_location_char_t char_code_location[10] = {
    {.code_location = SYMBOL_0, .symbol_char = '0'},
    {.code_location = SYMBOL_1, .symbol_char = '1'},
    {.code_location = SYMBOL_2, .symbol_char = '2'},
    {.code_location = SYMBOL_3, .symbol_char = '3'},
    {.code_location = SYMBOL_4, .symbol_char = '4'},
    {.code_location = SYMBOL_5, .symbol_char = '5'},
    {.code_location = SYMBOL_6, .symbol_char = '6'},
    {.code_location = SYMBOL_7, .symbol_char = '7'},
    {.code_location = SYMBOL_8, .symbol_char = '8'},
    {.code_location = SYMBOL_9, .symbol_char = '9'}};

/// Буфер со спец. символами
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

/// Flag to control is data completed
volatile bool is_read_data_completed = false;

// uint8_t reverse_byte(uint8_t n) {
//   uint8_t result = 0;
//   for (int i = 0; i < 8; i++) {
//     result <<= 1;      // Сдвигаем result влево
//     result |= (n & 1); // Берём младший бит из n
//     n >>= 1;           // Сдвигаем n вправо
//   }
//   return result;
// }

/**
 * @brief  Обработка данных по протоколу NKU_SD7.
 * @param
 * @retval None
 */
void process_data_nku_sd7() {

  drawing_data.floor = byte_buf_copy[1] & SYMBOL_FLOOR_MASK;

  // transform_direction_to_common(byte_buf_copy[3] & ARROW_MASK);

  setting_symbols(matrix_string, &drawing_data, MAX_POSITIVE_NUMBER_FLOOR,
                  special_symbols_code_location, SPECIAL_SYMBOLS_BUFF_SIZE);

  while (is_read_data_completed == false && is_interface_connected == true) {
    draw_string_on_matrix(matrix_string);
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

uint32_t received_bit_time[32] = {
    0,
};

/// Value of the current received bit
volatile uint8_t bit = 1;

/// Number of data bits in received packet
static const uint8_t packet_size = 8;

/// Maximum index of received data
static const uint8_t max_index_packet = packet_size - 1;

// volatile uint8_t current_byte = 0;
volatile uint32_t current_byte = 0;
volatile uint32_t current_byte_copy = 0;

volatile uint8_t bit_index = 0;
volatile uint8_t byte_count = 0;

extern uint16_t tim4_ms_counter;
extern volatile bool is_time_start;

extern uint32_t tim3_ms_counter;
extern volatile uint8_t bit_message_number;

void reset_state() {
  bit_index = 0;
  byte_count = 0;
  current_byte = 0;
  is_start_bit_received = false;
  TIM3_Stop();
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

#if 1
void read_data_bit() {

  bit = HAL_GPIO_ReadPin(DATA_GPIO_Port, DATA_Pin);

  if (bit_index < 32) {
    if (bit_index == 0) {
      is_start_bit_received = true;
      current_byte |= bit << (31 - bit_index);
      bit_index++;
      TIM3_Start(PRESCALER_FOR_US, nku_sd7_timings[0]);
    } else {
      current_byte |= (bit) << (31 - bit_index);
      bit_index++;
    }
  } else {
    is_read_data_completed = true;
    current_byte_copy = ~current_byte;
    for (int i = 0; i < 4; i++) {
      byte_buf_copy[i] = (current_byte_copy >> (8 * i)) & 0xFF;
    }
    reset_state();
  }

  //   if (bit_index < 7) {
  //     current_byte |= (bit) << (7 - bit_index);
  //     bit_index++;
  //   } else if (bit_index == 7) {
  // // стоп-бит
  // #if 1
  //     if (bit != 1) {
  //       // Ошибка, невалидный пакет
  //       reset_state();
  //     } else {
  //       current_byte |= (bit) << (7 - bit_index);
  //       byte_buf[byte_count] = ~current_byte;
  //       byte_count++;
  //     }
  // #endif

  //     if (byte_count == 4) {

  //       is_read_data_completed = true;
  //       memcpy(byte_buf_copy, byte_buf, sizeof(byte_buf));

  //       reset_state(); // Переход к прерывнию по спаду фронта
  //     }
  //   }
}
#endif

#if 0
void read_data_bit() {

  bit = HAL_GPIO_ReadPin(DATA_GPIO_Port, DATA_Pin);

  if (bit_index == 0) {
    // current_byte = 0;
    if (bit == 0) {
      is_start_bit_received = true;
      current_byte |= bit << (7 - bit_index);
      bit_index++;
      TIM3_Start(PRESCALER_FOR_US, nku_sd7_timings[0]);
    } else {
      reset_state(); // Переход к прерывнию по спаду фронта
    }
    return;
  }

  if (bit_index < 7) {
    current_byte |= (bit) << (7 - bit_index);
    bit_index++;
  } else if (bit_index == 7) {
// стоп-бит
#if 1
    if (bit != 1) {
      // Ошибка, невалидный пакет
      reset_state();
    } else {
      current_byte |= (bit) << (7 - bit_index);
      byte_buf[byte_count] = ~current_byte;
      byte_count++;
    }
#endif

    if (byte_count == 4) {

      is_read_data_completed = true;
      memcpy(byte_buf_copy, byte_buf, sizeof(byte_buf));

      reset_state(); // Переход к прерывнию по спаду фронта
    }

    bit_index = 0; // Для следующего байта
    current_byte = 0;

    // is_start_bit_received = false;
    // TIM3_Stop();
    // HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  }
}
#endif

#if 0
void read_data_bit() {
  uint8_t bit = HAL_GPIO_ReadPin(DATA_GPIO_Port, DATA_Pin);

  if (bit_index >= 1 && bit_index <= 6) {     // 6 data-битов
    current_byte |= (bit << (6 - bit_index)); // LSB first
    bit_index++;
    TIM3_Start(PRESCALER_FOR_US, nku_sd7_timings[0]); // Следующий бит
    return;
  }

  if (bit_index == 7) { // Стоп-бит
    if (bit == 1) {
      // Стоп-бит корректен

      byte_buf[byte_count] = ~(current_byte & 0x3F); // Только 6 бит
      byte_count++;

      if (byte_count >= 4) {
        is_read_data_completed = true;
        memcpy(byte_buf_copy, byte_buf, sizeof(byte_buf));
        byte_count = 0;
        tim4_ms_counter = 0;
      }

      //  ожидания старт bit
      bit_index = 0;
      current_byte = 0;
      is_start_bit_received = false;
      HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
      TIM3_Stop();
    } else {
      // Ошибка стоп-бита
      bit_index = 0;
      current_byte = 0;
      byte_count = 0;
      is_start_bit_received = false;
      HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
      TIM3_Stop();
    }
  }
}
#endif