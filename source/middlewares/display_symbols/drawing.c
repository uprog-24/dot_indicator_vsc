/**
 * @file drawing.c
 */
#include "drawing.h"

#include "config.h"
#include "dot.h"
#include "font.h"

#if DOT_SPI
#include "LED_driver.h"
#include "software_SPI.h"
#endif

#include <stdbool.h>
#include <string.h>

#define BINARY_SYMBOL_CODE_SIZE 6 ///< Binary symbol code size (number of bits)
#define MIN_POSITION_COLUMN                                                    \
  0 ///< Minimum index of position of column for symbol
#define MAX_POSITION_COLUMN                                                    \
  11 ///< Maximum index of position of column for symbol

/**
 * Индексы строки, которая будет отображаться на матрице.
 * Направление имеет позицию 0;
 * MSB (старший бит, первый символ) имеет позицию 1;
 * LSB (младший бит, второй символ) имеет позицию 2.
 */
typedef enum { DIRECTION = 0, MSB = 1, LSB = 2, INDEX_NUMBER } symbol_index_t;

/* Структура, содержащая коды символов для отрисовки */
typedef struct {
  symbol_code_e symbol_code_1;
  symbol_code_e symbol_code_2;
  symbol_code_e symbol_code_3;
} displayed_symbols_t;

static displayed_symbols_t symbols = {
    .symbol_code_1 = SYMBOL_EMPTY,
    .symbol_code_2 = SYMBOL_EMPTY,
    .symbol_code_3 = SYMBOL_EMPTY,
};

/**
 * @brief Установка символа в структуру по индексу
 *
 * @param index: Индекс символа
 * @param symbol Код символа из перечисления symbol_code_e
 */
static void set_symbol_at(symbol_index_t index, symbol_code_e symbol) {
  if (index >= INDEX_NUMBER)
    return;

  switch (index) {
  case DIRECTION:
    symbols.symbol_code_1 = symbol;
    break;
  case MSB:
    symbols.symbol_code_2 = symbol;
    break;
  case LSB:
    symbols.symbol_code_3 = symbol;
    break;
  }
}

/**
 * @brief  Установка символа направления движения
 * @param  direction_code: Код направления (из перечисления symbol_code_e)
 */
void set_direction_symbol(symbol_code_e direction_code) {
  set_symbol_at(DIRECTION, direction_code);
}

/**
 * @brief  Установка символов для этажей
 * @param  left_symbol_code:  Код символа 1
 * @param  right_symbol_code: Код символа 2
 */
void set_floor_symbols(symbol_code_e left_symbol_code,
                       symbol_code_e right_symbol_code) {
  set_symbol_at(MSB, left_symbol_code);
  set_symbol_at(LSB, right_symbol_code);
}

/**
 * @brief  Установка символов (направление + этаж)
 * @param  s1_code:  Код символа 1
 * @param  s2_code:  Код символа 2
 * @param  s3_code:  Код символа 3
 */
void set_symbols(symbol_code_e s1_code, symbol_code_e s2_code,
                 symbol_code_e s3_code) {
  set_direction_symbol(s1_code);
  set_floor_symbols(s2_code, s3_code);
}

/**
 * @brief  Setting structure with type drawing_data_t
 * @param  drawing_data: Pointer to the structure with type drawing_data_t
 * @param  floor:         Floor
 * @param  direction:     Direction with type directionType
 * @retval None
 */
void drawing_data_setter(drawing_data_t *drawing_data, uint8_t floor,
                         directionType direction) {
  drawing_data->floor = floor;
  drawing_data->direction = direction;
}
#if DOT_SPI

#define SYMBOL_TABLE_SIZE 128

// Маппинг символов char в код symbol_code_e
static const symbol_code_e char_to_symbol_table[SYMBOL_TABLE_SIZE] = {
    ['0'] = SYMBOL_0,    ['1'] = SYMBOL_1,     ['2'] = SYMBOL_2,
    ['3'] = SYMBOL_3,    ['4'] = SYMBOL_4,     ['5'] = SYMBOL_5,
    ['6'] = SYMBOL_6,    ['7'] = SYMBOL_7,     ['8'] = SYMBOL_8,
    ['9'] = SYMBOL_9,    ['S'] = SYMBOL_S,     ['I'] = SYMBOL_I,
    ['D'] = SYMBOL_D,    ['-'] = SYMBOL_MINUS, [' '] = SYMBOL_EMPTY,
    ['.'] = SYMBOL_DOT,  ['c'] = SYMBOL_EMPTY, ['V'] = SYMBOL_V,
    ['L'] = SYMBOL_L,    ['C'] = SYMBOL_C,     ['E'] = SYMBOL_E,
    ['+'] = SYMBOL_PLUS, ['p'] = SYMBOL_P,     ['g'] = SYMBOL_G,
    ['K'] = SYMBOL_K,    ['U'] = SYMBOL_U_BIG,
};

/**
 * @brief Преобразование символа char в код symbol_code_e
 * @param ch
 * @return symbol_code_e
 */
static inline symbol_code_e char_to_symbol(char ch) {
  if (ch < SYMBOL_TABLE_SIZE) {
    return char_to_symbol_table[(unsigned char)ch];
  } else {
    return SYMBOL_EMPTY;
  }
}

/**
 * @brief Отображение строки
 * @param matrix_string: Указатель на строку, которая будет отображаться
 */
void draw_string(char *matrix_string) {
// Преобразуем символ char в код symbol_code_e
#if 0
  set_symbols(char_to_symbol(matrix_string[0]),
              char_to_symbol(matrix_string[1]),
              char_to_symbol(matrix_string[2]));
  display_symbols_spi();
#else
  // indication_set_static_arrow(char_to_symbol(matrix_string[0]), 0);
  // indication_set_floor(char_to_symbol(matrix_string[1]),
  //                      char_to_symbol(matrix_string[2]), 0);

  // update_LED_panel();

  prepare_symbols(char_to_symbol(matrix_string[0]),
                  char_to_symbol(matrix_string[1]),
                  char_to_symbol(matrix_string[2]));

  render_prepared_symbols();

#endif
}

extern volatile bool is_time_ms_for_display_str_elapsed;
/**
 * @brief  Отображение строки на матрице в течение
 *         TIME_DISPLAY_STRING_DURING_MS (определено в tim.c)
 * @note   Для протоколов при запуске индикатора
 * @param  matrix_string: Указатель на строку, которая будет отображаться
 */
void display_string_during_ms(char *matrix_string) {
  is_time_ms_for_display_str_elapsed = false;

  while (!is_time_ms_for_display_str_elapsed) {
    draw_string(matrix_string);
  }

  // Очищаем поля структуры с символами
  // set_symbols(SYMBOL_EMPTY, SYMBOL_EMPTY, SYMBOL_EMPTY);
  // indication_clear_all_panels();
  prepare_symbols(SYMBOL_EMPTY, SYMBOL_EMPTY, SYMBOL_EMPTY);
}

/**
 * @brief  Отображение строки на матрице в течение
 *         TIME_DISPLAY_STRING_DURING_MS (определено в tim.c)
 * @note   Для DEMO_MODE
 */
void display_symbols_during_ms() {
  is_time_ms_for_display_str_elapsed = false;

  while (!is_time_ms_for_display_str_elapsed) {
    // display_symbols_spi();
  }
}

extern const uint8_t bitmap[46][6];

typedef struct {
  uint16_t data[6]; // 6 строк символа (бинарное представление)
} SymbolData;

SymbolData g_sym1;
SymbolData g_sym2;
SymbolData g_sym3;

static void prepare_symbol_data(const uint8_t *symbol, SymbolData *out_data) {
  for (int row = 0; row < 6; ++row) {
    uint16_t row_flag = 1 << (2 + row);
    out_data->data[row] = row_flag | ((uint16_t)symbol[row] << 8);
  }
}

#if 1
void prepare_floor_symbols(uint8_t index2, uint8_t index3) {

  // Если левый символ пустой
  if (index2 == SYMBOL_EMPTY) {

    uint8_t *symbol_bitmap_ptr = (uint8_t *)bitmap[index3];

    // левое знакоместо
    for (int row = 0; row < 3; ++row) {
      uint16_t row_flag = 1 << (2 + row);
      g_sym2.data[row] = row_flag | ((uint16_t)symbol_bitmap_ptr[row + 3] << 8);
    }

    // правое знакоместо
    for (int row = 3; row < 6; ++row) {
      uint16_t row_flag = 1 << (2 + row);
      g_sym3.data[row - 3] =
          row_flag | ((uint16_t)symbol_bitmap_ptr[row - 3] << 8);
    }

    for (int row = 3; row < 6; ++row) {
      g_sym3.data[row] = 0;
    }
    for (int row = 3; row < 6; ++row) {
      g_sym2.data[row] = 0;
    }

  } else {
    prepare_symbol_data(bitmap[index2], &g_sym2);
    prepare_symbol_data(bitmap[index3], &g_sym3);
  }
}
#endif

void prepare_dir_symbol(uint8_t index1) {
  prepare_symbol_data(bitmap[index1], &g_sym1);
}

void prepare_symbols(uint8_t index1, uint8_t index2, uint8_t index3) {
  prepare_dir_symbol(index1);
  prepare_floor_symbols(index2, index3);
}

static void LED_driver_send2(uint16_t data) {
  uint8_t hi = (data >> 8) & 0xFF;
  uint8_t lo = data & 0xFF;

  software_SPI_sendByte(lo); // сначала старший
  software_SPI_sendByte(hi); // потом младший
}

static void LED_driver_send_buffer_raw(uint16_t d1, uint16_t d2, uint16_t d3) {
  // В порядке: последним первым (т.к. регистры сдвигаются)
  LED_driver_send2(d1);
  LED_driver_send2(d2);
  LED_driver_send2(d3);

  LED_driver_impulse_to_latch();
  LED_driver_start_indication();
}

void render_prepared_symbols() {
  for (int row = 0; row < 6; ++row) {
    LED_driver_send_buffer_raw(g_sym1.data[row], g_sym2.data[row],
                               g_sym3.data[row]);
  }
}

void render_symbols_by_index(uint8_t index1, uint8_t index2, uint8_t index3) {
  const uint8_t *symbol1 = bitmap[index1];
  const uint8_t *symbol2 = bitmap[index2];
  const uint8_t *symbol3 = bitmap[index3];

  for (int row = 0; row < 6; ++row) {
    uint16_t row_flag = 1 << (2 + row);

    uint16_t data1 = row_flag | ((uint16_t)symbol1[row] << 8);
    uint16_t data2 = row_flag | ((uint16_t)symbol2[row] << 8);
    uint16_t data3 = row_flag | ((uint16_t)symbol3[row] << 8);

    // Отрисовать строку всех 3 драйверов
    LED_driver_send_buffer_raw(data1, data2, data3);
  }
}

#else

extern volatile bool is_tim4_period_elapsed;

/**
 * @brief  Draw the symbol on matrix starting with start_pos in range
 *         [MIN_POSITION_COLUMN, MAX_POSITION_COLUMN]
 * @note   1. Get the symbol code from symbols[];
 *         2. Go by current_row, TURN_ON current_row, TURN_OFF others;
 *         3. Get binary_symbol_code_row for current_row from cur_symbol_code[];
 *         4. Go by all columns and set TURN_ON if
 *            binary_symbol_code_row[num_bit] = 1 else set TURN_OFF;
 *         5. TURN_OFF all columns and rows after process current_row in order
 *            to avoid the effect of shadow (turning on unnecessary LEDs).
 * @param  symbol:    Symbol from symbols[] (font.c)
 * @param  start_pos: Start position (index of column) for symbol
 * @param  shift:     Shift by Y for animation of symbol movement
 * @retval None
 */
static void draw_symbol_on_matrix(char symbol, uint8_t start_pos,
                                  uint8_t shift) {

  uint8_t *cur_symbol_code = get_symbol_code(symbol);
  if (cur_symbol_code == NULL)
    return;

  static uint8_t current_row = 0;

  // Включаем текущую строку
  set_row_state(current_row, TURN_ON);

  // Получаем значения для колонок текущей строки
  uint8_t binary_symbol_code_row[BINARY_SYMBOL_SIZE];
  if (current_row + shift < ROWS) {
    convert_number_from_dec_to_bin(cur_symbol_code[current_row + shift],
                                   binary_symbol_code_row,
                                   BINARY_SYMBOL_CODE_SIZE);
  } else {
    memset(binary_symbol_code_row, 0, BINARY_SYMBOL_SIZE);
  }

  // Включаем колонку, если бит = 1
  for (uint8_t i = 0; i < 7; i++) {
    uint8_t current_col = BINARY_SYMBOL_CODE_SIZE - i;
    if (binary_symbol_code_row[current_col] == 1) {
      set_col_state(start_pos + i, TURN_ON);
    }
  }

  /**
   * Держим состояние строки с колонками, пока таймер не завершит
   * отсчет (1000 мкс)
   */
  if (is_tim4_period_elapsed) {
    is_tim4_period_elapsed = false;

    // Переходим к следующей строке
    current_row++;

    // Выключаем предыдущую строку
    if (current_row) {
      set_row_state(current_row - 1, TURN_OFF);
    }
    // Выключаем все колонки
    set_all_cols_state(TURN_OFF);

    // Завершаем проход по строкам
    if (current_row >= ROWS) {
      current_row = 0;
    }
  }
}

/**
 * @brief  Check type of start symbol, special: 'c', '>', '<', '+' or not
 * @param  matrix_string: Pointer to the matrix_string that will be displayed on
 *         matrix
 * @retval None
 */
static bool is_start_symbol_special(char *matrix_string) {
  return (matrix_string[DIRECTION] == 'c' || matrix_string[DIRECTION] == '>' ||
          matrix_string[DIRECTION] == '<' || matrix_string[DIRECTION] == '+' ||
          matrix_string[DIRECTION] == '-' || matrix_string[DIRECTION] == 'p');
}

/**
 * @brief  Draw matrix_string (without special start symbol - matrix_string[0]:
 *         'c','>', '<', '+') on matrix
 * @param  matrix_string: Pointer to the matrix_string that will be displayed on
 *                        matrix
 * @retval None
 */
static void draw_symbols(char *matrix_string) {
  if (strlen(matrix_string) == 3) { // 3 symbols, font_width = 4

    // draw DIRECTION symbol
    if (matrix_string[DIRECTION] == 'V' || matrix_string[DIRECTION] == 'K' ||
        matrix_string[MSB] == 'K') { // font_width = 5
      draw_symbol_on_matrix(matrix_string[DIRECTION], 0, 0);
    } else {
      draw_symbol_on_matrix(matrix_string[DIRECTION], 1, 0);
    }

    // draw MSB symbol
    if (matrix_string[DIRECTION] == 'U' && matrix_string[MSB] == 'K') {
      draw_symbol_on_matrix(matrix_string[MSB], 5, 0);
    } else {
      draw_symbol_on_matrix(matrix_string[MSB], 6, 0);
    }

    // draw LSB symbol
    if (matrix_string[MSB] == '.') { // version
      draw_symbol_on_matrix(matrix_string[LSB], 8, 0);
    } else {
      draw_symbol_on_matrix(matrix_string[LSB], 11, 0);
    }
  }

  if (strlen(matrix_string) == 4) {
    draw_symbol_on_matrix(matrix_string[0], 0, 0);
    draw_symbol_on_matrix(matrix_string[1], 5, 0);
    draw_symbol_on_matrix(matrix_string[2], 8, 0);
    draw_symbol_on_matrix(matrix_string[3], 11, 0);
  }
}

/**
 * @brief  Draw matrix_string (with special start symbol matrix_string[0]: 'c',
 *         '>', '<', '+') on matrix
 * @param  matrix_string: Pointer to the matrix_string that will be displayed on
 *                        matrix
 * @retval None
 */
static void draw_special_symbols(char *matrix_string) {
  // stop floor 1..9: c1c
  if (matrix_string[DIRECTION] == 'c' && matrix_string[LSB] == 'c') {
    draw_symbol_on_matrix(matrix_string[DIRECTION], 0, 0);
    draw_symbol_on_matrix(matrix_string[LSB], 0, 0);
    draw_symbol_on_matrix(matrix_string[MSB], 6, 0);

  } else if (matrix_string[DIRECTION] ==
             'c') { // stop floor c10..c99 and c-1..c-9

    draw_symbol_on_matrix(matrix_string[DIRECTION], 0, 0);
    if (matrix_string[MSB] == '1' || matrix_string[MSB] == 'I') {
      draw_symbol_on_matrix(matrix_string[MSB], 5, 0);
      draw_symbol_on_matrix(matrix_string[LSB], 9, 0);
    } else if (matrix_string[MSB] != '-') {
      draw_symbol_on_matrix(matrix_string[MSB], 4, 0);
      // "cKg" перегруз
      if (matrix_string[MSB] == 'K' && matrix_string[LSB] == 'g') {
        draw_symbol_on_matrix(matrix_string[LSB], 10, 0);
      } else {
        draw_symbol_on_matrix(matrix_string[LSB], 9, 0);
      }
    }

    if (matrix_string[MSB] == '-' && matrix_string[LSB] != '-') {
      draw_symbol_on_matrix(matrix_string[MSB], 4, 0);
      draw_symbol_on_matrix(matrix_string[LSB], 8, 0);
    }

    // "c--" interface is not connected
    if (matrix_string[MSB] == '-' && matrix_string[LSB] == '-') {
      draw_symbol_on_matrix(matrix_string[MSB], 9, 0);
      draw_symbol_on_matrix(matrix_string[LSB], 4, 0);
    }
  } else if (matrix_string[DIRECTION] == '>' ||
             matrix_string[DIRECTION] == '<' ||
             matrix_string[DIRECTION] == '+' ||
             matrix_string[DIRECTION] == '-' ||
             matrix_string[DIRECTION] == 'p') { // in moving up/down: >10 or >1c

    if (matrix_string[DIRECTION] == '-' || matrix_string[DIRECTION] == 'p') {
      draw_symbol_on_matrix(matrix_string[DIRECTION], 1, 0);
    } else {
      draw_symbol_on_matrix(matrix_string[DIRECTION], 0, 0);
    }

    draw_symbol_on_matrix(matrix_string[MSB], 6, 0);

    // font_width = 3 for '1' and '-'
    if (matrix_string[MSB] == '1' || matrix_string[MSB] == '-') {
      draw_symbol_on_matrix(matrix_string[LSB], 10, 0);
    } else if (matrix_string[MSB] != '-') {
      draw_symbol_on_matrix(matrix_string[LSB], 11, 0);
    }
  }
}

/**
 * @brief  Draw matrix_string on matrix depend on the type of matrix_string
 * @param  matrix_string: Pointer to the matrix_string that will be displayed on
 *                        matrix
 * @retval None
 */
void draw_string_on_matrix(char *matrix_string) {

  if (is_start_symbol_special(matrix_string)) {
    draw_special_symbols(matrix_string);
  } else {
    draw_symbols(matrix_string);
  }
}
#endif