/**
 * @file    nku_sd7.h
 * @brief   Этот файл содержит прототипы функций для файла nku_sd7.c
 */
#ifndef NKU_SD7_H
#define NKU_SD7_H

#include <stdint.h>

#define SD7_BIT_LENGTH_US 2556

/**
 * @brief  Обработка данных по протоколу НКУ-SD7
 * @note   Фильтрация, воспроизведение гонгов и отображение символов
 */
void process_data_nku_sd7();

/**
 * @brief Если данные получены, то обработать их
 *
 */
void process_data_pin();

/**
 * @brief Чтение бита, получение 4-х байтов по протоколу НКУ-SD7
 *
 */
void read_data_bit(void);

/**
 * @brief Сброс состояния перед переходом в режим меню
 *
 */
void stop_sd7_before_menu_mode();

#endif // NKU_SD7_H
