/**
 * @file test_mode.c
 */
#include "test_mode.h"

#include "buzzer.h"
#include "can.h"
#include "dot.h"
#include "drawing.h"
#include "main.h"
#include "tim.h"

/// Строка для отображения по приему данных по CAN в режиме loopback
static char *str_ok = "c0K";

/// Индекс текущей колонки в цикле.
static uint8_t current_col = 0;

/// Индекс текущей строки в цикле.
static uint8_t current_row = 0;

/**
 * @brief  Поочерёдно включает каждый светодиод на матрице, постепенно включая
 *         её полностью.
 * @param  None
 * @retval None
 */
static void set_matrix_by_rows() {
  while (current_row < ROWS) {
    set_all_cols_state(TURN_OFF);

    set_row_state(current_row, TURN_ON);
    while (current_col < COLUMNS) {
      set_col_state(current_col, TURN_ON);
      TIM3_Delay_ms(100);
      current_col++;
    }

    for (uint8_t r = 0; r < current_row; r++) {
      set_row_state(r, TURN_ON);
    }

    current_row++;
    current_col = 0;
  }
}

#if 0
static void set_wave_by_rows() {
  current_row = 0;
  current_col = 0;
  while (current_row < ROWS) {
    set_all_cols_state(TURN_OFF);

    set_row_state(current_row, TURN_ON);
    while (current_col < COLUMNS) {
      set_col_state(current_col, TURN_ON);
      TIM3_Delay_ms(100);
      current_col++;
    }

    set_row_state(current_row, TURN_OFF);

    current_row++;
    current_col = 0;
  }
}

static void set_wave_by_cols() {
  current_col = 0;
  current_row = 0;
  while (current_col < COLUMNS) {
    set_all_rows_state(TURN_OFF);

    set_col_state(current_col, TURN_ON);
    while (current_row < ROWS) {
      set_row_state(current_row, TURN_ON);
      TIM3_Delay_ms(100);
      current_row++;
    }

    set_col_state(current_col, TURN_OFF);

    current_col++;
    current_row = 0;
  }
}
#else
static void set_wave_by_rows() {
  current_row = 0;
  current_col = 0;
  set_all_cols_state(TURN_ON);
  while (current_row < ROWS) {

    set_row_state(current_row, TURN_ON);
    TIM3_Delay_ms(100);
    set_row_state(current_row, TURN_OFF);

    current_row++;
    current_col = 0;
  }
}

static void set_wave_by_cols() {
  current_col = 0;
  current_row = 0;
  set_all_rows_state(TURN_ON);
  while (current_col < COLUMNS) {

    set_col_state(current_col, TURN_ON);
    TIM3_Delay_ms(100);
    set_col_state(current_col, TURN_OFF);

    current_col++;
    current_row = 0;
  }
}
#endif

/// Флаг для проверки, получены ли данные по CAN
extern volatile bool is_data_received;

/**
 * @brief  Запуск тестового режима (проверка светодиодов матрицы, CAN и бузера).
 * @retval
 */
void test_mode_start() {

#if 0
  /* Построчное заполнение матрицы (включение светодиодов) */
  set_matrix_by_rows();
  TIM3_Delay_ms(500);

  /* Выключение и включение матрицы */
  set_full_matrix_state(TURN_OFF);
  TIM3_Delay_ms(1000);
  set_full_matrix_state(TURN_ON);

  /* Включение пассивного бузера (воспроизведение гонга из 3-х частот: 1000,
   * 900, 800 Гц) с максимальной громкостью */
  play_gong(3, 1000, VOLUME_3);

  TIM3_Delay_ms(1000);
  set_full_matrix_state(TURN_OFF);
  TIM3_Delay_ms(1000);

  /* Отправка данных по CAN в режиме loopback */
  MX_CAN_Init();
  start_can(&hcan, TEST_MODE_STD_ID);
  CAN_TxData(TEST_MODE_STD_ID);

  /* Отображение строки, если данные получены */
  set_symbols(SYMBOL_EMPTY, SYMBOL_0, SYMBOL_K);
  while (1) {
    if (is_data_received) {
      draw_symbols();
    }
  }
#else

  // current_col = 0;
  // current_row = 0;

  set_full_matrix_state(TURN_OFF);
  TIM3_Delay_ms(100);
  set_wave_by_cols();
  set_full_matrix_state(TURN_OFF);
  set_wave_by_rows();
  set_full_matrix_state(TURN_OFF);
#endif
}
