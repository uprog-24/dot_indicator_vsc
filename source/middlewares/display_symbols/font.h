
#if 0
/**
 * @file    font.h
 * @brief   This file contains all the function prototypes for
 *          the font.c file
 */
#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

#define FONT_WIDTH 5U ///< Width of font

#if DOT_PIN
#define BINARY_SYMBOL_SIZE                                                     \
  8 ///< 8 rows massive for binary representation of symbol

#elif DOT_SPI
#define BINARY_SYMBOL_SIZE                                                     \
  6 ///< 6 rows massive for binary representation of symbol

#endif
/**
 * @brief  Convert decimal number to binary
 * @param  number:     Decimal number (code of symbol)
 * @param  binary_mas: Pointer to the binary representation of the symbol
 * @retval None
 */
void convert_number_from_dec_to_bin(uint8_t number, uint8_t *binary_mas,
                                    uint8_t bin_size);

/**
 * @brief  Get code of the symbol from buffer symbols[]
 * @param  symbol:               Symbol from buffer symbols[] (font.c)
 * @retval Pointer to buff_code: Code of symbol
 */
uint8_t *get_symbol_code(char symbol);

#endif /*__FONT_H__ */

#endif

/**
 * @file    font.h
 */
#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

/// @brief symbol_code_e - перечисление поддерживаемых символов
typedef enum {
  SYMBOL_0 = 0,
  SYMBOL_1,
  SYMBOL_2,
  SYMBOL_3,
  SYMBOL_4,
  SYMBOL_5,
  SYMBOL_6,
  SYMBOL_7,
  SYMBOL_8,
  SYMBOL_9,

  SYMBOL_ARROW_UP,
  SYMBOL_ARROW_DOWN,
  SYMBOL_ARROW_BOTH,

  SYMBOL_A,       // Символ A
  SYMBOL_B,       // Символ B
  SYMBOL_b,       // Символ b
  SYMBOL_C,       // Символ C
  SYMBOL_D,       // Символ D
  SYMBOL_d,       // Символ d
  SYMBOL_E,       // Символ E
  SYMBOL_F,       // Символ F
  SYMBOL_G,       // Символ G
  SYMBOL_H,       // Символ H
  SYMBOL_I,       // Символ I
  SYMBOL_K,       // Символ K
  SYMBOL_L,       // Символ L
  SYMBOL_N,       // Символ N
  SYMBOL_P,       // Символ P
  SYMBOL_R,       // Символ R
  SYMBOL_S,       // Символ S
  SYMBOL_T,       // Символ T
  SYMBOL_U_BIG,   // Символ U
  SYMBOL_U_SMALL, // Символ u
  SYMBOL_V,       // Символ V
  SYMBOL_Y,       // Символ Y

  SYMBOL_b_BIG_RU,                // Символ Б
  SYMBOL_G_RU,                    // Символ Г
  SYMBOL_UNDERGROUND_FLOOR_BIG,   // Символ П
  SYMBOL_UNDERGROUND_FLOOR_SMALL, // Символ п
  SYMBOL_Y_RU,                    // Символ У

  SYMBOL_EMPTY,      // Символ Пробел
  SYMBOL_MINUS,      // Символ -
  SYMBOL_UNDERSCORE, // Символ _
  SYMBOL_PLUS,       // Символ +
  SYMBOL_DOT,        // Символ .
  SYMBOL_ALL_ON, // Символ Включить все точки (для DEMO_MODE)

  SYMBOLS_NUMBER
} symbol_code_e;

#define NUMBER_OF_SYMBOLS SYMBOLS_NUMBER
#define NUMBER_OF_ROWS 6

extern const uint8_t bitmap[NUMBER_OF_SYMBOLS][NUMBER_OF_ROWS];

#endif /*__FONT_H__ */
