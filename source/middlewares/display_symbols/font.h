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
