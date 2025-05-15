/**
 * @file    drawing.h
 * @brief   Этот файл содержит прототипы функций для файла drawing.c
 */
#ifndef __DRAWING_H__
#define __DRAWING_H__

#include <stdint.h>

#include "font.h"

/**
 * Общие значения направления движения (для всех протоколов и режимов,
 * определённых в config.h). У каждого протокола есть функция
 * transform_direction_to_common, которая преобразует значения направления
 * протокола в общий тип directionType.
 */
typedef enum { NO_DIRECTION, DIRECTION_UP, DIRECTION_DOWN } directionType;

/**
 * Структура содержит этаж (код) и направление движения (для протоколов).
 */
typedef struct drawing_data {
  uint16_t floor;
  directionType direction;
} drawing_data_t;

typedef struct {
  symbol_e symbol_code_1;
  symbol_e symbol_code_2;
  symbol_e symbol_code_3;
} displayed_symbols_t;

/**
 * Структура содержит код местоположения и соответствующую ему строку для
 * отображения (для протоколов).
 */
typedef struct {
  uint16_t code_location;
  char symbols[3];
} code_location_symbols_t;



void set_symbols(symbol_e s1, symbol_e s2, symbol_e s3);

void draw_symbols();

void draw_string();

void set_direction_symbol(symbol_e direction_code);

void set_floor_symbols(symbol_e left_symbol_code, symbol_e right_symbol_code);

/**
 * @brief  Отображение символов на матрице в течение
 *         TIME_DISPLAY_STRING_DURING_MS (определено в tim.c)
 * @note   Для DEMO_MODE и для протоколов при запуске индикатора.
 * @param  matrix_string: Указатель на строку, которая будет отображаться.
 * @retval None
 */
void display_string_during_ms(char *matrix_string);

#endif /*__ DRAWING_H__ */
