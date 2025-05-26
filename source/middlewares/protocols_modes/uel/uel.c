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

#define FILTER_BUFF_SIZE 5 ///< Ширина фильтра (5 сообщений)

/**
 * Stores values of control bits
 */
typedef enum CONTROL_BITS_STATES {

  /* Контрольные биты посылки 1 */
  UEL_MOVE_UP = 0x140,
  UEL_MOVE_DOWN = 0x180,
  UEL_NO_MOVE = 0x100,

  NUMBER_CLICKED_BTN_ORDER = 0x1C0, // Контрольные биты посылки 2

  // Контрольные биты посылки 3
  MOVE_UP_AFTER_STOP = 0x40,
  MOVE_DOWN_AFTER_STOP = 0x80,
  MOVE_UP_OR_DOWN_AFTER_STOP = 0xC0,

  // Контрольные биты посылки 4
  SPECIAL_FORMAT = 0x0
} control_bits_states_t;

/**
 * Stores values of code locations
 */
typedef enum CODE_LOCATION_UEL {
  LOCATION_DASH = 0x29,    // 41
  LOCATION_P2 = 0x2C,      // 44
  LOCATION_P1 = 0x2D,      // 45
  LOCATION_P = 0x2E,       // 46
  LOCATION_MINUS_4 = 0x2F, // 47
  LOCATION_MINUS_3 = 0x30, // 48
  LOCATION_MINUS_2 = 0x31, // 49
  LOCATION_MINUS_1 = 0x32, // 50

  LOCATION_C1 = 0x34,          // 52
  LOCATION_C2 = 0x35,          // 53
  LOCATION_REVISION = 0x37,    // 55
  LOCATION_NORMAL_WORK = 0x38, // 56
  LOCATION_LOADING = 0x39,     // 57

  // repeated values
  CABIN_OVERLOAD = 1,
  GONG_ARRIVAL = 8,
  FIRE_DANGER_SYMBOL = 2,
  FIRE_DANGER_SOUND = 32,
  EVACUATION = 4,
  SEISMIC_DANGER = 16
} code_location_uel_t;

typedef enum SYMBOLS {
  UEL_SYMBOL_0 = 0,
  UEL_SYMBOL_1 = 1,
  UEL_SYMBOL_2 = 2,
  UEL_SYMBOL_3 = 3,
  UEL_SYMBOL_4 = 4,
  UEL_SYMBOL_5 = 5,
  UEL_SYMBOL_6 = 6,
  UEL_SYMBOL_7 = 7,
  UEL_SYMBOL_8 = 8,
  UEL_SYMBOL_9 = 9,
  UEL_SYMBOL_NUMBER
} uel_symbols_t;

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
 * @brief Маппинг направления движения в код symbol_code_e
 *
 * @param direction
 * @return symbol_code_e
 */
static inline symbol_code_e
map_direction_to_common_symbol(control_bits_states_t direction) {
  switch (direction) {
  case UEL_MOVE_UP:
    return SYMBOL_ARROW_UP;
  case UEL_MOVE_DOWN:
    return SYMBOL_ARROW_DOWN;

  case UEL_NO_MOVE:
    return SYMBOL_EMPTY;

  case MOVE_UP_AFTER_STOP:
    return SYMBOL_ARROW_UP;

  case MOVE_DOWN_AFTER_STOP:
    return SYMBOL_ARROW_DOWN;

  case MOVE_UP_OR_DOWN_AFTER_STOP:
    return SYMBOL_ARROW_BOTH;

    // default:
    //   return SYMBOL_EMPTY;
  }
}

void update_arrow_by_direction(control_bits_states_t direction) {
  symbol_code_e symbol = map_direction_to_common_symbol(direction);

  if (symbol == SYMBOL_EMPTY) {
    // Нет движения: очищаем стрелку
    indication_clear_arrow();
  } else {
    indication_set_static_arrow(symbol);
  }
}

// Маппинг символов nku_symbols_t в код symbol_code_e
static const symbol_code_e nku_symbol_to_common_table[UEL_SYMBOL_NUMBER] = {
    [UEL_SYMBOL_0] = SYMBOL_0, [UEL_SYMBOL_1] = SYMBOL_1,
    [UEL_SYMBOL_2] = SYMBOL_2, [UEL_SYMBOL_3] = SYMBOL_3,
    [UEL_SYMBOL_4] = SYMBOL_4, [UEL_SYMBOL_5] = SYMBOL_5,
    [UEL_SYMBOL_6] = SYMBOL_6, [UEL_SYMBOL_7] = SYMBOL_7,
    [UEL_SYMBOL_8] = SYMBOL_8, [UEL_SYMBOL_9] = SYMBOL_9};

/**
 * @brief Маппинг символов nku_symbols_t в код symbol_code_e с применением
 *        таблицы nku_symbol_to_common_table
 *
 * @param symbol_code
 * @return symbol_code_e
 */
static inline symbol_code_e map_to_common_symbol(uint8_t symbol_code) {
  if (symbol_code < UEL_SYMBOL_NUMBER) {
    return nku_symbol_to_common_table[symbol_code];
  } else {
    return SYMBOL_EMPTY;
  }
}

/**
 * @brief  Transform UIM6100 values of direction to common directionType that
 *         defined in drawing.h
 * @param  direction: Value from enum direction_uim_6100_t:
 *                    UIM_6100_MOVE_UP/UIM_6100_MOVE_DOWN/UIM_6100_NO_MOVE
 * @retval None
 */
static void transform_direction_to_common(control_bits_states_t direction) {
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

    // default:
    //   drawing_data.direction = NO_DIRECTION;
    //   break;
  }
}

static uint8_t gong[2] = {
    0,
};

// Гонг на стоп-этаже
static void setting_gong_stop(uint16_t current_location, uint8_t volume) {
  uint8_t arrival = current_location & GONG_ARRIVAL;

  /*  Если сигнал Гонг из 0 меняется на 1 и быты Движение Вверх и Вниз равны 0,
   * тогда детектируем прибытие на этаж */
  gong[0] = ((arrival == GONG_ARRIVAL) == 1) ? 1 : 0;

  if (gong[0] && !gong[1]) {
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
      break;
    }
  }
  gong[1] = gong[0];
}

bool is_evacuation_symbol = false;
// Режим Эвакуация (символы)
static void set_evacuation_symbol(uint8_t current_location) {
  if ((current_location & EVACUATION) == EVACUATION) {
    // set_floor_symbols(SYMBOL_E, SYMBOL_EMPTY);
    indication_set_floor(SYMBOL_EMPTY, SYMBOL_E);
    is_evacuation_symbol = true;
  } else {
    is_evacuation_symbol = false;
  }
}

/// Флаг для контроля перегруза кабины
bool is_cabin_overload = false;

bool is_overload_sound_on = false; // состояние звука

extern uint16_t overload_sound_ms;
extern volatile bool is_time_ms_for_overload_elapsed;

// Режим Перегрузка (символы и звук)
static void set_cabin_overload_symbol_sound(uint16_t current_location) {

  if ((current_location & CABIN_OVERLOAD) == CABIN_OVERLOAD) {

    if (matrix_settings.volume != VOLUME_0) {

      // Первый вход в режим перегрузки
      if (!is_cabin_overload) {
        is_cabin_overload = true;
        overload_sound_ms = 0;
        is_overload_sound_on = true;
        // start_buzzer_sound(BUZZER_FREQ_CABIN_OVERLOAD, VOLUME_3);
      }

      if (is_time_ms_for_overload_elapsed) {
        is_time_ms_for_overload_elapsed = false;

        if (is_overload_sound_on) {
          stop_buzzer_sound();
          is_overload_sound_on = false;
        } else {
          // start_buzzer_sound(BUZZER_FREQ_CABIN_OVERLOAD, VOLUME_3);
          is_overload_sound_on = true;
        }
      }
    }
    // set_symbols(SYMBOL_EMPTY, SYMBOL_K, SYMBOL_G_RU);
    indication_set_floor(SYMBOL_K, SYMBOL_G_RU);
  } else if (is_cabin_overload) {
    stop_buzzer_sound();
    is_cabin_overload = false;
    is_overload_sound_on = false;
    overload_sound_ms = 0;
  }
}

static bool is_fire_danger_symbol = false;

// Режим Пожар (символы)
static void set_fire_danger_symbol(uint8_t current_location) {
  if ((current_location & FIRE_DANGER_SYMBOL) == FIRE_DANGER_SYMBOL) {
    is_fire_danger_symbol = true;
    indication_set_floor(SYMBOL_EMPTY, SYMBOL_F);
  } else {
    is_fire_danger_symbol = false;
  }
}

static uint8_t fire_sound_edge[2] = {
    0,
};

uint8_t fire_sound_on_cnt = 0;
uint8_t fire_sound_off_cnt = 0;

// Режим Пожар (звук)
static void set_fire_danger_sound(uint8_t current_location) {
  if ((current_location & FIRE_DANGER_SOUND) == FIRE_DANGER_SOUND) {

    // Первый вход в режим Пожар (звук)
    if (!is_fire_danger_sound) {
      is_fire_danger_sound = true;
      fire_sound_on_cnt = 0;
      fire_sound_off_cnt = 0;
      // start_buzzer_sound(BUZZER_FREQ_CABIN_OVERLOAD, VOLUME_3);
    }

    fire_sound_on_cnt++;
    if (fire_sound_on_cnt > 4U) {
      stop_buzzer_sound();

      fire_sound_off_cnt++;
      if (fire_sound_off_cnt > 8U) {
        is_fire_danger_sound = false;
      }
    }
  } else if (is_fire_danger_sound) {
    stop_buzzer_sound();
    fire_sound_on_cnt = 0;
    fire_sound_off_cnt = 0;
  }
}

void cabin_indicator_special_regime(uint16_t current_location,
                                    control_bits_states_t control_bits) {

  // Гонг
  if (matrix_settings.volume != VOLUME_0) {
    setting_gong_stop(current_location, matrix_settings.volume);
  }

  // Перегруз, символы КГ
  set_cabin_overload_symbol_sound(current_location);

  // Режим Эвакуация, символ E
  set_evacuation_symbol(current_location);

  // Пожар, символ F
  set_fire_danger_symbol(current_location);

  // Пожар, по счетчику
  if (matrix_settings.volume != VOLUME_0 && is_fire_danger_symbol) {
    set_fire_danger_sound(current_location);
  }
}

static uint8_t shift_address = 0;

static void floor_indicator_special_regime(uint16_t current_location,
                                           control_bits_states_t control_bits) {

  // Гонг, прибытие на этаж с номером matrix_settings.addr_id
  if (drawing_data.floor >= LOCATION_P2 &&
      drawing_data.floor <= LOCATION_MINUS_1) {
    shift_address = 4;
  } else {
    shift_address = 0;
  }

  if (matrix_settings.volume != VOLUME_0 &&
      (matrix_settings.addr_id == drawing_data.floor - shift_address)) {
    // setting_gong_stop(current_location, matrix_settings.volume);
  }

  // Режим Эвакуация, символ E
  set_evacuation_symbol(current_location);

  // Пожар, символ F
  set_fire_danger_symbol(current_location);
}

//============================ ФИЛЬТРАЦИЯ ==================================

/// Флаг для контроля фильтрации данных
static bool is_data_filtered = false;

/// Кол-во полученных данных
static uint8_t number_received_data = 0;

/// Текущий индекс элемента в filter_buff
static uint8_t current_index_buff = 0;

typedef struct {
  uint16_t received_data; // 2 байта полученных данных (9 бит)
  uint8_t counter;        // счетчик повторений
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
                                 uint16_t received_data, uint8_t counter) {
  filter_struct->received_data = received_data;
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

/**
 * @brief Фильтрация полученных данных
 *
 */
static void filter_data(uint16_t received_data) {
  if (current_index_buff == 0) {
    for (uint8_t i = 0; i < FILTER_BUFF_SIZE; i++) {
      set_filter_structure(&filter_buff[i], 0, 0);
    }
    current_index_buff = 0;
  }

  bool is_data_found = false;
  number_received_data++;

  for (uint8_t i = 0; i < current_index_buff; i++) {
    if (filter_buff[i].received_data == received_data) {
      filter_buff[i].counter++;
      is_data_found = true;
      break;
    }
  }

  if (!is_data_found && current_index_buff < FILTER_BUFF_SIZE - 1) {
    set_filter_structure(&filter_buff[current_index_buff], received_data, 1);
    current_index_buff++;
  }

  if (number_received_data == FILTER_BUFF_SIZE) {
    sort_bubble(filter_buff, FILTER_BUFF_SIZE);

    is_data_filtered = true;
    number_received_data = 0;
    current_index_buff = 0;
  }
}

//============================ ОБРАБОТКА ==================================

/// @brief 6..8 биты
static control_bits_states_t control_bits;

/// @brief  0..5 биты
static uint16_t current_location;

static uint8_t dash_error_cnt = 0;
static uint8_t dash_error_disable = 0;
static const uint8_t TIME_FLOOR_DURING_DASH = 8;
static const uint8_t TIME_DASH_DURING_DASH = 10;
static bool is_dash_error_displayed = false;

uint8_t yy = 0;
uint8_t nn = 0;
uint8_t xx = 0;

#if 0
_Bool get_floor_symbols(uint8_t floor_code, symbol_code_e *left_elem,
                        symbol_code_e *right_elem) {
  assert((left_elem != NULL) && (right_elem != NULL));
  if ((floor_code > 0) && (floor_code <= 39)) {
    *left_elem = floor_code / 10;
    *right_elem = floor_code % 10;
  } else if ((floor_code >= LOCATION_C2) && (floor_code <= LOCATION_MINUS_1)) {
    UEL_write_ufloor_symbols(floor_code, left_elem, right_elem);
  } else {
    return 0;
  }
  if (*left_elem == 0)
    *left_elem = SYMBOL_EMPTY;
  return 1;
}

void write_floor_to_panel(uint8_t floor_code) {
  symbol_code_e left_elem = SYMBOL_EMPTY, right_elem = SYMBOL_EMPTY;
  /* записывем символы, соответсвующие
   * полученому номеру этажа
   */
  if (!get_floor_symbols(floor_code, &left_elem, &right_elem))
    return;
  /* установка этажа */
  indication_set_floor(left_elem, right_elem, (_Bool)1);
}
#endif

/**
 * @brief  Process data using UEL protocol
 *         1. Get 9 bits from 16 received bits;
 *         2. Set drawing_data structure, setting symbols and sound;
 *         3. Display matrix_string while next data is not received and
 *            interface is connected.
 * @param  received_data: Pointer to the buffer with received data by CAN
 * @retval None
 */
void process_data_uel(uint16_t *received_data) {
  /// Flag to control is receiving data completed by UART
  extern bool is_rx_data_completed;

  filter_data(*received_data);

  if (is_data_filtered) {
    is_data_filtered = false;

    data = ~(filter_buff[0].received_data) & NINE_BITS_MASK;

    control_bits = data & CONTROL_BITS_MASK;
    current_location = data & CODE_LOCATION_MASK;

#if 0
    xx++;
    control_bits = UEL_MOVE_DOWN;
    current_location = 15;
    set_floor_symbols(SYMBOL_E, SYMBOL_E);
    // if (xx > 15U) {
    //   control_bits = SPECIAL_FORMAT;
    //   current_location = GONG_ARRIVAL;
    // }
#endif

    /* Разделение по контрольным битам */
    switch (control_bits) {
      /* Посылка 2 */
    case NUMBER_CLICKED_BTN_ORDER:

      break;

      /* Посылка 4: Перегрузка, Пожар, Эвакуация, Гонг */
    case SPECIAL_FORMAT:
      // Спец. режимы
      // Кабинный индикатор
      if (matrix_settings.addr_id == MAIN_CABIN_ID) {
        cabin_indicator_special_regime(current_location, control_bits);
      } else {
        // Этажный индикатор
        floor_indicator_special_regime(current_location, control_bits);
      }
      break;

    default: /* Посылки 1 и 3 (без 2 и 4) */
      // Этаж для отображения
      drawing_data.floor = current_location;

      // Направление для гонга
      transform_direction_to_common(control_bits);

      // Настройка кода стрелки
      // indication_set_static_arrow(map_direction_to_common_symbol(control_bits));

      update_arrow_by_direction(control_bits);

      // Этажи с 0 по 9
      if (!is_fire_danger_symbol && !is_cabin_overload &&
          !is_evacuation_symbol) {

        // Этажи с 0 по 9
        if (drawing_data.floor >= 0 && drawing_data.floor <= 9) {
          indication_set_floor(map_to_common_symbol(SYMBOL_EMPTY),
                               map_to_common_symbol(drawing_data.floor));
        } else if (drawing_data.floor <= MAX_POSITIVE_NUMBER_LOCATION) {
          // Этажи с 10 по 39
          indication_set_floor(map_to_common_symbol(drawing_data.floor / 10),
                               map_to_common_symbol(drawing_data.floor % 10));
        } else { // Этажи после 39
          /* Этажи --, П2, П1, П, -4, -3, -2, -1 */
          /* Режимы МП1, МП2, Ревизия, Норм. Раб., Погр */
          switch (drawing_data.floor) {
          case LOCATION_DASH:
            indication_set_floor(SYMBOL_MINUS, SYMBOL_MINUS);
            break;

          case LOCATION_P2:
            indication_set_floor(SYMBOL_UNDERGROUND_FLOOR_BIG, SYMBOL_2);
            break;
          case LOCATION_P1:
            indication_set_floor(SYMBOL_UNDERGROUND_FLOOR_BIG, SYMBOL_1);
            break;

          case LOCATION_P:
            indication_set_floor(SYMBOL_EMPTY, SYMBOL_UNDERGROUND_FLOOR_BIG);
            break;

          case LOCATION_MINUS_4:
            indication_set_floor(SYMBOL_MINUS, SYMBOL_4);
            break;

          case LOCATION_MINUS_3:
            indication_set_floor(SYMBOL_MINUS, SYMBOL_3);
            break;

          case LOCATION_MINUS_2:
            indication_set_floor(SYMBOL_MINUS, SYMBOL_2);
            break;

          case LOCATION_MINUS_1:
            indication_set_floor(SYMBOL_MINUS, SYMBOL_1);
            break;

          case LOCATION_C1:
            indication_set_floor(SYMBOL_C, SYMBOL_1);
            break;

          case LOCATION_C2:
            indication_set_floor(SYMBOL_C, SYMBOL_2);
            break;

          case LOCATION_REVISION:
            indication_set_floor(SYMBOL_P, SYMBOL_E);
            break;

            // case LOCATION_NORMAL_WORK:
            //   break;

          case LOCATION_LOADING:
            if (matrix_settings.addr_id == MAIN_CABIN_ID) {
              indication_set_floor(SYMBOL_UNDERGROUND_FLOOR_BIG, SYMBOL_G_RU);
            }
            break;

          default:
            break;
          } //  switch (drawing_data.floor)
        }   // Этажи после 39
      }
      break; // default
    }        //  switch (control_bits)
  }          // if (is_data_filtered)

  // indication_set_floor(SYMBOL_K, SYMBOL_G_RU);
  // indication_set_static_arrow(SYMBOL_ARROW_DOWN);

  while (is_rx_data_completed == false && is_interface_connected == true) {
#if DOT_PIN
    draw_string_on_matrix(matrix_string);
#elif DOT_SPI
    // display_symbols_spi();
    update_LED_panel();
    // write_floor_to_panel(11);
    // send_bitmap_to_display();
#endif
  }
}

#if 0
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
#endif

#if 0
  if ((current_location & GONG_ARRIVAL) == GONG_ARRIVAL) {
    if (!is_gong_play) {
      stop_buzzer_sound();

      if (matrix_settings.volume != VOLUME_0) {
        play_gong(3, GONG_BUZZER_FREQ, matrix_settings.volume);
      }

      is_gong_play = true;
    }
  } else {
    is_gong_play = false;
  }
#endif
