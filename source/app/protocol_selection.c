/**
 * @file protocol_selection.c
 */
#include "protocol_selection.h"

#include "config.h"
#include "drawing.h"

/**
 * @brief  Инициализация интерфейса для протокола.
 * @param  None
 * @retval None
 */
void protocol_init() {
  bool is_id_from_flash_valid = matrix_settings.addr_id >= ADDR_ID_MIN &&
                                matrix_settings.addr_id <= ADDR_ID_LIMIT;

  if (!is_id_from_flash_valid) {
    matrix_settings.addr_id = MAIN_CABIN_ID;
    matrix_settings.volume = VOLUME_1;
  }

#if PROTOCOL_UIM_6100
  MX_CAN_Init();
#endif
}

/**
 * @brief   Запуск обработки протокола (запуск интерфейса).
 * @param  None
 * @retval None
 */
void protocol_start() {
#if PROTOCOL_UIM_6100

  bool is_id_from_flash_valid = matrix_settings.addr_id >= ADDR_ID_MIN &&
                                matrix_settings.addr_id <= ADDR_ID_LIMIT;

  if (is_id_from_flash_valid) {
    start_can(&hcan, matrix_settings.addr_id);
  } else {
    start_can(&hcan, MAIN_CABIN_ID);
  }
#endif
}

/**
 * @brief  Обработка данных протокола, если интерфейс подключен, иначе
 *         отображается "--".
 * @param  None
 * @retval None
 */

void protocol_process_data() {
  if (is_interface_connected) {
#if PROTOCOL_UIM_6100
    process_data_from_can();
#endif
  } else {
    draw_string_on_matrix("c--");
  }
}

/**
 * @brief  Остановка обработки протокола.
 * @param  None
 * @retval None
 */
void protocol_stop() {
  is_interface_connected = false;

#if PROTOCOL_UIM_6100
  stop_can(&hcan);
#endif
}
