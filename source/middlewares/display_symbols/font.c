/**
 * @file font.c
 */
#include "font.h"

#include <stdio.h>
#include <string.h>

#define START_BIT_INDEX_FONT 6 ///< Start index for element of symbols[]

/**
 * Stores the parameters of the symbol: symbol and binary_symbol massive
 */
typedef struct {
  char symbol;
  uint8_t buff_code[BINARY_SYMBOL_SIZE];
} symbol_t;

/// Massive of symbols
static symbol_t symbols[] = {{'c',
                              {// clear
                               0b00000000, 0b00000000, 0b00000000, 0b00000000,
                               0b00000000, 0b00000000, 0b00000000, 0b00000000}},
                             {'>',
                              {// up
                               0b00010000, 0b00111000, 0b01010100, 0b00010000,
                               0b00010000, 0b00010000, 0b00010000, 0b00010000}},
                             {'<',
                              {// down
                               0b00010000, 0b00010000, 0b00010000, 0b00010000,
                               0b00010000, 0b01010100, 0b00111000, 0b00010000}},
                             {'0',
                              {0b00110000, 0b01001000, 0b01001000, 0b01001000,
                               0b01001000, 0b01001000, 0b01001000, 0b00110000}},
                             {'1',
                              {0b00100000, 0b01100000, 0b00100000, 0b00100000,
                               0b00100000, 0b00100000, 0b00100000, 0b01110000}},
                             {'2',
                              {0b00110000, 0b01001000, 0b00001000, 0b00001000,
                               0b00010000, 0b00100000, 0b01000000, 0b01111000}},
                             {'3',
                              {0b01111000, 0b00001000, 0b00010000, 0b00110000,
                               0b00001000, 0b00001000, 0b01001000, 0b00110000}},
                             {'4',
                              {0b00001000, 0b00011000, 0b00101000, 0b01001000,
                               0b01111000, 0b00001000, 0b00001000, 0b00001000}},
                             {'5',
                              {0b01111000, 0b01000000, 0b01000000, 0b01110000,
                               0b00001000, 0b00001000, 0b01001000, 0b00110000}},
                             {'6',
                              {0b00110000, 0b01001000, 0b01000000, 0b01110000,
                               0b01001000, 0b01001000, 0b01001000, 0b00110000}},
                             {'7',
                              {0b01111000, 0b00001000, 0b00001000, 0b00010000,
                               0b00100000, 0b00100000, 0b00100000, 0b00100000}},
                             {'8',
                              {0b00110000, 0b01001000, 0b01001000, 0b00110000,
                               0b01001000, 0b01001000, 0b01001000, 0b00110000}},
                             {'9',
                              {0b00110000, 0b01001000, 0b01001000, 0b01001000,
                               0b00111000, 0b00001000, 0b01001000, 0b00110000}},
                             {'O',
                              {0b00000000, 0b00110000, 0b01001000, 0b01001000,
                               0b01001000, 0b01001000, 0b00110000, 0b00000000}},
                             {'K',
                              {0b01000100, 0b01001000, 0b01010000, 0b01100000,
                               0b01100000, 0b01010000, 0b01001000, 0b01000100}},
                             {'-',
                              {0b00000000, 0b00000000, 0b00000000, 0b01110000,
                               0b00000000, 0b00000000, 0b00000000, 0b00000000}},
                             {'b',
                              {0b01000000, 0b01000000, 0b01000000, 0b01110000,
                               0b01001000, 0b01001000, 0b01001000, 0b00110000}},
                             {'L',
                              {0b01000000, 0b01000000, 0b01000000, 0b01000000,
                               0b01000000, 0b01000000, 0b01001000, 0b01111000}},
                             {'A',
                              {0b00110000, 0b01001000, 0b01001000, 0b01111000,
                               0b01001000, 0b01001000, 0b01001000, 0b01001000}},
                             {'P',
                              {0b01110000, 0b01001000, 0b01001000, 0b01001000,
                               0b01110000, 0b01000000, 0b01000000, 0b01000000}},
                             {'H',
                              {0b01001000, 0b01001000, 0b01001000, 0b01111000,
                               0b01001000, 0b01001000, 0b01001000, 0b01001000}},
                             {'C',
                              {0b00110000, 0b01001000, 0b01000000, 0b01000000,
                               0b01000000, 0b01000000, 0b01001000, 0b00110000}},
                             {'E',
                              {0b01111000, 0b01000000, 0b01000000, 0b01000000,
                               0b01111000, 0b01000000, 0b01000000, 0b01111000}},
                             {'F',
                              {0b01111000, 0b01000000, 0b01000000, 0b01000000,
                               0b01110000, 0b01000000, 0b01000000, 0b01000000}},
                             {'U',
                              {0b01001000, 0b01001000, 0b01001000, 0b01001000,
                               0b01001000, 0b01001000, 0b01001000, 0b00110000}},
                             {'p', // П
                              {0b01111000, 0b01001000, 0b01001000, 0b01001000,
                               0b01001000, 0b01001000, 0b01001000, 0b01001000}},
                             {'I',
                              {0b01110000, 0b00100000, 0b00100000, 0b00100000,
                               0b00100000, 0b00100000, 0b00100000, 0b01110000}},
                             {'D',
                              {0b01110000, 0b01001000, 0b01001000, 0b01001000,
                               0b01001000, 0b01001000, 0b01001000, 0b01110000}},
                             {'S',
                              {0b00110000, 0b01001000, 0b01000000, 0b00100000,
                               0b00010000, 0b00001000, 0b01001000, 0b00110000}},
                             {'g',
                              {// Г
                               0b01111000, 0b01000000, 0b01000000, 0b01000000,
                               0b01000000, 0b01000000, 0b01000000, 0b01000000}},
                             {'+',
                              {0b00000000, 0b00010000, 0b00010000, 0b01111100,
                               0b00010000, 0b00010000, 0b00000000, 0b00000000}},
                             {'V',
                              {0b01000100, 0b01000100, 0b01000100, 0b01000100,
                               0b01000100, 0b01000100, 0b00101000, 0b00010000}},
                             {'T',
                              {0b01111100, 0b00010000, 0b00010000, 0b00010000,
                               0b00010000, 0b00010000, 0b00010000, 0b00010000}},
                             {'.',
                              {0b00000000, 0b00000000, 0b00000000, 0b00000000,
                               0b00000000, 0b00000000, 0b00000000, 0b01000000}},
                             {'%',
                              // {0b00000000, 0b00000000, 0b00000000,
                              // 0b01100100, 0b01101000, 0b00010000,
                              //  0b00101100, 0b01001100}

                              // !!!
                              //  {0b01010100, 0b01010100, 0b01010100,
                              //  0b01010100, 0b01010100, 0b01010100,
                              //   0b00000000, 0b01010100}

                              // ! большой
                              //  {0b00010000, 0b00111000, 0b00111000,
                              //  0b00111000, 0b00111000, 0b00010000,
                              //   0b00000000, 0b00010000}

                              // >
                              //  {0b00000000, 0b01000000, 0b00100000,
                              //  0b00010000, 0b00001000, 0b00010000,
                              //   0b00100000, 0b01000000}

                              //  {0b00010000, 0b00111000, 0b00111000,
                              //  0b00111000, 0b00010000, 0b00000000,
                              //   0b00010000, 0b00111000}

                              {0b00010000, 0b00010000, 0b00010000, 0b00010000,
                               0b00010000, 0b00010000, 0b00000000, 0b00010000}}

};

/**
 * @brief  Convert decimal number to binary
 * @param  number:     Decimal number (code of symbol)
 * @param  binary_mas: Pointer to the binary representation of the symbol
 * @retval None
 */
void convert_number_from_dec_to_bin(uint8_t number, uint8_t *binary_mas,
                                    uint8_t bin_size) {
  int8_t font_index = bin_size;
  uint8_t bit_num = 0;
  while (font_index != -1) {
    binary_mas[bit_num] = number % 2;
    number /= 2;
    font_index--;
    bit_num++;
  }
}

/**
 * @brief  Get code of the symbol from buffer symbols[]
 * @param  symbol:               Symbol from buffer symbols[] (font.c)
 * @retval Pointer to buff_code: Code of symbol
 */
uint8_t *get_symbol_code(char symbol) {
  for (uint8_t ind = 0; ind < sizeof(symbols) / sizeof(symbols[0]); ind++) {
    if (symbols[ind].symbol == symbol) {
      return symbols[ind].buff_code;
    }
  }
  return NULL;
}
