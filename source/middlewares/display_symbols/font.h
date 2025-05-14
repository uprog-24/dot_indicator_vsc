/**
 * @file    font.h
 * @brief   Этот файл содержит прототипы функций для файла font.c
 */
#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

#define BINARY_SYMBOL_SIZE 8 ///< 8 бит в строке символа

/**
 * @brief  Преобразование десятичного числа в двоичный массив (для получения
 *         двоичных строк символа в drawing.c).
 * @param  number:     Число для двоичной строки символа.
 * @param  binary_mas: Указатель на массив с двоичным представлением строки
 *                     символа (из font.c symbols[]).
 * @param  bin_size:   Размер двоичной строки символа.
 * @retval None
 */
void convert_number_from_dec_to_bin(uint8_t number, uint8_t *binary_mas,
                                    uint8_t bin_size);

/**
 * @brief  Получение кода символа из массива symbols[].
 * @param  symbol:                 Символ из symbols[] (font.c).
 * @retval Указатель на buff_code: Код символа.
 */
uint8_t *get_symbol_code(char symbol);

/// @brief symbol_e - перечисление поддерживаемых символов
typedef enum {
  SYMBOL_ZERO = 0,
  SYMBOL_ONE = 1,
  SYMBOL_TWO = 2,
  SYMBOL_THREE = 3,
  SYMBOL_FOUR = 4,
  SYMBOL_FIVE = 5,
  SYMBOL_SIX = 6,
  SYMBOL_SEVEN = 7,
  SYMBOL_EIGHT = 8,
  SYMBOL_NINE = 9,

  SYMBOL_ARROW_UP = 10,
  SYMBOL_ARROW_DOWN = 11,

  SYMBOL_A = 12,                       // Символ A
  SYMBOL_b = 13,                       // Символ b
  SYMBOL_C = 14,                       // Символ C
  SYMBOL_d = 15,                       // Символ d
  SYMBOL_E = 16,                       // Символ E
  SYMBOL_F = 17,                       // Символ F
  SYMBOL_EMPTY = 18,                   // Символ Пробел
  SYMBOL_UNDERGROUND_FLOOR_BIG = 19,   // Символ П
  SYMBOL_P = 20,                       // Символ P
  SYMBOL_UNDERGROUND_FLOOR_SMALL = 21, // Символ п
  SYMBOL_H = 22,                       // Символ H
  SYMBOL_U_BIG = 23,                   // Символ U
  SYMBOL_MINUS = 24,                   // Символ -
  SYMBOL_UNDERSCORE = 25,              // Символ _
  SYMBOL_U_SMALL = 26,                 // Символ u
  SYMBOL_L = 27,                       // Символ L
  SYMBOL_Y_RU = 28,                    // Символ У
  SYMBOL_B_RU = 29,                    // Символ Б
  SYMBOL_G_RU = 30,                    // Символ Г
  SYMBOL_R = 31,                       // Символ R
  SYMBOL_V = 32,                       // Символ V
  SYMBOL_N = 33,                       // Символ N
  SYMBOL_S = 34,                       // Символ S
  SYMBOL_K = 35,                       // Символ K
  SYMBOL_Y = 36,                       // Символ Y
  SYMBOL_G = 37,                       // Символ G
  SYMBOL_B = 38,                       // Символ B
  SYMBOL_T = 39,                       // Символ T
  SYMBOLS_NUMBER
} symbol_e;

#define NUMBER_OF_SYMBOLS 40
#define NUMBER_OF_ROWS 8

extern const uint8_t bitmap[NUMBER_OF_SYMBOLS][NUMBER_OF_ROWS];

#endif /*__FONT_H__ */
