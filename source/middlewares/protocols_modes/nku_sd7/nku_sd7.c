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

/// Структура с данными для отображения (direction, floor).
static drawing_data_t drawing_data = {0, 0};

static inline symbol_e
map_direction_to_common_symbol(direction_nku_sd7_t direction) {
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
    return SYMBOL_0;
  case NKU_SYMBOL_1:
    return SYMBOL_1;
  case NKU_SYMBOL_2:
    return SYMBOL_2;
  case NKU_SYMBOL_3:
    return SYMBOL_3;
  case NKU_SYMBOL_4:
    return SYMBOL_4;
  case NKU_SYMBOL_5:
    return SYMBOL_5;
  case NKU_SYMBOL_6:
    return SYMBOL_6;
  case NKU_SYMBOL_7:
    return SYMBOL_7;
  case NKU_SYMBOL_8:
    return SYMBOL_8;
  case NKU_SYMBOL_9:
    return SYMBOL_9;

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

  case NKU_SYMBOL_UNDERGROUND_FLOOR_BIG: // Символ П
    return SYMBOL_UNDERGROUND_FLOOR_BIG;
  case NKU_SYMBOL_P: // Символ P
    return SYMBOL_P;

  case NKU_SYMBOL_UNDERGROUND_FLOOR_SMALL: // Символ п
    return SYMBOL_UNDERGROUND_FLOOR_SMALL;
  case NKU_SYMBOL_H: // Символ H
    return SYMBOL_H;
  case NKU_SYMBOL_U_BIG: // Символ U
    return SYMBOL_U_BIG;
  case NKU_SYMBOL_MINUS: // Символ -
    return SYMBOL_MINUS;
  case NKU_SYMBOL_UNDERSCORE: // Символ _
    return SYMBOL_UNDERSCORE;
  case NKU_SYMBOL_U_SMALL: // Символ u
    return SYMBOL_U_SMALL;
  case NKU_SYMBOL_L: // Символ L
    return SYMBOL_L;
  case NKU_SYMBOL_Y_RU: // Символ У
    return SYMBOL_Y_RU;
  case NKU_SYMBOL_B_RU: // Символ Б
    return SYMBOL_B_RU;
  case NKU_SYMBOL_G_RU: // Символ Г
    return SYMBOL_G_RU;
  case NKU_SYMBOL_R: // Символ R
    return SYMBOL_R;
  case NKU_SYMBOL_V: // Символ V
    return SYMBOL_V;
  case NKU_SYMBOL_N: // Символ N
    return SYMBOL_N;
  case NKU_SYMBOL_S: // Символ S
    return SYMBOL_S;
  case NKU_SYMBOL_K: // Символ K
    return SYMBOL_K;
  case NKU_SYMBOL_Y: // Символ Y
    return SYMBOL_Y;
  case NKU_SYMBOL_G: // Символ G
    return SYMBOL_G;
  case NKU_SYMBOL_B: // Символ B
    return SYMBOL_B;
  case NKU_SYMBOL_T: // Символ T
    return SYMBOL_T;

  default:
    return SYMBOL_EMPTY;
  }
}

// Режим Погрузка (символы)
static void set_loading_symbol(uint8_t control_byte_first) {
  if ((control_byte_first & LOADING_MASK) == LOADING_MASK) {
    set_symbols(map_to_common_symbol(NKU_SYMBOL_EMPTY),
                map_to_common_symbol(NKU_SYMBOL_UNDERGROUND_FLOOR_BIG),
                map_to_common_symbol(NKU_SYMBOL_G_RU));
  }
}

/// Флаг для контроля перегруза кабины
static bool is_cabin_overload = false;

// Режим Перегрузка (символы и звук)
static void set_cabin_overload_symbol_sound(uint8_t control_byte_first) {
  if ((control_byte_first & CABIN_OVERLOAD_MASK) == CABIN_OVERLOAD_MASK) {

    if (matrix_settings.volume != VOLUME_0) {
      is_cabin_overload = true;
      start_buzzer_sound(BUZZER_FREQ_CABIN_OVERLOAD, VOLUME_3);
    }

    set_symbols(map_to_common_symbol(NKU_SYMBOL_EMPTY),
                map_to_common_symbol(NKU_SYMBOL_K),
                map_to_common_symbol(NKU_SYMBOL_G_RU));
  } else if (is_cabin_overload) {
    stop_buzzer_sound();
    is_cabin_overload = false;
  }
}

// Режим Авария (символы)
static void set_accident_symbol(uint8_t control_byte_second) {
  if ((control_byte_second & ACCIDENT_MASK) == ACCIDENT_MASK) {
    set_floor_symbols(map_to_common_symbol(NKU_SYMBOL_A),
                      map_to_common_symbol(NKU_SYMBOL_EMPTY));
  }
}

static bool is_fire_danger_symbol = false;

// Режим Пожар (символы)
static void set_fire_danger_symbol(uint8_t control_byte_first) {
  if ((control_byte_first & FIRE_DANGER_MASK) == FIRE_DANGER_MASK) {
    is_fire_danger_symbol = true;
    set_floor_symbols(map_to_common_symbol(NKU_SYMBOL_F),
                      map_to_common_symbol(NKU_SYMBOL_EMPTY));
  }
}

static uint8_t fire_sound_edge[2] = {
    0,
};

// Режим Пожар (звук)
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

// Гонг
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

    // Настройка кода стрелки
    set_direction_symbol(map_direction_to_common_symbol(direction_code));

    // Настройка кода этажа
    // Этаж 0
    bool is_zero_floor = (first_symbol_code == 0 && second_symbol_code == 0);
    // Этаж 1..9
    bool is_first_symbol_empty = (first_symbol_code == NKU_SYMBOL_EMPTY);
    // Этаж cП
    bool is_floor_underground_p =
        (is_first_symbol_empty &&
         second_symbol_code == SYMBOL_UNDERGROUND_FLOOR_BIG);

    if (is_zero_floor || is_first_symbol_empty || is_floor_underground_p) {
      set_floor_symbols(map_to_common_symbol(second_symbol_code),
                        map_to_common_symbol(NKU_SYMBOL_EMPTY));
    } else {
      // Этажи с 10, спец. символы
      set_floor_symbols(map_to_common_symbol(first_symbol_code),
                        map_to_common_symbol(second_symbol_code));
    }

#if 0
    if (first_symbol_code == 0 && second_symbol_code == 0) {

      set_symbols_second_third(map_to_common_symbol(second_symbol_code),
                               map_to_common_symbol(NKU_SYMBOL_EMPTY));
    } else if (first_symbol_code == NKU_SYMBOL_EMPTY) {

      set_symbols_second_third(map_to_common_symbol(second_symbol_code),
                               map_to_common_symbol(NKU_SYMBOL_EMPTY));

      // Этаж cП
      if (second_symbol_code == SYMBOL_UNDERGROUND_FLOOR_BIG) {

        set_symbols_second_third(map_to_common_symbol(second_symbol_code),
                                 map_to_common_symbol(NKU_SYMBOL_EMPTY));
      }

    } else {
      set_symbols_second_third(map_to_common_symbol(first_symbol_code),
                               map_to_common_symbol(second_symbol_code));
    }

#endif

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
    draw_symbols();
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
