/**
 * @file flash.c
 */
#include "flash.h"

#define LOW_HALF_WORD_MASK \
  0xFF  ///< Mask for 16 bits of data: addr_id (8 bits) and volume (8 bits)

/// Structure for section SETTINGS
static settings_t settings_flash
    __attribute__((__section__(".settings"), used));

/**
 * @brief  Erase 1 page (p. 127) in Flash-memory, starting with
 *         __SETTINGS_SECTION_START (section SETTINGS is declared in .ld file)
 * @param  None
 * @retval status: HAL Status
 */
static HAL_StatusTypeDef erase_settings(void) {
  /// Start address of section SETTINGS that defined in .ld file
  extern uint32_t __SETTINGS_SECTION_START;

  HAL_StatusTypeDef status = HAL_ERROR;

  static FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = (uint32_t)&__SETTINGS_SECTION_START;
  EraseInitStruct.NbPages = 1;

  uint32_t page_error = 0;

  __disable_irq();
  HAL_FLASH_Unlock();

  status = HAL_FLASHEx_Erase(&EraseInitStruct, &page_error);

  HAL_FLASH_Lock();
  __enable_irq();

  HAL_Delay(25);
  return status;
}

/**
 * @brief  Write WORD (32 bits) in Flash-memory: addr_id of matrix and level of
 *         volume for buzzer (Example: 0xFFFF022D)
 * @param  addr_id: ID of matrix
 * @param  volume:  Level of volume for buzzer
 * @retval status:  HAL Status
 */
static HAL_StatusTypeDef write_settings(uint8_t addr_id, volume_t volume) {
  // 2 bytes: id and volume
  uint32_t packed_data = (addr_id << 8) | (uint8_t)volume | (0xFFFF << 16);

  HAL_StatusTypeDef status = erase_settings();
  if (status != HAL_OK) {
    return status;
  }

  HAL_FLASH_Unlock();
  __disable_irq();

  status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)&settings_flash,
                             packed_data);

  __enable_irq();
  HAL_FLASH_Lock();

  uint32_t read_data = *(uint32_t *)&settings_flash;

  if (read_data != packed_data) {
    return HAL_ERROR;
  }

  return status;
}

/**
 * @brief  Read addr_id and volume from Flash into structure
 * @param  settings: Pointer to the settings structure
 * @retval status:   HAL Status
 */
HAL_StatusTypeDef read_settings(settings_t *settings) {
  uint32_t packed_data = *(uint32_t *)&settings_flash;

  settings->addr_id = (packed_data >> 8) & LOW_HALF_WORD_MASK;
  settings->volume = (volume_t)(packed_data & LOW_HALF_WORD_MASK);

  return HAL_OK;
}

/**
 * @brief  Overwrite settings if it's necessary (args settings != current
 *         settings in Flash)
 * @param  settings: Pointer to the settings structure
 * @retval None
 */
void overwrite_settings(settings_t *settings) {
  settings_t current_flash_settings = {1, 1};
  read_settings(&current_flash_settings);

  if (current_flash_settings.addr_id != settings->addr_id ||
      current_flash_settings.volume != settings->volume) {
    write_settings(settings->addr_id, settings->volume);
  }
}

/**
 * @brief  Update settings structure with new values
 * @param  settings:   Pointer to the settings structure
 * @param  new_volume: New selected level of volume
 * @param  new_id:     New selected ID of matrix
 * @retval None
 */
void update_structure(settings_t *settings, volume_t new_volume,
                      uint8_t new_id) {
  settings->volume = new_volume;
  settings->addr_id = new_id;
}
