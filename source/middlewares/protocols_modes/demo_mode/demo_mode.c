/**
 * @file demo_mode.c
 */
#include "demo_mode.h"

#include "drawing.h"
#include "tim.h"

#include <stdlib.h>
#include <string.h>

#define STOP_FLOORS_BUFF_SIZE 4 ///< Размер буфера с этажами-остановками.
#define DISPLAY_STR_DURING_MS                                                  \
  2000 ///< Время в мс, в течение которого символы отображаются на матрице

/// буфер с этажами-остановками
static uint8_t buff_stop_floors[STOP_FLOORS_BUFF_SIZE] = {7, 8, 10, 11};

/**
 * @brief  Отображение строки на матрице (DEMO_MODE).
 * @note   1. Заполнение структуры drawing_data (направление, этаж);
 *         2. Установка строки matrix_string, которая будет отображаться на
 *            матрице;
 *         3. Отображение строки matrix_string в течение DISPLAY_STR_DURING_MS.
 * @param  floor:            Текущий этаж.
 * @param  direction:        Текущеее направление движения (directionType:
 *                           DIRECTION_UP/DIRECTION_DOWN/NO_DIRECTION).
 * @param  buff_stop_floors: Указатель на буфер с этажами-остановками.
 * @param  buff_stop_size:   Размер буфера с этажами-остановками.
 * @retval None
 */
static void display_symbols(uint8_t floor, directionType direction) {
  char matrix_string[3];
  drawing_data_t drawing_data = {0, 0};

  drawing_data_setter(&drawing_data, floor, direction);
  setting_symbols(matrix_string, &drawing_data, floor, NULL, 0);
  TIM4_Diaplay_symbols_on_matrix(DISPLAY_STR_DURING_MS, matrix_string);
}

/**
 * @brief  Движение от стартового до финишного этажа с остановками.
 * @param  start_floor
 * @param  finish_floor
 * @param  buff_stop_floors: Указатель на буфер с этажами-остановками.
 * @param  buff_stop_size:   Размер буфера с этажами-остановками.
 * @retval None
 */
static void demo_start_finish_floors_movement(uint8_t start_floor,
                                              uint8_t finish_floor,
                                              uint8_t *buff_stop_floors,
                                              uint8_t buff_stop_size) {
  uint8_t current_floor = start_floor;

  // Стартовый этаж
  display_symbols(start_floor, NO_DIRECTION);

  // Движение вверх/вниз
  if (finish_floor > start_floor) {
    display_symbols(start_floor, DIRECTION_UP);
  } else {
    display_symbols(start_floor, DIRECTION_DOWN);
  }

  // Этажи с остановками
  while (abs(current_floor - finish_floor) > 0) {
    if (buff_stop_size != 0 && buff_stop_floors != NULL) {
      for (uint8_t ind = 0; ind < buff_stop_size; ind++) {
        if (current_floor == buff_stop_floors[ind]) {
          display_symbols(current_floor, DIRECTION_UP);
          display_symbols(current_floor, NO_DIRECTION);
          break;
        }
      }
    }

    if (finish_floor > start_floor) {
      display_symbols(current_floor, DIRECTION_UP);
      current_floor++;
    } else {
      display_symbols(current_floor, DIRECTION_DOWN);
      current_floor--;
    }
  }

  // Финишный этаж
  if (finish_floor > start_floor) {
    display_symbols(finish_floor, DIRECTION_UP);
  } else {
    display_symbols(finish_floor, DIRECTION_DOWN);
  }
  display_symbols(finish_floor, NO_DIRECTION);
}

/**
 * @brief  Запуск демонстрационного режима (движение с остановками)
 * @retval None
 */
void demo_mode_start(void) {
  demo_start_finish_floors_movement(1, 14, buff_stop_floors,
                                    STOP_FLOORS_BUFF_SIZE);
  demo_start_finish_floors_movement(14, 1, NULL, 0);
}
