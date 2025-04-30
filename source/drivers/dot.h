/**
 * @file    dot.h
 * @brief   Этот файл содержит прототипы функций для файла dot.c
 */
#ifndef __DOT_H__
#define __DOT_H__

#include "main.h"

#include <stdint.h>

#define ROWS 8     ///< Количество строк в матрице
#define COLUMNS 16 ///< Количество колонок в матрице

/**
 * @brief  Установка состояния строки, включение/выключение.
 * @param  row:   Текущая строка в диапазоне [0, ROWS).
 * @param  state: Состояние строки типа states_t из main.h: TURN_ON, TURN_OFF.
 * @retval None
 */
void set_row_state(uint8_t row, states_t state);

/**
 * @brief  Установка состояния колонки, включение/выключение.
 * @param  col:   Текущая колонка в диапазоне [0, COLUMNS).
 * @param  state: Состояние колонки типа states_t из main.h: TURN_ON, TURN_OFF.
 * @retval None
 */
void set_col_state(uint8_t col, states_t state);

/**
 * @brief  Установка состояния для всех строк, включение/выключение.
 * @param  state: Состояние для всех строк типа states_t из main.h: TURN_ON,
 *                TURN_OFF.
 * @retval None
 */
void set_all_rows_state(states_t state);

/**
 * @brief  Установка состояния для всех колонок, включение/выключение.
 * @param  state: Состояние для всех колонок типа states_t из main.h: TURN_ON,
 *                TURN_OFF.
 * @retval None
 */
void set_all_cols_state(states_t state);

/**
 * @brief  Установка состояния матрицы, включение/выключение.
 * @param  state: Состояние матрицы типа states_t из main.h: TURN_ON, TURN_OFF.
 * @retval None
 */
void set_full_matrix_state(states_t state);

#endif /*__DOT_H__ */
