/**
 * @file    font.h
 * @brief   Этот файл содержит прототипы функций для файла font.c
 */
#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

#define BINARY_SYMBOL_SIZE 8 ///< 8 бит в строке символа

/**
 * @brief  Преобразование десятичного числа в двоичный массив (для получения
 *         двоичных строк символа в drawing.c).
 * @param  number:     Число для двоичной строки символа.
 * @param  binary_mas: Указатель на массив с двоичным представлением строки
 *                     символа (из font.c symbols[]).
 * @param  bin_size:   Размер двоичной строки символа.
 * @retval None
 */
void convert_number_from_dec_to_bin(uint8_t number, uint8_t *binary_mas,
                                    uint8_t bin_size);

/**
 * @brief  Получение кода символа из массива symbols[].
 * @param  symbol:                 Символ из symbols[] (font.c).
 * @retval Указатель на buff_code: Код символа.
 */
uint8_t *get_symbol_code(char symbol);

#endif /*__FONT_H__ */
