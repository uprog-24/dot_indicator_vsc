/**
 * @file test_mode.c
 */
#include "test_mode.h"

#include "buzzer.h"
#include "can.h"
#include "dot.h"
#include "main.h"
#include "tim.h"


#define BUZZER_BUFF_SIZE 3

/**
 * @brief  Start in test mode
 * @retval None
 */
void test_mode_start() {

  uint16_t buzzer_freq_buff[BUZZER_BUFF_SIZE] = {3000, 4000};

  MX_CAN_Init();

  set_matrix_by_rows();
  TIM3_Delay_ms(500);

  set_full_matrix_state(TURN_OFF);
  TIM3_Delay_ms(500);
  set_full_matrix_state(TURN_ON);

  set_passive_buzzer_melody(buzzer_freq_buff, BUZZER_BUFF_SIZE);

  set_full_matrix_state(TURN_OFF);

  start_can(&hcan, TEST_MODE_STD_ID);

  while (1) {
    CAN_TxData(TEST_MODE_STD_ID);
  }
}