/**
 * @file protocol_selection.c
 */
#include "protocol_selection.h"

#include "config.h"
#include "drawing.h"
#include "tim.h"

#define TIME_MS_FOR_PROTOCOL_NAME                                              \
  3000 ///< Time of TIM4 (ms) to display protocol's name when the power is
       ///< applied

/**
 * @brief  Display protocol name - UKL/SHK/UEL - on the matrix
 * @param  protocol_name Protocol name declared in config.h
 * @retval None
 */
void display_protocol_name(char *protocol_name) {
  TIM4_Diaplay_symbols_on_matrix(TIME_MS_FOR_PROTOCOL_NAME, protocol_name);
}

/**
 * @brief  Initialize protocol UIM_6100/UEL/UKL
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
#elif PROTOCOL_UEL
  MX_USART1_UART_Init();
#elif PROTOCOL_UKL

  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DATA_GPIO_Port, &GPIO_InitStruct);

#elif PROTOCOL_ALPACA
  MX_CAN_Init();

#endif
}

/**
 * @brief  Start protocol UIM_6100/UEL/UKL
 * @param  None
 * @retval None
 */
void protocol_start() {
#if !TEST_MODE && !DEMO_MODE
  TIM4_Stop();
  TIM4_Start(); // Timer for checking interface connection
#endif

#if PROTOCOL_UIM_6100

  bool is_id_from_flash_valid = matrix_settings.addr_id >= ADDR_ID_MIN &&
                                matrix_settings.addr_id <= ADDR_ID_LIMIT;

  if (is_id_from_flash_valid) {
    start_can(&hcan, matrix_settings.addr_id);
  } else {
    start_can(&hcan, UIM6100_MAIN_CABIN_CAN_ID);
  }

#elif PROTOCOL_UEL
  receive_data_uart();
#elif PROTOCOL_UKL

#elif PROTOCOL_ALPACA

  start_can(&hcan, 0);

#endif
}

/**
 * @brief  Process data by protocol UIM_6100/UEL/UKL
 * @param  None
 * @retval None
 */

void protocol_process_data() {
  if (is_interface_connected) {
#if PROTOCOL_UIM_6100
    process_data_from_can();
#elif PROTOCOL_UEL
    process_data_from_uart();
#elif PROTOCOL_UKL
    process_data_pin();
#elif PROTOCOL_ALPACA
    process_data_from_can();
#endif
  } else {

#if DOT_PIN
    draw_string_on_matrix("c--");
#elif DOT_SPI
    display_symbols_spi("c--");
#endif
    // draw_string_on_matrix("c--");
  }
}

/**
 * @brief  Stop protocol UIM_6100/UEL/UKL
 * @param  None
 * @retval None
 */
void protocol_stop() {
  is_interface_connected = false;

#if PROTOCOL_UIM_6100
  stop_can(&hcan);
#elif PROTOCOL_UEL

#elif PROTOCOL_UKL

  stop_ukl_before_menu_mode();
#endif

  TIM4_Stop();
}
