/**
 * @file    drawing.h
 * @brief   Этот файл содержит прототипы функций для файла drawing.c
 */
#ifndef __DRAWING_H__
#define __DRAWING_H__

#include <stdbool.h>
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

/**
 * Структура содержит код местоположения и соответствующую ему строку для
 * отображения (для протоколов).
 */
typedef struct {
  uint16_t code_location;
  char symbols[3];
} code_location_symbols_t;

/**
 * @brief Заполнение структуры: битмап и ширина символа (вызывается в main.c
 *        перед отрисовкой!!!)
 */
void init_symbols_width();

/**
 * @brief  Установка символа направления движения
 * @param  direction_code: Код направления (из перечисления symbol_code_e)
 */
void set_direction_symbol(symbol_code_e direction_code);

/**
 * @brief  Установка символов для этажей
 * @param  symbol_center_code: Код центрального символа
 * @param  symbol_right_code:  Код правого символа
 */
void set_floor_symbols(symbol_code_e symbol_center_code,
                       symbol_code_e symbol_right_code);

/**
 * @brief  Установка символов (направление + этаж)
 * @param  s1_code:  Код символа 1
 * @param  s2_code:  Код символа 2
 * @param  s3_code:  Код символа 3
 */
void set_symbols(symbol_code_e s1, symbol_code_e s2, symbol_code_e s3);

/**
 * @brief Отображение символов, заранее установленных в структуру symbols
 */
void draw_symbols();

/**
 * @brief Отображение строки
 * @param matrix_string: Указатель на строку, которая будет отображаться
 */
void draw_string(char *matrix_string);

/**
 * @brief  Отображение строки на матрице в течение
 *         TIME_DISPLAY_STRING_DURING_MS (определено в tim.c)
 * @note   Для DEMO_MODE и для протоколов при запуске индикатора
 * @param  matrix_string: Указатель на строку, которая будет отображаться
 */
void display_string_during_ms(char *matrix_string);

/**
 * @brief  Отображение строки на матрице в течение
 *         TIME_DISPLAY_STRING_DURING_MS (определено в tim.c)
 * @note   Для DEMO_MODE
 */
void display_symbols_during_ms();

/**
 * @brief Сброс состояния символов дисплея для завершения анимации
 *
 */
void stop_animation();

#endif /*__ DRAWING_H__ */
