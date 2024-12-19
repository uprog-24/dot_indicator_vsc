/**
 * @file    button.h
 * @brief   This file contains all the function prototypes for
 *          the button.c file
 */
#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <stdint.h>

/**
 * @brief  Start TIM4 for counting 20 seconds between click buttons when
 *         matrix_state = MATRIX_STATE_MENU
 * @param  None
 * @retval None
 */
void start_timer_menu();

/**
 * @brief  Handle pressing BUTTON_1 and BUTTON_2.
 * @note   When BUTTON_1 is pressed 1st time - matrix_state = MATRIX_STATE_MENU,
 *         BUTTON_1 allows to select settings_mode_t: ID, VOLUME, ESCAPE
 *         BUTTON_2 allows to select value for ID, VOLUME
 * @param  None
 * @retval None
 */
void press_button();

/**
 * @brief  Stop TIM4 when matrix_state = MATRIX_STATE_MENU
 * @param  None
 * @retval None
 */
void stop_timer_menu();

#endif /*__ BUTTON_H__ */
