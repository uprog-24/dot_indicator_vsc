/**
 * @file    test_mode.h
 * @brief   Этот файл содержит все прототипы функций для файла test_mode.c
 */
#ifndef TEST_MODE_H
#define TEST_MODE_H

#include <stdint.h>

/**
 * @brief  Запуск тестового режима (проверка светодиодов матрицы, CAN и бузера).
 * @retval
 */
void test_mode_start();

#endif // TEST_MODE_H