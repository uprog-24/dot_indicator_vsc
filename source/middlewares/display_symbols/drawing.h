/**
 * @file    drawing.h
 * @brief   This file contains all the function prototypes for
 *          the drawing.c file
 */
#ifndef __DRAWING_H__
#define __DRAWING_H__

#include <stdint.h>

/**
 * Stores common (for all protocols and modes defined in config.h) values of
 * direction of the movement. Each protocol has function
 * transform_direction_to_common that transform protocol's values of direction
 * to common directionType
 */
typedef enum { NO_DIRECTION, DIRECTION_UP, DIRECTION_DOWN } directionType;

/**
 * Stores floor and direction of the movement
 */
typedef struct drawing_data {
  uint16_t floor;
  directionType direction;
} drawing_data_t;

/**
 * Stores code location and it's symbols
 */
typedef struct {
  uint16_t code_location;
  char symbols[3];
} code_location_symbols_t;

/**
 * Stores indexes of string that will be displayed on matrix.
 * Direction has position 0;
 * MSB (Most Significant Bit) has position 1;
 * LSB (Least Significant Bit) has position 2.
 */
enum { DIRECTION = 0, MSB = 1, LSB = 2 };

/**
 * @brief  Setting structure with type drawing_data_t
 * @param  drawing_data: Pointer to the structure with type drawing_data_t
 * @param  floor:         Floor
 * @param  direction:     Direction with type directionType
 * @retval None
 */
void drawing_data_setter(drawing_data_t *drawing_data, uint8_t floor,
                         directionType direction);

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
void setting_symbols(char *matrix_string,
                     const drawing_data_t *const drawing_data,
                     uint8_t max_positive_number_location,
                     const code_location_symbols_t *code_location_symbols,
                     uint8_t spec_symbols_buff_size);

/**
 * @brief  Draw matrix_string on matrix depend on the type of matrix_string
 * @param  matrix_string: Pointer to the matrix_string that will be displayed on
 *                        matrix
 * @retval None
 */
void draw_string_on_matrix(char *str_symbols);

void display_symbols_spi(char *matrix_string);

char convert_int_to_char(uint8_t number);

/**
 * @brief  Display symbols on matrix (DEMO_MODE)
 * @param  time_ms:     The time (ms) during which the symbols will be displayed
 * @param  str_symbols: Pointer to the string to be displayed
 * @retval None
 */
void TIM4_Diaplay_symbols_on_matrix(uint16_t time_ms, char *str_symbols);

#endif /*__ DRAWING_H__ */
