/**
 * @file    uim6100.h
 * @brief   This file contains all the function prototypes for
 *          the uim6100.c file
 */
#ifndef UIM6100_H
#define UIM6100_H

#include <stdint.h>

#define UIM6100_DLC 6                ///< Length of data (6 bit)
#define UIM6100_MAIN_CABIN_CAN_ID 46 ///< ID of the main cabin
#define BYTE_CODE_OPERATION_0_VALUE                                            \
  0x81 ///< Value of BYTE_CODE_OPERATION_0 byte
#define BYTE_CODE_OPERATION_1_VALUE                                            \
  0x00 ///< Value of BYTE_CODE_OPERATION_1 byte

/**
 * @brief  Process data using UIM6100 protocol
 * @note   1. Set drawing_data structure, process code message, setting gong
 *            and symbols;
 *         2. Display matrix_string while next data is not received and
 *            interface is connected.
 * @param  rx_data_can: Pointer to the buffer with received data by CAN
 * @retval None
 */
void process_data_uim(uint8_t *rx_data_can);

#endif // UIM6100_H
