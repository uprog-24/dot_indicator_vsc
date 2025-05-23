/**
 * @file demo_mode.c
 */
#include "demo_mode.h"

#include "drawing.h"
#include "tim.h"

#include <stdlib.h>
#include <string.h>

#define STOP_FLOORS_BUFF_SIZE 4 ///< Size of buff_stop_floors
#define DISPLAY_STR_DURING_MS                                                  \
  2000 ///< Time during which symbols are displayed on matrix

/// Buffer with stop floors
static uint8_t buff_stop_floors[STOP_FLOORS_BUFF_SIZE] = {7, 8, 10, 11};

/**
 * @brief  Display string on matrix (DEMO_MODE).
 * @note   1. Set drawing_data structure (direction, floor);
 *         2. Set matrix_string that will be displayed on matrix;
 *         3. Display matrix_string during DISPLAY_STR_DURING_MS.
 * @param  floor:            Current floor
 * @param  direction:        Direction with directionType:
 *                           DIRECTION_UP/DIRECTION_DOWN/NO_DIRECTION
 * @param  buff_stop_floors: Pointer to the buffer with stop floors
 * @param  buff_stop_size:   Size of the buff_stop_floors
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
 * @brief  Movement from start to finish floor with stop floors
 * @param  start_floor
 * @param  finish_floor
 * @param  buff_stop_floors: Pointer to the buffer with stop floors
 * @param  buff_stop_size:   Size of the buff_stop_floors
 * @retval None
 */
static void demo_start_finish_floors_movement(uint8_t start_floor,
                                              uint8_t finish_floor,
                                              uint8_t *buff_stop_floors,
                                              uint8_t buff_stop_size) {
  uint8_t current_floor = start_floor;

  // start floor
  display_symbols(start_floor, NO_DIRECTION);
  if (finish_floor > start_floor) {
    display_symbols(start_floor, DIRECTION_UP);
  } else {
    display_symbols(start_floor, DIRECTION_DOWN);
  }

  // other floors
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

  // finish floor
  if (finish_floor > start_floor) {
    display_symbols(finish_floor, DIRECTION_UP);
  } else {
    display_symbols(finish_floor, DIRECTION_DOWN);
  }
  display_symbols(finish_floor, NO_DIRECTION);
}

/**
 * @brief  Start lift movement in demo mode
 * @retval None
 */
void demo_mode_start(void) {
#if DOT_PIN
  demo_start_finish_floors_movement(1, 14, buff_stop_floors,
                                    STOP_FLOORS_BUFF_SIZE);
  demo_start_finish_floors_movement(14, 1, NULL, 0);
#elif DOT_SPI

  // demo_start_finish_floors_movement(1, 14, buff_stop_floors,
  //                                   STOP_FLOORS_BUFF_SIZE);
  // demo_start_finish_floors_movement(14, 1, NULL, 0);

  display_symbols_spi("---");
#endif
}
