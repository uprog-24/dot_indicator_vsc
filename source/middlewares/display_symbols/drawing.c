/**
 * @file drawing.c
 */
#include "drawing.h"

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
 * @brief  Convert integer number (0..9) to char ('0'..'9')
 * @param  number: Number that will be converted into char
 * @retval char:   Symbol 'number' if number is correct else symbol 'e'
 */
char convert_int_to_char(uint8_t number) {
  if (number <= 9) {
    return number + '0';
  }
  return 'e';
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

/**
 * @brief  Set direction symbol in matrix_string by index = DIRECTION
 * @param  matrix_string: Pointer to the matrix_string that will be displayed on
 *                        matrix
 * @param  direction:     Direction with directionType:
 *                        DIRECTION_UP/DIRECTION_DOWN/NO_DIRECTION
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
 * @brief  Set floor symbols in matrix_string by indexes MSB and LSB
 * @param  matrix_string:                Pointer to the matrix_string that will
 *                                       be displayed on matrix
 * @param  floor:                        Floor
 * @param  max_positive_number_location: Maximum used number of positive floor
 * @param  code_location_symbols:        Pointer to the buffer with code
 *                                       location and it's symbols
 * @param  spec_symbols_buff_size:       Number of special symbols
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
    // special symbols
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
 * @brief  Setting string with symbols of floor and direction
 * @param  matrix_string:                 Pointer to the matrix_string that will
 *                                        be displayed on matrix
 * @param  drawing_data:                  Pointer to the structure with current
 *                                        floor and direction
 * @param  max_positive_number_location:  Max positive number of location of
 *                                        used protocol
 * @param  special_symbols_code_location: Pointer to the buffer with code
 *                                        location and it's symbols
 * @param  spec_symbols_buff_size:        Number of special symbols for
 *                                        code_location
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
  uint8_t *cur_symbol_code = NULL;

  cur_symbol_code = get_symbol_code(symbol);

  if (cur_symbol_code == NULL) {
    return;
  }

  for (uint8_t current_row = 0; current_row < ROWS; current_row++) {
    for (uint8_t row = 0; row < ROWS; row++) {
      if (row == current_row) {
        set_row_state(current_row, TURN_ON);
      } else {
        set_row_state(row, TURN_OFF);
      }
    }

    uint8_t binary_symbol_code_row[BINARY_SYMBOL_SIZE];
    uint8_t num_bit = BINARY_SYMBOL_CODE_SIZE;

    if (current_row + shift < ROWS) {
      convert_number_from_dec_to_bin(cur_symbol_code[current_row + shift],
                                     binary_symbol_code_row,
                                     BINARY_SYMBOL_CODE_SIZE);
    } else {
      memset(binary_symbol_code_row, 0, BINARY_SYMBOL_SIZE);
    }

    uint8_t start_col = start_pos;
    if (start_col <= MAX_POSITION_COLUMN) {
      uint8_t end_col = start_col + FONT_WIDTH;
      for (uint8_t col = start_col; col < end_col; col++) {
        /* from index 6 to 6 - FONT_WIDTH = 6 - 5 = 1. Symbols from symbols[] in
         font.c */
        if (binary_symbol_code_row[num_bit] == 1) {
          set_col_state(col, TURN_ON);
        } else {
          set_col_state(col, TURN_OFF);
        }
        num_bit--;
      }
    }
    // set_all_cols_state(TURN_OFF);
    set_all_rows_state(TURN_OFF);
    set_all_cols_state(TURN_OFF);
    // set_full_matrix_state(TURN_OFF);
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
      draw_symbol_on_matrix(matrix_string[LSB], 9, 0);
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
