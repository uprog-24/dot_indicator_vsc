/**
 * @file dot.c
 */
#include "dot.h"

/**
 * Содержит параметры светодиода: порт и пин.
 */
typedef struct {
  GPIO_TypeDef *port;
  uint16_t pin;
} pin_config_t;

/// Буфер с объявлением строк (порт, пин, определенные в main.h.).
pin_config_t rows[] = {
    {ROW_1_GPIO_Port, ROW_1_Pin}, {ROW_2_GPIO_Port, ROW_2_Pin},
    {ROW_3_GPIO_Port, ROW_3_Pin}, {ROW_4_GPIO_Port, ROW_4_Pin},
    {ROW_5_GPIO_Port, ROW_5_Pin}, {ROW_6_GPIO_Port, ROW_6_Pin},
    {ROW_7_GPIO_Port, ROW_7_Pin}, {ROW_8_GPIO_Port, ROW_8_Pin},
};

/// Буфер с объявлением колонок (порт, пин, определенные в main.h.).
pin_config_t cols[] = {

    {COL_R1_GPIO_Port, COL_R1_Pin}, {COL_R2_GPIO_Port, COL_R2_Pin},
    {COL_R3_GPIO_Port, COL_R3_Pin}, {COL_R4_GPIO_Port, COL_R4_Pin},
    {COL_R5_GPIO_Port, COL_R5_Pin}, {COL_R6_GPIO_Port, COL_R6_Pin},
    {COL_R7_GPIO_Port, COL_R7_Pin}, {COL_R8_GPIO_Port, COL_R8_Pin},

    {COL_L1_GPIO_Port, COL_L1_Pin}, {COL_L2_GPIO_Port, COL_L2_Pin},
    {COL_L3_GPIO_Port, COL_L3_Pin}, {COL_L4_GPIO_Port, COL_L4_Pin},
    {COL_L5_GPIO_Port, COL_L5_Pin}, {COL_L6_GPIO_Port, COL_L6_Pin},
    {COL_L7_GPIO_Port, COL_L7_Pin}, {COL_L8_GPIO_Port, COL_L8_Pin},

};

/**
 * @brief  Установка состояния строки, включение/выключение.
 * @param  row:   Текущая строка в диапазоне [0, ROWS).
 * @param  state: Состояние строки типа states_t из main.h: TURN_ON, TURN_OFF.
 * @retval None
 */
void set_row_state(uint8_t row, states_t state) {
  if (row < ROWS) {
    switch (state) {
    case TURN_OFF:
      HAL_GPIO_WritePin(rows[row].port, rows[row].pin, GPIO_PIN_RESET);
      break;

    case TURN_ON:
      HAL_GPIO_WritePin(rows[row].port, rows[row].pin, GPIO_PIN_SET);
      break;
    }
  }
}

/**
 * @brief  Установка состояния колонки, включение/выключение.
 * @param  col:   Текущая колонка в диапазоне [0, COLUMNS).
 * @param  state: Состояние колонки типа states_t из main.h: TURN_ON, TURN_OFF.
 * @retval None
 */
void set_col_state(uint8_t col, states_t state) {
  if (col < COLUMNS) {
    switch (state) {
    case TURN_OFF:
      HAL_GPIO_WritePin(cols[col].port, cols[col].pin, GPIO_PIN_RESET);
      break;

    case TURN_ON:
      HAL_GPIO_WritePin(cols[col].port, cols[col].pin, GPIO_PIN_SET);
      break;
    }
  }
}

/**
 * @brief  Установка состояния для всех строк, включение/выключение.
 * @param  state: Состояние для всех строк типа states_t из main.h: TURN_ON,
 *                TURN_OFF.
 * @retval None
 */
void set_all_rows_state(states_t state) {
  for (uint8_t row = 0; row < ROWS; row++) {
    set_row_state(row, state);
  }
}

/**
 * @brief  Установка состояния для всех колонок, включение/выключение.
 * @param  state: Состояние для всех колонок типа states_t из main.h: TURN_ON,
 *                TURN_OFF.
 * @retval None
 */
void set_all_cols_state(states_t state) {
  for (int col = 0; col < COLUMNS; col++) {
    set_col_state(col, state);
  }
}

/**
 * @brief  Установка состояния матрицы, включение/выключение.
 * @param  state: Состояние матрицы типа states_t из main.h: TURN_ON, TURN_OFF.
 * @retval None
 */
void set_full_matrix_state(states_t state) {
  set_all_rows_state(state);
  set_all_cols_state(state);
}
