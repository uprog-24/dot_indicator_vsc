/**
 * @file drawing.c
 */
#include "drawing.h"

#include "dot.h"
#include "font.h"

#include <stdbool.h>
#include <string.h>

#define START_INDEX_SYMBOL_ROW                                                 \
  6 ///< Код символа в двоичном представлении начинается с 6-го индекса в
    ///< строке. Отрисовка символа слева направо

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

/* Флаг для удержания состояния строки в течение 1 мс (максимальная яркость,
 * частота обновления матрицы 125 Гц) */
extern volatile bool is_tim4_period_elapsed;
/**
 * @brief  Отображение символа на матрице
 * @note   Построчно проходим по коду символа, удерживая состояние строки 1 мс
 *         (максимвльная яркость, частота обновления матрицы 125 Гц)
 * @param  symbol_code: Код символа для отображения (из font.h)
 * @param  start_pos:   Начальная позиция (индекс столбца матрицы) для символа
 */
static void draw_symbol_on_matrix(symbol_code_e symbol_code,
                                  uint8_t start_pos) {

  static uint8_t current_row = 0;

  // Включаем текущую строку
  set_row_state(current_row, TURN_ON);

  // Получаем значения для колонок текущей строки (строка кода символа)
  uint8_t binary_symbol_code_row = bitmap[symbol_code][current_row];

  for (uint8_t i = 0; i <= START_INDEX_SYMBOL_ROW; i++) {
    // Если бит в строке символа = 1, то включаем колонку
    if ((binary_symbol_code_row >> (START_INDEX_SYMBOL_ROW - i)) & 1) {
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
 * @brief Расчет ширины символа (заполнение структуры)
 * @param symbol_bitmap_rows: Двоичное представление символа
 * @return uint8_t
 */
static uint8_t calculate_symbol_width(const uint8_t *symbol_bitmap_rows) {
  uint8_t min_col = 8;
  uint8_t max_col = 0;
  bool has_pixels = false;

  for (int row = 0; row < NUMBER_OF_ROWS; row++) {
    uint8_t row_data = symbol_bitmap_rows[row];

    if (row_data != 0) {
      has_pixels = true;

      // Найдём первую установленную единицу слева (индексы от 7 до 0)
      for (int bit_index = 0; bit_index <= 7; ++bit_index) {
        if (row_data & (1 << (7 - bit_index))) {
          if (bit_index < min_col)
            min_col = bit_index;
          break;
        }
      }

      // Найдём последнюю установленную единицу справа
      for (int bit_index = 7; bit_index >= 0; --bit_index) {
        if (row_data & (1 << (7 - bit_index))) {
          if (bit_index > max_col)
            max_col = bit_index;
          break;
        }
      }
    }
  }

  if (!has_pixels)
    return 0; // символ без 1 (Пусто)

  return (max_col - min_col + 1);
}

/**
 * Структура для ширин символов
 */
typedef struct {
  const uint8_t *bitmap;
  uint8_t width;
} symbol_descriptor_t;

static symbol_descriptor_t symbols_meta[NUMBER_OF_SYMBOLS];

/**
 * @brief Заполнение структуры: битмап и ширина символа
 */
void init_symbols_width() {
  for (int i = 0; i < NUMBER_OF_SYMBOLS; i++) {
    symbols_meta[i].bitmap = bitmap[i];
    symbols_meta[i].width = calculate_symbol_width(bitmap[i]);
  }
}

/**
 * @brief Отображение символов, заранее установленных в структуру symbols
 */
void draw_symbols() {

  uint8_t start_pos = 0;

  // c1c..c9c, cFc и др.
  if (symbols.symbol_code_1 == SYMBOL_EMPTY &&
      symbols.symbol_code_3 == SYMBOL_EMPTY) {
    start_pos += 5 + 1; // Сдвиг в связи с символом Пусто
    draw_symbol_on_matrix(symbols.symbol_code_2, start_pos);

  } else if (symbols.symbol_code_1 == SYMBOL_EMPTY &&
             symbols.symbol_code_3 != SYMBOL_EMPTY) {
    // c10..c99, cКГ и др.
    start_pos += 3 + 1; // Сдвиг в связи с символом Пусто
    draw_symbol_on_matrix(symbols.symbol_code_2, start_pos);

    start_pos += symbols_meta[symbols.symbol_code_2].width + 1;
    draw_symbol_on_matrix(symbols.symbol_code_3, start_pos);
  } else {
    // Если символ 1 и символ 3 не SYMBOL_EMPTY
    draw_symbol_on_matrix(symbols.symbol_code_1, start_pos);

    start_pos += symbols_meta[symbols.symbol_code_1].width + 1;
    draw_symbol_on_matrix(symbols.symbol_code_2, start_pos);

    start_pos += symbols_meta[symbols.symbol_code_2].width + 1;
    draw_symbol_on_matrix(symbols.symbol_code_3, start_pos);
  }
}

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
    ['K'] = SYMBOL_K,
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
  set_symbols(char_to_symbol(matrix_string[0]),
              char_to_symbol(matrix_string[1]),
              char_to_symbol(matrix_string[2]));
  draw_symbols();
}

extern volatile bool is_time_ms_for_display_str_elapsed;
/**
 * @brief  Отображение строки на матрице в течение
 *         TIME_DISPLAY_STRING_DURING_MS (определено в tim.c)
 * @note   Для DEMO_MODE и для протоколов при запуске индикатора
 * @param  matrix_string: Указатель на строку, которая будет отображаться
 */
void display_string_during_ms(char *matrix_string) {
  is_time_ms_for_display_str_elapsed = false;

  while (!is_time_ms_for_display_str_elapsed) {
    draw_string(matrix_string);
  }

  // Очищаем поля структуры с символами
  set_symbols(SYMBOL_EMPTY, SYMBOL_EMPTY, SYMBOL_EMPTY);
}
