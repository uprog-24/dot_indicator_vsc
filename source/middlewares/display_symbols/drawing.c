/**
 * @file drawing.c
 */
#include "drawing.h"

#include "config.h"
#include "dot.h"
#include "font.h"

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

#if 0

//=====================================================INDICATION

struct floor_indication {
  symbol_code_e t_elem;
  symbol_code_e l_elem;
  symbol_code_e r_elem;
  // enum arrow_panel_status		arrow_status;
  // uint32_t		 			afterstop_arrow_stamp;
  // // для организации задержки при смене стрелки со статической на ARROWS_NONE
};
// #####################################################################
// VERIABLES
// #####################################################################
static struct floor_indication floor_displaying = {
    .t_elem = SYMBOL_EMPTY, .l_elem = SYMBOL_EMPTY, .r_elem = SYMBOL_EMPTY,
    // .arrow_status = AS_CLEAR_ARROW_PANEL,
    // .afterstop_arrow_stamp = 0
};

static _Bool set_element(struct led_panel_element *element,
                         symbol_code_e symbol) {
  // если для элемента уже воспроизводится анимация, останавливаем ее
  // if (element->animation.current_transition != TRANSITION_NON) {
  //   if (element->animation.animation_type == ANIMATION_VERTICAL_CONTINOUS)
  //     stop_animation(element);
  //   else
  //     return 0;
  // }

  // смена символа без анимации
  // if (change_type == TRANSITION_NON) {
  element->element_bitmap_ptr = (uint8_t *)bitmap[symbol];

  // смена символа с анимацией
  // } else {
  //   if (!start_animation(element, symbol, change_type)) return 0;
  // }

  element->is_update = 1;
  return 1;
}

static _Bool update_floor(struct floor_indication *fdisplaying_p,
                          symbol_code_e l_elem, symbol_code_e r_elem) {

#if defined(config_SPLIT_SYMBOL)
  if (l_elem == SYMBOL_EMPTY) {
    return set_bottom_block(r_elem);
  }
#endif
  if (fdisplaying_p->l_elem != l_elem)
    if (set_bottom_left_element(l_elem) == 0)
      return 0;
  if (set_bottom_right_element(r_elem) == 0)
    return 0;
  return 1;
}

_Bool indication_set_static_arrow(symbol_code_e arrow_elem) {

  /* некорректные входные значения */
  if (arrow_elem > SYMBOLS_NUMBER)
    return 0;

  /* уже установлено */
  if ((floor_displaying.t_elem == arrow_elem))
    return 1;

  // if (state_get_state() == STATE_DISPLAYING_FLOOR)
  if (set_top_element(arrow_elem) == 0)
    return 0;
  return 1;
}

_Bool indication_clear_arrow() {
  const symbol_code_e symbol_clean_btmp = SYMBOL_EMPTY;

  // if (floor_displaying.t_elem == symbol_clean_btmp)
  //   return 1;
  if (set_top_element(symbol_clean_btmp) == 0)
    return 0;
  floor_displaying.t_elem = symbol_clean_btmp;
  return 1;
}

_Bool indication_set_floor(symbol_code_e l_elem, symbol_code_e r_elem) {

  /* некорректные входные значения */
  if ((l_elem > SYMBOLS_NUMBER) || (r_elem > SYMBOLS_NUMBER))
    return 0;
  /* аналогичные значения уже установлены */
  if ((l_elem == floor_displaying.l_elem) &&
      (r_elem == floor_displaying.r_elem))
    return 1;
  // if(state_get_state() == STATE_DISPLAYING_FLOOR) {
  if (update_floor(&floor_displaying, l_elem, r_elem) == 0)
    return 0;
  // }
  floor_displaying.l_elem = l_elem;
  floor_displaying.r_elem = r_elem;
  return 1;
}

_Bool indication_set_full_panel(symbol_code_e top_elem, symbol_code_e left_elem,
                                symbol_code_e right_elem) {
  indication_set_static_arrow(top_elem);
  indication_set_floor(left_elem, right_elem);
}
#endif

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
#endif
  indication_set_static_arrow(char_to_symbol(matrix_string[0]), 0);
  indication_set_floor(char_to_symbol(matrix_string[1]),
                       char_to_symbol(matrix_string[2]), 0);

  update_LED_panel();
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
  indication_clear_all_panels();
}

/**
 * @brief  Отображение строки на матрице в течение
 *         TIME_DISPLAY_STRING_DURING_MS (определено в tim.c)
 * @note   Для DEMO_MODE
 */
void display_symbols_during_ms() {
  is_time_ms_for_display_str_elapsed = false;

  while (!is_time_ms_for_display_str_elapsed) {
    display_symbols_spi();
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