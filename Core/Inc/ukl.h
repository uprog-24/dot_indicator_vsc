/**
 * @file    ukl.h
 * @brief   This file contains all the function prototypes for
 *          the ukl.c file
 */
#ifndef __UKL_H__
#define __UKL_H__

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief  Process data received by DATA_Pin.
 * @note   If transmitted data by UKL protocol is received then process data
 * @param  None
 * @retval None
 */
void process_data_pin();

/**
 * @brief  Read data bit when time from ukl_timings[PACKET_SIZE] is elapsed.
 * @note   Function is called in HAL_TIM_PeriodElapsedCallback
 *         1. Read DATA_Pin, add bit to received_data_ukl by bit_index, stop
 *            TIM3 and start it for next bit;
 *         2. Check received_data_ukl_copy (not all 1), counters for interface
 *            connection;
 *         3. Start TIM3 for finished delay in 200 ms;
 *         4. If bit_index == packet_size (13 bits) then reset states of
 *            variables. Set enable IRQ for reading DATA_Pin.
 * @param  None
 * @retval None
 */
void read_data_bit();

/**
 * @brief  Reset variables before menu mode
 * @param  None
 * @retval None
 */
void stop_ukl_before_menu_mode();

#endif /* __UKL_H__ */
