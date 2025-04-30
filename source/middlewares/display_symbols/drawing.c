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
 * @brief Преобразование числа в символ (для этажей 0..9).
 *
 * @param number: номер этажа 0..9.
 * @return char:  символ '0'..'9'.
 */
char convert_int_to_char(uint8_t number) {
  if (number <= 9) {
    return number + '0';
  }
  return 'e';
}

/**
 * @brief Установка значений структуры drawing_data_t.
 * @param  drawing_data: Указатель на структуру.
 * @param  floor:         Этаж (код).
 * @param  direction:     Направление движения.
 * @retval None
 */
void drawing_data_setter(drawing_data_t *drawing_data, uint16_t floor,
                         directionType direction) {
  drawing_data->floor = floor;
  drawing_data->direction = direction;
}

/**
 * @brief  Установка символа направления в matrix_string по индексу = DIRECTION.
 * @param  matrix_string: Указатель на строку.
 * @param  direction:     Направление движения directionType:
 *                        DIRECTION_UP/DIRECTION_DOWN/NO_DIRECTION.
 * @retval None
 */
void set_direction_symbol(char *matrix_string, directionType direction) {
  switch (direction) {
  case DIRECTION_UP:
    matrix_string[DIRECTION] = '>';
    break;
  case DIRECTION_DOWN:
    matrix_string[DIRECTION] = '<';
    break;
  case NO_DIRECTION:
    matrix_string[DIRECTION] = 'c';
    break;
  }
}

/**
 * @brief  Установка символов для этажей в matrix_string по индексам MSB и LSB.
 * @param  matrix_string:                Указатель на строку для отображения.
 * @param  floor:                        Этаж (код).
 * @param  max_positive_number_location: Максимвльное значение для
 *                                       положительного номера этажа в
 *                                       протоколе.
 * @param  code_location_symbols:        Указатель на буфер с символами для
 *                                       спец. режимов/символов протокола.
 * @param  spec_symbols_buff_size:       Кол-во элементов в буфере
 *                                       special_symbols_code_location.
 * @retval None
 */
void set_floor_symbols(char *matrix_string, uint16_t floor,
                       uint8_t max_positive_number_location,
                       const code_location_symbols_t *code_location_symbols,
                       uint8_t spec_symbols_buff_size) {
  if (floor <= 9) {
    matrix_string[MSB] = convert_int_to_char(floor % 10);
    matrix_string[LSB] = 'c';
  } else if (floor <= max_positive_number_location) {
    matrix_string[MSB] = convert_int_to_char(floor / 10);
    matrix_string[LSB] = convert_int_to_char(floor % 10);
  } else {
    // Спец. символы
    for (uint8_t ind = 0; ind < spec_symbols_buff_size; ind++) {
      if (code_location_symbols[ind].code_location == floor) {
        matrix_string[MSB] = code_location_symbols[ind].symbols[0];
        matrix_string[LSB] = strlen(code_location_symbols[ind].symbols) == 1
                                 ? 'c'
                                 : code_location_symbols[ind].symbols[1];
      }
    }
  }
}

/**
 * @brief  Установка строки для отображения.
 * @param  matrix_string:                 Указатель на строку.
 * @param  drawing_data:                  Указатель на структуру с этажом и
 *                                        направлением движения.
 * @param  max_positive_number_location:  Максимвльное значение для
 *                                        положительного номера этажа в
 *                                        протоколе.
 * @param  special_symbols_code_location: Указатель на буфер с символами для
 *                                        спец. режимов/символов протокола.
 * @param  spec_symbols_buff_size:        Кол-во элементов в буфере
 *                                        special_symbols_code_location.
 * @retval None
 */
void setting_symbols(
    char *matrix_string, const drawing_data_t *const drawing_data,
    uint8_t max_positive_number_location,
    const code_location_symbols_t *special_symbols_code_location,
    uint8_t spec_symbols_buff_size) {
  set_direction_symbol(matrix_string, drawing_data->direction);
  set_floor_symbols(matrix_string, drawing_data->floor,
                    max_positive_number_location, special_symbols_code_location,
                    spec_symbols_buff_size);
}

/// Флаг для удержания состояния строки в темение 1 мс (максимвльная яркость,
/// частота обновления матрицы 125 Гц).
extern volatile bool is_tim4_period_elapsed;
/**
 * @brief  Отображение символа на матрице.
 * @note   Построчно проходим по коду символа, удерживая состояние строки 1 мс
 *         (максимвльная яркость, частота обновления матрицы 125 Гц).
 * @param  symbol:    Символ для отображения (из font.c).
 * @param  start_pos: Начальная позиция (индекс столбца) для символа.
 * @param  shift:     Сдвиг по Y для анимации. НЕ используется (ВСЕГДА 0).
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

  // Включаем колонку, если бит в строке символа = 1
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

/**
 * @brief  Отображение спец. строки (с начальным спец. символом -
 *         matrix_string[DIRECTION]: 'c','>', '<', '+').
 * @param  matrix_string: Указатель на строку.
 * @retval None
 */
static void draw_special_symbols(char *matrix_string) {
  // stop floor 1..9: "c1c"
  if (matrix_string[DIRECTION] == 'c' && matrix_string[LSB] == 'c') {
    draw_symbol_on_matrix(matrix_string[DIRECTION], 0, 0);
    draw_symbol_on_matrix(matrix_string[LSB], 0, 0);
    draw_symbol_on_matrix(matrix_string[MSB], 6, 0);

  } else if (matrix_string[DIRECTION] ==
             'c') { // stop floor "c10".."c99" and "c-1".."c-9"

    // draw DIRECTION symbol
    draw_symbol_on_matrix(matrix_string[DIRECTION], 0, 0);

    // draw MSB and LSB symbols
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
 * @brief Тестовая версия для ЩЛЗ (НО используется первая версия выше).
 * Этажи 1-9 справа, точки (светодиоды):
 *  0..4: стрелка;
 *  5,6: пробел;
 *  7..10: первый символ;
 *  11: пробел;
 *  12..15: второй символ. Для "1" (ширина 3 точки) сдвиг по правой части.
 */
#if 0
static void draw_special_symbols(char *matrix_string) {
  // stop floor 1..9: c1c
  if (matrix_string[DIRECTION] == 'c' && matrix_string[LSB] == 'c') {
    draw_symbol_on_matrix(matrix_string[DIRECTION], 0, 0);
    draw_symbol_on_matrix(matrix_string[LSB], 0, 0);
    draw_symbol_on_matrix(matrix_string[MSB], 6, 0);

  } else if (matrix_string[DIRECTION] ==
             'c') { // stop floor c10..c99 and c-1..c-9

    // draw DIRECTION symbol
    draw_symbol_on_matrix(matrix_string[DIRECTION], 0, 0);

    // draw MSB and LSB symbols
    if (matrix_string[MSB] == '1' || matrix_string[MSB] == 'I') {
      draw_symbol_on_matrix(matrix_string[MSB], 5, 0);
      draw_symbol_on_matrix(matrix_string[LSB], 9, 0);
    } else if (matrix_string[MSB] != '-') {
      draw_symbol_on_matrix(matrix_string[MSB], 4, 0);
      // draw_symbol_on_matrix(matrix_string[LSB], 9, 0);
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
    //========== MSB
    // >1c
    if (matrix_string[MSB] == '1' && matrix_string[LSB] == 'c') {
      draw_symbol_on_matrix(matrix_string[MSB], 13, 0);
    }

    // >2c ... >9c
    if (matrix_string[MSB] != '1' && matrix_string[LSB] == 'c') {
      draw_symbol_on_matrix(matrix_string[MSB], 12, 0);
    }

    // >10 ... >19
    if (matrix_string[MSB] == '1' && matrix_string[LSB] != 'c') {
      draw_symbol_on_matrix(matrix_string[MSB], 8, 0);
    }

    // >-1 ... >-9
    if (matrix_string[MSB] == '-') {
      draw_symbol_on_matrix(matrix_string[MSB], 8, 0);
    }

    // >20 ... >99
    if (matrix_string[MSB] != '1' && matrix_string[MSB] != '-' &&
        matrix_string[LSB] != 'c') {
      draw_symbol_on_matrix(matrix_string[MSB], 7, 0);
    }
    //========== MSB

    //========== LSB
    // font_width = 3 for '1' and '-'
    if (matrix_string[LSB] == '1') {
      draw_symbol_on_matrix(matrix_string[LSB], 13, 0);
    } else {
      draw_symbol_on_matrix(matrix_string[LSB], 12, 0);
    }

#if 0
    if (matrix_string[MSB] == '1' || matrix_string[MSB] == '-') {
      if (matrix_string[LSB] == '1') {
        draw_symbol_on_matrix(matrix_string[LSB], 13, 0);
      } else {
        draw_symbol_on_matrix(matrix_string[LSB], 12, 0);
      }

    } else if (matrix_string[MSB] != '-') {
      draw_symbol_on_matrix(matrix_string[LSB], 12, 0);
    }
#endif
    //========== LSB
  }
}
#endif

/**
 * @brief  Отображение matrix_string в зависимости от типа строки.
 * @param  matrix_string: Указатель на строку для отображения.
 * @retval None
 */
void draw_string_on_matrix(char *matrix_string) {

  if (is_start_symbol_special(matrix_string)) {
    draw_special_symbols(matrix_string);
  } else {
    draw_symbols(matrix_string);
  }
}

extern volatile bool is_time_ms_for_display_str_elapsed;
/**
 * @brief  Отображение символов на матрице в течение
 *         TIME_DISPLAY_STRING_DURING_MS (определено в tim.c)
 * @note   Для DEMO_MODE и для протоколов при запуске индикатора.
 * @param  matrix_string: Указатель на строку, которая будет отображаться.
 * @retval None
 */
void display_symbols_during_ms(char *matrix_string) {
  is_time_ms_for_display_str_elapsed = false;

  while (!is_time_ms_for_display_str_elapsed) {
    draw_string_on_matrix(matrix_string);
  }
}
