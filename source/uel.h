/**
 * @file    uel.h
 * @brief   This file contains all the function prototypes for
 *          the uel.c file
 */
#ifndef __UEL_H__
#define __UEL_H__

#include <stdbool.h>
#include <stdint.h>

#define UEL_MAIN_CABIN_ID 1  ///< ID of the main cabin

/**
 * @brief  Process data using UEL protocol
 *         1. Get 9 bits from 16 received bits;
 *         2. Set drawing_data structure, setting symbols and sound;
 *         3. Display matrix_string while next data is not received and
 *            interface is connected.
 * @param  received_data: Pointer to the buffer with received data by CAN
 * @retval None
 */
void process_data_uel(uint16_t *rx_data);

#endif /* __UEL_H__ */
