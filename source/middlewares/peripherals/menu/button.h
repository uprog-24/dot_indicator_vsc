/**
 * @file    button.h
 * @brief   This file contains all the function prototypes for
 *          the button.c file
 */
#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <stdint.h>

/**
 * @brief  Handle pressing BUTTON_1 and BUTTON_2.
 * @note   When BUTTON_1 is pressed 1st time - matrix_state = MATRIX_STATE_MENU,
 *         BUTTON_1 allows to select settings_mode_t: ID, VOLUME, ESCAPE
 *         BUTTON_2 allows to select value for ID, VOLUME
 * @param  None
 * @retval None
 */
void press_button();

#endif /*__ BUTTON_H__ */
