/**
 * @file    drawing.h
 * @brief   Этот файл содержит прототипы функций для файла drawing.c
 */
#ifndef __DRAWING_H__
#define __DRAWING_H__

#include <stdint.h>

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
 * Индексы строки, которая будет отображаться на матрице.
 * Направление имеет позицию 0;
 * MSB (старший бит, первый символ) имеет позицию 1;
 * LSB (младший бит, второй символ) имеет позицию 2.
 */
enum { DIRECTION = 0, MSB = 1, LSB = 2 };

/**
 * @brief Установка значений структуры drawing_data_t.
 * @param  drawing_data: Указатель на структуру.
 * @param  floor:         Этаж (код).
 * @param  direction:     Направление движения.
 * @retval None
 */
void drawing_data_setter(drawing_data_t *drawing_data, uint16_t floor,
                         directionType direction);

/**
 * @brief  Установка строки для отображения.
 * @param  matrix_string:                 Указатель на строку.
 * @param  drawing_data:                  Указатель на структуру с этажом и
 *                                        направлением движения.
 * @param  max_positive_number_location:  Максимвльное значение для
 *                                        положительного номера этажа в
 *                                        протоколе.
 * @param  special_symbols_code_location: Указатель на буфер с символами для
 *                                        спец. режимов/символов протокола.
 * @param  spec_symbols_buff_size:        Кол-во элементов в буфере
 *                                        special_symbols_code_location.
 * @retval None
 */
void setting_symbols(char *matrix_string,
                     const drawing_data_t *const drawing_data,
                     uint8_t max_positive_number_location,
                     const code_location_symbols_t *code_location_symbols,
                     uint8_t spec_symbols_buff_size);

/**
 * @brief  Отображение matrix_string в зависимости от типа строки.
 * @param  matrix_string: Указатель на строку для отображения.
 * @retval None
 */
void draw_string_on_matrix(char *matrix_string);

/**
 * @brief Преобразование числа в символ (для этажей 0..9).
 *
 * @param number: номер этажа 0..9.
 * @return char:  символ '0'..'9'.
 */
char convert_int_to_char(uint8_t number);

/**
 * @brief  Отображение символов на матрице в течение
 *         TIME_DISPLAY_STRING_DURING_MS (определено в tim.c)
 * @note   Для DEMO_MODE и для протоколов при запуске индикатора.
 * @param  matrix_string: Указатель на строку, которая будет отображаться.
 * @retval None
 */
void display_symbols_during_ms(char *matrix_string);

#endif /*__ DRAWING_H__ */
