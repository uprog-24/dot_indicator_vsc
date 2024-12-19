/**
 * @file    flash.h
 * @brief   This file contains all the function prototypes for
 *          the flash.c file
 */
#ifndef __FLASH_H__
#define __FLASH_H__

#include "buzzer.h"

/**
 * Stores the settings of matrix: addr_id of matrix and level of volume for
 * buzzer
 */
typedef struct {
  uint8_t addr_id;
  volume_t volume;
} settings_t;

/**
 * @brief  Read addr_id and volume from Flash into structure
 * @param  settings: Pointer to the settings structure
 * @retval status:   HAL Status
 */
HAL_StatusTypeDef read_settings(settings_t *settings);

/**
 * @brief  Overwrite settings if it's necessary (args settings != current
 *         settings in Flash)
 * @param  settings: Pointer to the settings structure
 * @retval None
 */
void overwrite_settings(settings_t *settings);

/**
 * @brief  Update settings structure with new values
 * @param  settings:   Pointer to the settings structure
 * @param  new_volume: New selected level of volume
 * @param  new_id:     New selected ID of matrix
 * @retval None
 */
void update_structure(settings_t *settings, volume_t new_volume,
                      uint8_t new_id);

#endif /*__ BUTTON_H__ */
