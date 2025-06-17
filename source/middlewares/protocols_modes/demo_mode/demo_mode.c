/**
 * @file demo_mode.c
 */
#include "demo_mode.h"

#include "dot.h"
#include "drawing.h"
#include "main.h"

#include <stdlib.h>
#include <string.h>

#define START_FLOOR 1   ///< Стартовый этаж
#define FINISH_FLOOR 14 ///< Финишный этаж

#define STOP_FLOORS_BUFF_SIZE 4 ///< Размер буфера с этажами-остановками.

/// буфер с этажами-остановками
static uint8_t buff_stop_floors[STOP_FLOORS_BUFF_SIZE] = {7, 8, 10, 11};

typedef enum { MOVING_UP, MOVING_DOWN, NO_MOVING } moving_e;

symbol_code_e dir_sym;
static inline symbol_code_e
map_direction_to_common_symbol(directionType direction, moving_e moving_type) {

  switch (moving_type) {
  case MOVING_UP:
    dir_sym = SYMBOL_ARROW_UP_ANIMATION;
    break;

  case MOVING_DOWN:
    dir_sym = SYMBOL_ARROW_DOWN_ANIMATION;
    break;

  case NO_MOVING:
    if (direction == DIRECTION_UP) {
      dir_sym = SYMBOL_ARROW_UP;
    } else if (direction == DIRECTION_DOWN) {
      dir_sym = SYMBOL_ARROW_DOWN;
    } else {
      dir_sym = SYMBOL_EMPTY;
    }
    break;
  }

  return dir_sym;
}

/**
 * @brief  Отображение символов на матрице (DEMO_MODE).
 * @note   1. Установка символов (направление, этаж);
 *         2. Отображение символов в течение
 *            TIME_DISPLAY_STRING_DURING_MS (tim.c).
 * @param  floor:            Текущий этаж.
 * @param  direction:        Текущеее направление движения (directionType:
 *                           DIRECTION_UP/DIRECTION_DOWN/NO_DIRECTION).
 * @retval None
 */
static void display_symbols(uint8_t floor, directionType direction,
                            uint8_t is_moving) {
  char matrix_string[3];
  drawing_data_t drawing_data = {0, 0};

  // Настройка кода стрелки
  set_direction_symbol(map_direction_to_common_symbol(direction, is_moving));

  // Настройка кода этажа
  // Этаж 0..9
  if (floor >= 0 && floor <= 9) {
    set_floor_symbols(floor, SYMBOL_EMPTY);
  } else {
    // Этажи с 10
    set_floor_symbols(floor / 10, floor % 10);
  }

  display_symbols_during_ms();
}

/**
 * @brief  Запуск демонстрационного режима (движение с остановками)
 * @retval None
 */
void demo_mode_start(void) {

  display_symbols(1, NO_DIRECTION, NO_MOVING);
  display_symbols(1, DIRECTION_UP, NO_MOVING);
  display_symbols(1, DIRECTION_UP, MOVING_UP);
  display_symbols(2, DIRECTION_UP, MOVING_UP);

  display_symbols(3, DIRECTION_UP, NO_MOVING);
  display_symbols(3, NO_DIRECTION, NO_MOVING);
  display_symbols(3, DIRECTION_UP, NO_MOVING);
  display_symbols(3, DIRECTION_UP, MOVING_UP);

  display_symbols(4, NO_DIRECTION, NO_MOVING);

  display_symbols(4, DIRECTION_DOWN, NO_MOVING);
  display_symbols(4, DIRECTION_DOWN, MOVING_DOWN);

  display_symbols(3, DIRECTION_DOWN, MOVING_DOWN);
  display_symbols(2, DIRECTION_DOWN, MOVING_DOWN);
  display_symbols(1, NO_DIRECTION, NO_MOVING);
}
