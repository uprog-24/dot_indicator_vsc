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

  bool is_group_id_from_flash_valid =
      matrix_settings.group_id >= GROUP_ID_MIN && matrix_settings.group_id <= 4;

  if (!is_group_id_from_flash_valid) {
    matrix_settings.group_id = GROUP_ID_MIN;
  }

#if 0
#if PROTOCOL_UIM_6100
  MX_CAN_Init();
#endif
#endif

#if PROTOCOL_NKU_SD7
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DATA_GPIO_Port, &GPIO_InitStruct);

  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
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
#if PROTOCOL_NKU_SD7
    process_data_pin();
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
  stop_buzzer_sound();

#if PROTOCOL_UIM_6100
  stop_can(&hcan);

#elif PROTOCOL_NKU_SD7
  stop_sd7_before_menu_mode();
#endif
}
