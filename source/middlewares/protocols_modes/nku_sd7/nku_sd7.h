/**
 * @file    nku_sd7.h
 * @brief   Этот файл содержит прототипы функций для файла nku_sd7.c
 */
#ifndef NKU_SD7_H
#define NKU_SD7_H

#include <stdint.h>

#define SD7_BIT_LENGTH_US 2556

/**
 * @brief  Обработка данных по протоколу UIM6100 (ШК6000).
 * @note   1. Установка структуры drawing_data, обработка code message,
 *            воспроизведение гонга и отображение символов;
 *         2. Отображение matrix_string пока следующие данные не получены и
 *            интерфейс CAN подключен.
 * @param  msg: Указатель на структуру полученных данных.
 * @retval None
 */
void process_data_nku_sd7();

#endif // UIM6100_H
