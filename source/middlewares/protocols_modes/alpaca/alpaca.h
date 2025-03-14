/**
 * @file    alpaca.h
 * @brief   This file contains all the function prototypes for
 *          the alpaca.c file
 */
#ifndef ALPACA_H
#define ALPACA_H

#include <stdint.h>

#define ALPACA_DLC 2 ///< Length of data (2 bit)

/**
 * @brief  Process data using Alpaca protocol
 * @note   1. Set drawing_data structure, process code message, setting gong
 *            and symbols;
 *         2. Display matrix_string while next data is not received and
 *            interface is connected.
 * @param  rx_data_can: Pointer to the buffer with received data by CAN
 * @retval None
 */
void process_data_alpaca();

#endif // UIM6100_H