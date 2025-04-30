/**
 * @file test_mode.c
 */
#include "test_mode.h"

#include "buzzer.h"
#include "can.h"
#include "dot.h"
#include "main.h"
#include "tim.h"

#define BUZZER_BUFF_SIZE 2

static const uint16_t buzzer_freq_buff[BUZZER_BUFF_SIZE] = {3000, 1000};

/// Строка для отображения по приему данных по CAN в режиме loopback
static char *str_ok = "c0K";

/// Флаг для проверки, получены ли данные по CAN
extern volatile bool is_data_received;

/**
 * @brief  Запуск тестового режима (проверка матрицы, CAN и бузера)
 * @retval
 */
void test_mode_start() {

  /* Построчное заполнение матрицы (включение светодиодов) */
  set_matrix_by_rows();
  TIM3_Delay_ms(500);

  /* Выключение и включение матрицы */
  set_full_matrix_state(TURN_OFF);
  TIM3_Delay_ms(500);
  set_full_matrix_state(TURN_ON);

  /* Включение пассивного бузера (воспроизведение 2-х частот) */
  set_passive_buzzer_melody(buzzer_freq_buff, BUZZER_BUFF_SIZE);

  set_full_matrix_state(TURN_OFF);

  /* Отправка данных по CAN в режиме loopback */
  MX_CAN_Init();
  start_can(&hcan, TEST_MODE_STD_ID);
  CAN_TxData(TEST_MODE_STD_ID);

  /* Отображение строки, если данные получены */
  while (1) {
    if (is_data_received) {
      draw_string_on_matrix(str_ok);
    }
  }
}