/**
 * @file    nku.h
 * @brief   This file contains all the function prototypes for
 *          the nku.c file
 */
#ifndef NKU_H
#define NKU_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief  Process data using NKU protocol
 * @note   1. Set drawing_data structure, process code message, setting gong
 *            and symbols;
 *         2. Display matrix_string while next data is not received and
 *            interface is connected.
 * @param  rx_data_can: Pointer to the buffer with received data by CAN
 * @retval None
 */
void process_data_nku();

#endif // NKU_H
