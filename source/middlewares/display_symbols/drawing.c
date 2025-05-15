/**
 * @file drawing.c
 */
#include "drawing.h"

#include "dot.h"
#include "font.h"

#include <stdbool.h>
#include <string.h>

#define BINARY_SYMBOL_CODE_SIZE                                                \
  6 ///< Количество битов в строке кода символа в font.c.

/**
 * Индексы строки, которая будет отображаться на матрице.
 * Направление имеет позицию 0;
 * MSB (старший бит, первый символ) имеет позицию 1;
 * LSB (младший бит, второй символ) имеет позицию 2.
 */
typedef enum { DIRECTION = 0, MSB = 1, LSB = 2, INDEX_NUMBER } symbol_index_t;

static displayed_symbols_t symbols = {
    .symbol_code_1 = SYMBOL_EMPTY,
    .symbol_code_2 = SYMBOL_EMPTY,
    .symbol_code_3 = SYMBOL_EMPTY,
};

/**
 * @brief Установка символа в структуру по индексу
 *
 * @param index: Индекс символа
 * @param symbol Код символа из перечисления symbol_e
 */
static void set_symbol_at(symbol_index_t index, symbol_e symbol) {
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
 * @param  direction_code: Код направления (из перечисления symbol_e).
 */
void set_direction_symbol(symbol_e direction_code) {
  set_symbol_at(DIRECTION, direction_code);
}

/**
 * @brief  Установка символов для этажей
 * @param  left_symbol_code:  Код символа 1
 * @param  right_symbol_code: Код символа 2
 */
void set_floor_symbols(symbol_e left_symbol_code, symbol_e right_symbol_code) {
  set_symbol_at(MSB, left_symbol_code);
  set_symbol_at(LSB, right_symbol_code);
}

void set_symbols(symbol_e s1, symbol_e s2, symbol_e s3) {
  set_direction_symbol(s1);
  set_floor_symbols(s2, s3);
}

/* Флаг для удержания состояния строки в течение 1 мс (максимальная яркость,
 * частота обновления матрицы 125 Гц) */
extern volatile bool is_tim4_period_elapsed;
/**
 * @brief  Отображение символа на матрице
 * @note   Построчно проходим по коду символа, удерживая состояние строки 1 мс
 *         (максимвльная яркость, частота обновления матрицы 125 Гц)
 * @param  symbol_code: Код символа для отображения (из font.h)
 * @param  start_pos:   Начальная позиция (индекс столбца) для символа
 */
void draw_symbol_on_matrix(symbol_e symbol_code, uint8_t start_pos) {

  static uint8_t current_row = 0;

  // Включаем текущую строку
  set_row_state(current_row, TURN_ON);

  // Получаем значения для колонок текущей строки (строка кода символа)
  uint8_t binary_symbol_code_row = bitmap[symbol_code][current_row];

  // Включаем колонку, если бит в строке символа = 1
  for (uint8_t i = 0; i < 7; i++) {
    // uint8_t current_col = BINARY_SYMBOL_CODE_SIZE - i;
    // Если бит в строке символа = 1, то включаем колонку
    if ((binary_symbol_code_row >> (6 - i)) & 1) {
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
 * @brief  Проверка начального символа DIRECTION, является ли он спец.: 'c',
 *         '>', '<', '+'.
 * @param  matrix_string: Указатель на строку.
 * @retval None
 */
static bool is_start_symbol_special(char *matrix_string) {
  return (matrix_string[DIRECTION] == 'c' || matrix_string[DIRECTION] == '>' ||
          matrix_string[DIRECTION] == '<' || matrix_string[DIRECTION] == '+' ||
          matrix_string[DIRECTION] == '-' || matrix_string[DIRECTION] == 'p');
}

/**
 * @brief  Отображение обычной строки (без начального спец. символа -
 *         matrix_string[DIRECTION]: 'c','>', '<', '+').
 * @param  matrix_string: Указатель на строку.
 * @retval None
 */
#if 0
static void draw_symbols(char *matrix_string) {

  /* Для включения всех светодиодов в DEMO_MODE с ШИМ */
#if 0
  if (strlen(matrix_string) == 2) {
    draw_symbol_on_matrix(matrix_string[DIRECTION], 0, 0);
    draw_symbol_on_matrix(matrix_string[DIRECTION], 7, 0);
    draw_symbol_on_matrix(matrix_string[DIRECTION], 9, 0);
  }
#endif

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
#endif

void draw_symbols() {

  /* Для включения всех светодиодов в DEMO_MODE с ШИМ */
#if 0
  if (strlen(matrix_string) == 2) {
    draw_symbol_on_matrix(matrix_string[DIRECTION], 0, 0);
    draw_symbol_on_matrix(matrix_string[DIRECTION], 7, 0);
    draw_symbol_on_matrix(matrix_string[DIRECTION], 9, 0);
  }
#endif

  uint8_t pos = 4;

  if (symbols.symbol_code_1 == SYMBOL_EMPTY &&
      symbols.symbol_code_3 != SYMBOL_EMPTY) {
    draw_symbol_on_matrix(symbols.symbol_code_2, 4);
    pos += 5 + 1;
    draw_symbol_on_matrix(symbols.symbol_code_3, pos);
  } else {
    draw_symbol_on_matrix(symbols.symbol_code_1, 0);
    draw_symbol_on_matrix(symbols.symbol_code_2, 6);
    draw_symbol_on_matrix(symbols.symbol_code_3, 11);
  }

  // if (strlen(matrix_string) == 3) { // 3 symbols, font_width = 4

  // draw DIRECTION symbol
  // if (matrix_string[DIRECTION] == 'V' || matrix_string[DIRECTION] == 'K' ||
  //     matrix_string[MSB] == 'K') { // font_width = 5
  //   draw_symbol_on_matrix(matrix_string[DIRECTION], 0);
  // } else {
  //   draw_symbol_on_matrix(matrix_string[DIRECTION], 1);
  // }

  // // draw MSB symbol
  // if (matrix_string[DIRECTION] == 'U' && matrix_string[MSB] == 'K') {
  //   draw_symbol_on_matrix(matrix_string[MSB], 5);
  // } else {
  //   draw_symbol_on_matrix(matrix_string[MSB], 6);
  // }

  // // draw LSB symbol
  // if (matrix_string[MSB] == '.') { // version
  //   draw_symbol_on_matrix(matrix_string[LSB], 8);
  // } else {
  //   draw_symbol_on_matrix(matrix_string[LSB], 11);
  // }
  // }
}

/**
 * @brief  Отображение спец. строки (с начальным спец. символом -
 *         matrix_string[DIRECTION]: 'c','>', '<', '+').
 * @param  matrix_string: Указатель на строку.
 * @retval None
 */
static void draw_special_symbols(char *matrix_string) {
  // stop floor 1..9: "c1c"
  if (matrix_string[DIRECTION] == 'c' && matrix_string[LSB] == 'c') {
    draw_symbol_on_matrix(matrix_string[DIRECTION], 0);
    draw_symbol_on_matrix(matrix_string[LSB], 0);
    draw_symbol_on_matrix(matrix_string[MSB], 6);

  } else if (matrix_string[DIRECTION] ==
             'c') { // stop floor "c10".."c99" and "c-1".."c-9"

    // draw DIRECTION symbol
    draw_symbol_on_matrix(matrix_string[DIRECTION], 0);

    // draw MSB and LSB symbols
    if (matrix_string[MSB] == '1' || matrix_string[MSB] == 'I') {
      draw_symbol_on_matrix(matrix_string[MSB], 5);
      draw_symbol_on_matrix(matrix_string[LSB], 9);
    } else if (matrix_string[MSB] != '-') {
      draw_symbol_on_matrix(matrix_string[MSB], 4);

      // "cKg" перегруз
      if (matrix_string[MSB] == 'K' && matrix_string[LSB] == 'g') {
        draw_symbol_on_matrix(matrix_string[LSB], 10);
      } else {
        draw_symbol_on_matrix(matrix_string[LSB], 9);
      }
    }

    if (matrix_string[MSB] == '-' && matrix_string[LSB] != '-') {
      draw_symbol_on_matrix(matrix_string[MSB], 4);
      draw_symbol_on_matrix(matrix_string[LSB], 8);
    }

    // "c--" interface is not connected
    if (matrix_string[MSB] == '-' && matrix_string[LSB] == '-') {
      draw_symbol_on_matrix(matrix_string[MSB], 9);
      draw_symbol_on_matrix(matrix_string[LSB], 4);
    }
  } else if (matrix_string[DIRECTION] == '>' ||
             matrix_string[DIRECTION] == '<' ||
             matrix_string[DIRECTION] == '+' ||
             matrix_string[DIRECTION] == '-' ||
             matrix_string[DIRECTION] == 'p') { // in moving up/down: >10 or >1c

    if (matrix_string[DIRECTION] == '-' || matrix_string[DIRECTION] == 'p') {
      draw_symbol_on_matrix(matrix_string[DIRECTION], 1);
    } else {
      draw_symbol_on_matrix(matrix_string[DIRECTION], 0);
    }

    draw_symbol_on_matrix(matrix_string[MSB], 6);

    // font_width = 3 for '1' and '-'
    if (matrix_string[MSB] == '1' || matrix_string[MSB] == '-') {
      draw_symbol_on_matrix(matrix_string[LSB], 10);
    } else if (matrix_string[MSB] != '-') {
      draw_symbol_on_matrix(matrix_string[LSB], 11);
    }
  }
}

symbol_e char_to_symbol(char ch) {
  switch (ch) {
  case '0':
    return SYMBOL_0;
  case '1':
    return SYMBOL_1;
  case '2':
    return SYMBOL_2;
  case '3':
    return SYMBOL_3;
  case '4':
    return SYMBOL_4;
  case '5':
    return SYMBOL_5;
  case '6':
    return SYMBOL_6;
  case '7':
    return SYMBOL_7;
  case '8':
    return SYMBOL_8;
  case '9':
    return SYMBOL_9;
  case 'S':
    return SYMBOL_S;
  case 'I':
    return SYMBOL_I;
  case 'D':
    return SYMBOL_D;
  case '-':
    return SYMBOL_MINUS;
  case ' ':
    return SYMBOL_EMPTY;
  case '.':
    return SYMBOL_DOT;
  case 'c':
    return SYMBOL_EMPTY;
  case 'V':
    return SYMBOL_V;
  case 'L':
    return SYMBOL_L;
  case 'C':
    return SYMBOL_C;
  case 'E':
    return SYMBOL_E;
  // Добавь другие символы по необходимости
  default:
    return SYMBOL_EMPTY;
  }
}

void draw_string(char *matrix_string) {
  set_symbols(char_to_symbol(matrix_string[0]),
              char_to_symbol(matrix_string[1]),
              char_to_symbol(matrix_string[2]));
  draw_symbols();
}

extern volatile bool is_time_ms_for_display_str_elapsed;
/**
 * @brief  Отображение символов на матрице в течение
 *         TIME_DISPLAY_STRING_DURING_MS (определено в tim.c)
 * @note   Для DEMO_MODE и для протоколов при запуске индикатора.
 * @param  matrix_string: Указатель на строку, которая будет отображаться.
 * @retval None
 */
void display_string_during_ms(char *matrix_string) {
  is_time_ms_for_display_str_elapsed = false;

  while (!is_time_ms_for_display_str_elapsed) {
    draw_string(matrix_string);
  }

  set_symbols(SYMBOL_EMPTY, SYMBOL_EMPTY, SYMBOL_EMPTY);
  draw_symbols();
}
