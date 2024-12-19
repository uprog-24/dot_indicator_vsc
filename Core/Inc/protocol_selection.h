/**
 * @file    protocol_selection.h
 * @brief   This file contains all the function prototypes for
 *          the protocol_selection.c file
 */
#ifndef __PROTOCOL_SELECTION_H__
#define __PROTOCOL_SELECTION_H__

/**
 * @brief  Display protocol name - UKL/SHK/UEL - on the matrix
 * @param  protocol_name Protocol name declared in config.h
 * @retval None
 */
void display_protocol_name(char *protocol_name);

/**
 * @brief  Initialize protocol UIM_6100/UEL/UKL
 * @param  None
 * @retval None
 */
void protocol_init();

/**
 * @brief  Start protocol UIM_6100/UEL/UKL
 * @param  None
 * @retval None
 */
void protocol_start();

/**
 * @brief  Process data by protocol UIM_6100/UEL/UKL
 * @param  None
 * @retval None
 */
void protocol_process_data();

/**
 * @brief  Stop protocol UIM_6100/UEL/UKL
 * @param  None
 * @retval None
 */
void protocol_stop();

#endif /*__ PROTOCOL_SELECTION_H__ */
