/**
 * @file    button.h
 * @brief   Этот файл содержит прототипы функций для файла button.c
 */
#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <stdint.h>

/**
 * @brief  Обработка нажатий BUTTON_1 и BUTTON_2.
 * @note   Когда BUTTON_1 нажата 1 раз, то индикатор переходит в состояние меню
 *         matrix_state = MATRIX_STATE_MENU,
 *         BUTTON_1 позволяет выбирать режим меню: ID (адрес индикатора), VOLUME
 *                  (уровень громкости), ESCAPE (выход из меню С сохранением
 *                  выбранных значений).
 *         BUTTON_2 позволяет выбрать значение для ID, VOLUME.
 * @param  None
 * @retval None
 */
void press_button();

#endif /*__ BUTTON_H__ */
