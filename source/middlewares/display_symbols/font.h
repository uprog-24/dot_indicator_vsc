/**
 * @file    font.h
 */
#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

/// @brief symbol_e - перечисление поддерживаемых символов
typedef enum {
  SYMBOL_0 = 0,
  SYMBOL_1 = 1,
  SYMBOL_2 = 2,
  SYMBOL_3 = 3,
  SYMBOL_4 = 4,
  SYMBOL_5 = 5,
  SYMBOL_6 = 6,
  SYMBOL_7 = 7,
  SYMBOL_8 = 8,
  SYMBOL_9 = 9,

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
  SYMBOL_I = 40,                       // Символ I
  SYMBOL_D = 41,                       // Символ D
  SYMBOL_PLUS = 42,                    // Символ +
  SYMBOL_DOT = 43,                     // Символ .
  SYMBOL_ALL_ON = 44, // Символ Включить все точки (для DEMO_MODE)

  SYMBOLS_NUMBER
} symbol_e;

#define NUMBER_OF_SYMBOLS SYMBOLS_NUMBER
#define NUMBER_OF_ROWS 8

extern const uint8_t bitmap[NUMBER_OF_SYMBOLS][NUMBER_OF_ROWS];

#endif /*__FONT_H__ */
