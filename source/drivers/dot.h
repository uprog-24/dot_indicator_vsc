/**
 * @file    dot.h
 * @brief   This file contains all the function prototypes for
 *          the dot.c file
 */
#ifndef __DOT_H__
#define __DOT_H__

#include "main.h"

#include <stdint.h>

#define ROWS 8     ///< Number of rows in matrix
#define COLUMNS 16 ///< Number of columns in matrix

/**
 * @brief  Set row state, turn on/off
 * @param  row:   Current row in range [0, ROWS)
 * @param  state: Type states_t: TURN_ON, TURN_OFF
 * @retval None
 */
void set_row_state(uint8_t row, states_t state);

/**
 * @brief  Set column state, turn on/off
 * @param  col:   Current column in range [0, COLUMNS)
 * @param  state: Type states_t: TURN_ON, TURN_OFF
 * @retval None
 */
void set_col_state(uint8_t col, states_t state);

/**
 * @brief  Set all rows state, turn on/off
 * @param  state: Type states_t: TURN_ON, TURN_OFF
 * @retval None
 */
void set_all_rows_state(states_t state);

/**
 * @brief  Set all cols state, turn on/off
 * @param  state: Type states_t: TURN_ON, TURN_OFF
 * @retval None
 */
void set_all_cols_state(states_t state);

/**
 * @brief  Set matrix state, turn on/off
 * @param  state: Type states_t: TURN_ON, TURN_OFF
 * @retval None
 */
void set_full_matrix_state(states_t state);

/**
 * @brief  Turn on each LED on the matrix through 100 ms, turning it on
 *         completely (TEST_MODE)
 * @param  None
 * @retval None
 */
void set_matrix_by_rows();

#endif /*__DOT_H__ */
