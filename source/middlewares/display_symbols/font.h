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

  SYMBOL_ARROW_ANIMATION_UP_DYNAMIC,
  SYMBOL_ARROW_ANIMATION_DOWN_DYNAMIC,

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

// #define NUMBER_OF_SYMBOLS SYMBOLS_NUMBER
// #define NUMBER_OF_ROWS 6
// #define NUMBER_OF_DRIVERS 3
// #define ELEMENTS_IN_BITMAP \
//   NUMBER_OF_ROWS // количество элементов в bitmap'e равно количеству рядов

// extern const uint8_t bitmap[NUMBER_OF_SYMBOLS][NUMBER_OF_ROWS];

#endif /*__FONT_H__ */
