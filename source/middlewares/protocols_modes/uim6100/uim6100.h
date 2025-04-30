/**
 * @file    uim6100.h
 * @brief   Этот файл содержит прототипы функций для файла uim6100.c
 */
#ifndef UIM6100_H
#define UIM6100_H

#include <stdint.h>

#define UIM6100_DLC 6 ///< Длина сообщения (6 байт)
#define UIM6100_MAIN_CABIN_CAN_ID 46 ///< ID кабинного индикатора

/*
 * Структура для сохранения полученных байтов по CAN.
 */
typedef struct {
  uint8_t w0;
  uint8_t w1;
  uint8_t w2;
  uint8_t w3;
} msg_t;

/**
 * @brief  Обработка данных по протоколу UIM6100 (ШК6000).
 * @note   1. Установка структуры drawing_data, обработка code message,
 *            воспроизведение гонга и отображение символов;
 *         2. Отображение matrix_string пока следующие данные не получены и
 *            интерфейс CAN подключен.
 * @param  msg: Указатель на структуру полученных данных.
 * @retval None
 */
void process_data_uim(msg_t *msg);

#endif // UIM6100_H
