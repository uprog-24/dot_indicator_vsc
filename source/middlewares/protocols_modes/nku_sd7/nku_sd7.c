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
#define MOVING_MASK 0x60 ///< Маска для направления движения

// Маски для control_byte_first
#define LOADING_MASK 0x02
#define FIRE_DANGER_MASK 0x04
#define CABIN_OVERLOAD_MASK 0x08
#define GONG_MASK 0x10
#define MOVE_DOWN_MASK 0x20
#define MOVE_UP_MASK 0x40

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

#define MESSAGE_BYTES 4    ///< Длина сообщения в байтах
#define FILTER_BUFF_SIZE 5 ///< Ширина фильтра (5 сообщений)

static uint8_t byte_buf[MESSAGE_BYTES] = {0, 0, 0,
                                          0}; ///< Буфер для полученных данных
static uint8_t byte_buf_copy[MESSAGE_BYTES] = {
    0, 0, 0, 0}; ///< Буфер для полученных данных

/**
 * Направления движения.
 */
typedef enum {
  NKU_SD7_MOVE_UP = 64,
  NKU_SD7_MOVE_DOWN = 32,
  NKU_SD7_NO_MOVE = 0
} direction_nku_sd7_t;

typedef enum {
  NKU_SD7_MOVING_UP = 64,
  NKU_SD7_MOVING_DOWN = 32,
  NKU_SD7_NO_MOVING = 0
} moving_nku_sd7_t;

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
  NKU_SYMBOL_b_BIG_RU = 27,                // Символ Б
  NKU_SYMBOL_G_RU = 28,                    // Символ Г
  NKU_SYMBOL_R = 29,                       // Символ R
  NKU_SYMBOL_V = 30,                       // Символ V
  NKU_SYMBOL_N = 31,                       // Символ N
  NKU_SYMBOL_S = 32,                       // Символ S
  NKU_SYMBOL_K = 33,                       // Символ K
  NKU_SYMBOL_Y = 34,                       // Символ Y
  NKU_SYMBOL_G = 35,                       // Символ G
  NKU_SYMBOL_B = 36,                       // Символ B
  NKU_SYMBOL_T = 37,                       // Символ T
  NKU_SYMBOL_NUMBER
} nku_symbols_t;

/// Структура с данными для отображения (direction, floor).
static drawing_data_t drawing_data = {0, 0};

symbol_code_e dir_sym;
/**
 * @brief Маппинг направления движения в код symbol_code_e
 *
 * @param direction
 * @return symbol_code_e
 */
static inline symbol_code_e
map_direction_to_common_symbol(moving_nku_sd7_t moving_code,
                               direction_nku_sd7_t direction) {

#if 1
  switch (moving_code) {
  case NKU_SD7_MOVING_UP:
    dir_sym = SYMBOL_ARROW_UP_ANIMATION;
    break;

  case NKU_SD7_MOVING_DOWN:
    dir_sym = SYMBOL_ARROW_DOWN_ANIMATION;
    break;

  case NKU_SD7_NO_MOVING:
    if (direction == NKU_SD7_MOVE_UP) {
      dir_sym = SYMBOL_ARROW_UP;
    } else if (direction == NKU_SD7_MOVE_DOWN) {
      dir_sym = SYMBOL_ARROW_DOWN;
    } else {
      dir_sym = SYMBOL_EMPTY;
    }
    break;
  }

  return dir_sym;
#else
  switch (direction) {
  case NKU_SD7_MOVE_UP:
    // return SYMBOL_ARROW_UP;
    return SYMBOL_ARROW_UP_ANIMATION;
  case NKU_SD7_MOVE_DOWN:
    // return SYMBOL_ARROW_DOWN;
    return SYMBOL_ARROW_DOWN_ANIMATION;

  case NKU_SD7_NO_MOVE:
    return SYMBOL_EMPTY;
  default:
    return SYMBOL_EMPTY;
  }
#endif
}

// Маппинг символов nku_symbols_t в код symbol_code_e
static const symbol_code_e nku_symbol_to_common_table[NKU_SYMBOL_NUMBER] = {
    [NKU_SYMBOL_0] = SYMBOL_0,
    [NKU_SYMBOL_1] = SYMBOL_1,
    [NKU_SYMBOL_2] = SYMBOL_2,
    [NKU_SYMBOL_3] = SYMBOL_3,
    [NKU_SYMBOL_4] = SYMBOL_4,
    [NKU_SYMBOL_5] = SYMBOL_5,
    [NKU_SYMBOL_6] = SYMBOL_6,
    [NKU_SYMBOL_7] = SYMBOL_7,
    [NKU_SYMBOL_8] = SYMBOL_8,
    [NKU_SYMBOL_9] = SYMBOL_9,
    [NKU_SYMBOL_A] = SYMBOL_A,         // Символ A
    [NKU_SYMBOL_b] = SYMBOL_b,         // Символ b
    [NKU_SYMBOL_C] = SYMBOL_C,         // Символ C
    [NKU_SYMBOL_d] = SYMBOL_d,         // Символ d
    [NKU_SYMBOL_E] = SYMBOL_E,         // Символ E
    [NKU_SYMBOL_F] = SYMBOL_F,         // Символ F
    [NKU_SYMBOL_EMPTY] = SYMBOL_EMPTY, // Символ Пусто
    [NKU_SYMBOL_UNDERGROUND_FLOOR_BIG] =
        SYMBOL_UNDERGROUND_FLOOR_BIG, // Символ П
    [NKU_SYMBOL_P] = SYMBOL_P,        // Символ P
    [NKU_SYMBOL_UNDERGROUND_FLOOR_SMALL] =
        SYMBOL_UNDERGROUND_FLOOR_SMALL,          // Символ п
    [NKU_SYMBOL_H] = SYMBOL_H,                   // Символ H
    [NKU_SYMBOL_U_BIG] = SYMBOL_U_BIG,           // Символ U
    [NKU_SYMBOL_MINUS] = SYMBOL_MINUS,           // Символ -
    [NKU_SYMBOL_UNDERSCORE] = SYMBOL_UNDERSCORE, // Символ _
    [NKU_SYMBOL_U_SMALL] = SYMBOL_U_SMALL,       // Символ u
    [NKU_SYMBOL_L] = SYMBOL_L,                   // Символ L
    [NKU_SYMBOL_Y_RU] = SYMBOL_Y_RU,             // Символ У
    [NKU_SYMBOL_b_BIG_RU] = SYMBOL_b_BIG_RU,     // Символ Б
    [NKU_SYMBOL_G_RU] = SYMBOL_G_RU,             // Символ Г
    [NKU_SYMBOL_R] = SYMBOL_R,                   // Символ R
    [NKU_SYMBOL_V] = SYMBOL_V,                   // Символ V
    [NKU_SYMBOL_N] = SYMBOL_N,                   // Символ N
    [NKU_SYMBOL_S] = SYMBOL_S,                   // Символ S
    [NKU_SYMBOL_K] = SYMBOL_K,                   // Символ K
    [NKU_SYMBOL_Y] = SYMBOL_Y,                   // Символ Y
    [NKU_SYMBOL_G] = SYMBOL_G,                   // Символ G
    [NKU_SYMBOL_B] = SYMBOL_B,                   // Символ B
    [NKU_SYMBOL_T] = SYMBOL_T,                   // Символ T
};

/**
 * @brief Маппинг символов nku_symbols_t в код symbol_code_e с применением
 *        таблицы nku_symbol_to_common_table
 *
 * @param symbol_code
 * @return symbol_code_e
 */
static inline symbol_code_e map_to_common_symbol(uint8_t symbol_code) {
  if (symbol_code < NKU_SYMBOL_NUMBER) {
    return nku_symbol_to_common_table[symbol_code];
  } else {
    return SYMBOL_EMPTY;
  }
}

/**
 * @brief Режим Погрузка (символы)
 *
 * @param control_byte_first
 */
static void set_loading_symbol(uint8_t control_byte_first) {
  if ((control_byte_first & LOADING_MASK) == LOADING_MASK) {
    set_symbols(map_to_common_symbol(NKU_SYMBOL_EMPTY),
                map_to_common_symbol(NKU_SYMBOL_UNDERGROUND_FLOOR_BIG),
                map_to_common_symbol(NKU_SYMBOL_G_RU));
  }
}

/// Флаг для контроля перегруза кабины
bool is_cabin_overload = false;

/// Состояние звука (вкл/выкл)
bool is_overload_sound_on = false;

extern uint16_t overload_sound_ms;
extern volatile bool is_time_ms_for_overload_elapsed;

/**
 * @brief Режим Перегрузка (символы и звук)
 *
 * @param control_byte_first
 */
static void set_cabin_overload_symbol_sound(uint8_t control_byte_first) {

  if ((control_byte_first & CABIN_OVERLOAD_MASK) == CABIN_OVERLOAD_MASK) {

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

    set_symbols(map_to_common_symbol(NKU_SYMBOL_EMPTY),
                map_to_common_symbol(NKU_SYMBOL_K),
                map_to_common_symbol(NKU_SYMBOL_G_RU));
  } else if (is_cabin_overload) {
    stop_buzzer_sound();
    is_cabin_overload = false;
    is_overload_sound_on = false;
    overload_sound_ms = 0;
  }
}

/**
 * @brief Режим Авария (символы)
 *
 * @param control_byte_second
 */
static void set_accident_symbol(uint8_t control_byte_second) {
  if ((control_byte_second & ACCIDENT_MASK) == ACCIDENT_MASK) {
    set_floor_symbols(map_to_common_symbol(NKU_SYMBOL_A),
                      map_to_common_symbol(NKU_SYMBOL_EMPTY));
  }
}

static bool is_fire_danger_symbol = false;

/**
 * @brief Режим Пожар (символы)
 *
 * @param control_byte_first
 */
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

/**
 * @brief Режим Пожар (звук)
 *
 * @param control_byte_second
 */
static void set_fire_danger_sound(uint8_t control_byte_second) {
  uint8_t fire_danger_sound_bit = control_byte_second & FIRE_DANGER_SOUND_MASK;

  fire_sound_edge[0] =
      (fire_danger_sound_bit == FIRE_DANGER_SOUND_MASK) != 0 ? 1 : 0;

  if (fire_sound_edge[0] && !fire_sound_edge[1]) {
    stop_buzzer_sound();
    play_gong(1, BUZZER_FREQ_FIRE_DANGER, VOLUME_3);
  }
  fire_sound_edge[1] = fire_sound_edge[0];
}

static uint8_t gong[2] = {
    0,
};

/**
 * @brief Гонг до прибытия на этаж
 *
 * @param direction_code
 * @param control_byte_first
 * @param volume
 */
static void setting_gong(uint8_t direction_code, uint8_t control_byte_first,
                         uint8_t volume) {
  uint8_t arrival = control_byte_first & GONG_MASK;

  /*  Если сигнал Гонг из 0 меняется на 1 и быты Движение Вверх и Вниз равны 0,
   * тогда детектируем прибытие на этаж */
  gong[0] = ((arrival == GONG_MASK) == 1) ? 1 : 0;

  if (gong[0] && !gong[1]) {

    switch (direction_code) {
    case NKU_SD7_MOVE_UP:
      play_gong(1, GONG_BUZZER_FREQ, volume);
      break;
    case NKU_SD7_MOVE_DOWN:
      play_gong(1, GONG_BUZZER_FREQ, volume);
      break;
    case NKU_SD7_NO_MOVE:
      break;
    default:
      break;
    }
  }
  gong[1] = gong[0];
}

static uint8_t gong_stop_floor[2] = {
    0,
};

/**
 * @brief Гонг по прибытии на этаж
 *
 * @param direction_code
 * @param control_byte_first
 * @param volume
 */
static void setting_gong_stop(uint8_t direction_code,
                              uint8_t control_byte_first, uint8_t volume) {
  uint8_t arrival = control_byte_first & GONG_MASK;

  bool is_stop = ((control_byte_first & MOVE_DOWN_MASK) == 0) &&
                 ((control_byte_first & MOVE_UP_MASK) == 0);

  gong_stop_floor[0] = (((arrival == GONG_MASK) != 0) && is_stop) ? 1 : 0;

  if (gong_stop_floor[0] && !gong_stop_floor[1]) {
    play_gong(3, GONG_BUZZER_FREQ, volume);
  }
  gong_stop_floor[1] = gong_stop_floor[0];
}

/// Флаг контроля 4-х байтов
volatile bool is_read_data_completed = false;

/**
 * @brief Обработка спец. режимов для кабинного индикатора
 *
 * @param direction_code
 * @param control_byte_first
 * @param control_byte_second
 */
static void cabin_indicator_special_regime(uint8_t direction_code,
                                           uint8_t control_byte_first,
                                           uint8_t control_byte_second) {

  // Гонг
  if (matrix_settings.volume != VOLUME_0) {
    setting_gong(direction_code, control_byte_first, matrix_settings.volume);
    setting_gong_stop(direction_code, control_byte_first,
                      matrix_settings.volume);
  }

  // Погрузка
  set_loading_symbol(control_byte_first);

  // Перегруз
  set_cabin_overload_symbol_sound(control_byte_first);

  // Авария
  set_accident_symbol(control_byte_second);

  // Пожар, символ F
  set_fire_danger_symbol(control_byte_first);

  // Пожар, звук по фронту (на этаже старта и на 1-ом этаже)
  if (matrix_settings.volume != VOLUME_0) {
    set_fire_danger_sound(control_byte_second);
  }
}

/**
 * @brief Обработка спец. режимов для этажного индикатора
 *
 * @param direction_code
 * @param control_byte_first
 * @param control_byte_second
 */
static void floor_indicator_special_regime(uint8_t direction_code,
                                           uint8_t control_byte_first,
                                           uint8_t control_byte_second) {

  // Гонг, прибытие на этаж с номером matrix_settings.addr_id
  if (matrix_settings.addr_id == drawing_data.floor) {
    if (matrix_settings.volume != VOLUME_0) {
      setting_gong_stop(direction_code, control_byte_first,
                        matrix_settings.volume);
    }
  }

  // Авария
  set_accident_symbol(control_byte_second);

  // Пожар, символ F
  set_fire_danger_symbol(control_byte_first);
}

/// Флаг для контроля фильтрации данных
static bool is_data_filtered = false;

/// Кол-во полученных данных
static uint8_t number_received_data = 0;

/// Текущий индекс элемента в filter_buff
static uint8_t current_index_buff = 0;

typedef struct {
  uint8_t buffer[MESSAGE_BYTES]; // 4 байта полученных данных
  uint8_t counter;               // счетчик повторений
} floor_counter_t;

/// Буфер полученных данных с повторениями
static floor_counter_t filter_buff[FILTER_BUFF_SIZE];

/**
 * @brief  Запонение структуры floor_counter_t
 * @param  filter_struct: Указатель на структуру floor_counter_t
 * @param  buffer:        Указатель на буфер с полученными данными
 * @param  counter:       Кол-во повторений данных
 */
static void set_filter_structure(floor_counter_t *filter_struct,
                                 uint8_t *buffer, uint8_t counter) {
  memcpy(filter_struct->buffer, buffer, sizeof(filter_struct->buffer));
  // memcpy(filter_struct->buffer, buffer, MESSAGE_BYTES * sizeof(uint8_t));
  filter_struct->counter = counter;
}

/**
 * @brief  Сортировка filter_buff в порядке убывания с использованием метода
 *         пузырьковой сортировки
 * @note   filter_buff[0].counter имеет максимальное значение и будет
 *         отображаться на индикаторе
 * @param  filter_buff
 * @param  buff_size
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

/**
 * @brief Фильтрация полученных данных
 *
 */
static void filter_data() {
  if (current_index_buff == 0) {
    for (uint8_t i = 0; i < FILTER_BUFF_SIZE; i++) {
      set_filter_structure(&filter_buff[i], zero_buffer, 0);
    }
    current_index_buff = 0;
  }

  bool is_data_found = false;
  number_received_data++;

  for (uint8_t i = 0; i < current_index_buff; i++) {
    if (memcmp(filter_buff[i].buffer, byte_buf_copy, MESSAGE_BYTES) == 0) {
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
} nku_sd7_msg_t;

static nku_sd7_msg_t nku_sd7_msg = {
    0x00,
};

static uint8_t direction_code;
static uint8_t moving_code;

/**
 * @brief  Обработка данных по протоколу НКУ-SD7
 * @note   Фильтрация, воспроизведение гонгов и отображение символов
 */
void process_data_nku_sd7() {

  filter_data();

  if (is_data_filtered) {
    is_data_filtered = false;

    nku_sd7_msg.first_symbol_code =
        (filter_buff[0].buffer[0] & DATA_BITS_MASK) >> 1;
    nku_sd7_msg.second_symbol_code =
        (filter_buff[0].buffer[1] & DATA_BITS_MASK) >> 1;
    nku_sd7_msg.control_byte_first = filter_buff[0].buffer[2];
    nku_sd7_msg.control_byte_second = filter_buff[0].buffer[3];

    direction_code = nku_sd7_msg.control_byte_second & ARROW_MASK;
    moving_code = nku_sd7_msg.control_byte_first & MOVING_MASK;

    // Настройка кода стрелки
    set_direction_symbol(
        map_direction_to_common_symbol(moving_code, direction_code));

    // Настройка кода этажа
    // Этаж 0
    bool is_zero_floor = (nku_sd7_msg.first_symbol_code == 0 &&
                          nku_sd7_msg.second_symbol_code == 0);
    // Этаж 1..9
    bool is_first_symbol_empty =
        (nku_sd7_msg.first_symbol_code == NKU_SYMBOL_EMPTY);
    // Этаж cП
    bool is_floor_underground_p =
        (is_first_symbol_empty &&
         nku_sd7_msg.second_symbol_code == SYMBOL_UNDERGROUND_FLOOR_BIG);

    if (is_zero_floor || is_first_symbol_empty || is_floor_underground_p) {
      set_floor_symbols(map_to_common_symbol(nku_sd7_msg.second_symbol_code),
                        map_to_common_symbol(NKU_SYMBOL_EMPTY));
    } else {
      // Этажи с 10, спец. символы
      set_floor_symbols(map_to_common_symbol(nku_sd7_msg.first_symbol_code),
                        map_to_common_symbol(nku_sd7_msg.second_symbol_code));
    }

    // Спец. режимы и звуковые оповещения
    // Кабинный индикатор
    if (matrix_settings.addr_id == MAIN_CABIN_ID) {
      // Спец. режимы для кабинного индикатора
      cabin_indicator_special_regime(direction_code,
                                     nku_sd7_msg.control_byte_first,
                                     nku_sd7_msg.control_byte_second);
    } else {
      // Этажный индикатор
      /* Установка drawing_data.floor для гонга */
      // Этажи 0, с 1 по 9
      if (nku_sd7_msg.first_symbol_code == 0 ||
          nku_sd7_msg.first_symbol_code == NKU_SYMBOL_EMPTY) {
        drawing_data.floor = nku_sd7_msg.second_symbol_code;
      }

      // Этажи с 10 по 99
      if ((nku_sd7_msg.first_symbol_code >= 1 &&
           nku_sd7_msg.first_symbol_code <= 9) &&
          (nku_sd7_msg.second_symbol_code >= 0 &&
           nku_sd7_msg.second_symbol_code <= 9)) {
        drawing_data.floor =
            nku_sd7_msg.first_symbol_code * 10 + nku_sd7_msg.second_symbol_code;
      }

      // Спец. режимы для этажного индикатора
      floor_indicator_special_regime(direction_code,
                                     nku_sd7_msg.control_byte_first,
                                     nku_sd7_msg.control_byte_second);
    }
  }

  // Отображение полученных данных на индикаторе
  while (is_read_data_completed == false && is_interface_connected == true) {
    draw_symbols();
  }
}

/**
 * @brief Если данные получены, то обработать их
 *
 */
void process_data_pin() {
  if (is_read_data_completed) {
    is_read_data_completed = false;

    process_data_nku_sd7();
  }
}

/// Флаг для контроля старт-бита (DATA_Pin из 1 в 0)
volatile bool is_start_bit_received = false;

/// Тайминг для чтения бита
const uint16_t nku_sd7_timing = 2556;

/// Значение полученного бита
volatile uint8_t bit = 1;

volatile uint8_t current_byte = 0;
volatile uint8_t bit_index = 0;
volatile uint8_t byte_count = 0;

/**
 * @brief Сброс состояния для чтения очередного сообщения
 *
 */
static void reset_state() {
  bit_index = 0;
  byte_count = 0;
  current_byte = 0;
  is_start_bit_received = false;
  memset((void *)byte_buf, 0, 4 * sizeof(uint8_t));
  TIM3_Stop();
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
 * @brief Сброс состояния перед переходом в режим меню
 *
 */
void stop_sd7_before_menu_mode() {
  bit_index = 0;
  byte_count = 0;
  current_byte = 0;
  is_start_bit_received = false;
  memset((void *)byte_buf, 0, 4 * sizeof(uint8_t));
  TIM3_Stop();
}

/**
 * @brief Чтение бита, получение 4-х байтов по протоколу НКУ-SD7
 *
 */
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

  __HAL_TIM_SET_AUTORELOAD(&htim3, nku_sd7_timing);
}
