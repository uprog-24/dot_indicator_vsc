/**
 * @file flash.c
 */
#include "flash.h"

#define LOW_HALF_WORD_MASK                                                     \
  0xFF ///< Маска 16 bits для 2-х байт данных: addr_id (8 bits) and volume (8
       ///< bits)

/// Структура для секции SETTINGS, объявленнной в скрипте компоновщика
/// CubeMX/.ld
static settings_t settings_flash
    __attribute__((__section__(".settings"), used));

/**
 * @brief  Стираем 1 страницу (стр. 127) во flash-памяти, начиная с
 *         __SETTINGS_SECTION_START (раздел SETTINGS объявлен в файле .ld).
 * @param  None
 * @retval status: HAL Status.
 */
static HAL_StatusTypeDef erase_settings(void) {
  /// Стартовый адрес секции SETTINGS (из .ld файла).
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
 * @brief  Запись слова (WORD, 32 бита) во flash-память: адрес индикатора и
 *         уровень громкости бузера (Например: 0xFFFF022D).
 * @param  addr_id: Адрес индикатора.
 * @param  volume:  Уровень громкости бузера.
 * @retval status:  HAL Status.
 */
static HAL_StatusTypeDef write_settings(uint8_t addr_id, volume_t volume,
                                        uint8_t group_id) {
  // 2 байта: addr_id и volume, первые 2 байта заполнены 0xFFFF
  uint32_t packed_data =
      (addr_id << 8) | (uint8_t)volume | (0xFF00 << 16) | (group_id << 16);

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
 * @brief  Чтение адреса индикатора и уровня громкости бузера из Flash-памяти в
 *         структуру.
 * @param  settings: Указатель на структуру с настройками.
 * @retval status:   HAL Status.
 */
HAL_StatusTypeDef read_settings(settings_t *settings) {
  uint32_t packed_data = *(uint32_t *)&settings_flash;

  settings->addr_id = (packed_data >> 8) & LOW_HALF_WORD_MASK;
  settings->volume = (volume_t)(packed_data & LOW_HALF_WORD_MASK);
  settings->group_id = (packed_data >> 16) & LOW_HALF_WORD_MASK;

  return HAL_OK;
}

/**
 * @brief  Перезапись настроек, если есть изменения.
 * @param  settings: Указатель на структуру с настройками.
 * @retval None
 */
void overwrite_settings(settings_t *settings) {
  settings_t current_flash_settings = {1, 1, 1};
  read_settings(&current_flash_settings);

  if (current_flash_settings.addr_id != settings->addr_id ||
      current_flash_settings.volume != settings->volume ||
      current_flash_settings.group_id != settings->group_id) {
    write_settings(settings->addr_id, settings->volume, settings->group_id);
  }
}

/**
 * @brief  Обновление настроек в структуре.
 * @param  settings:   Указатель на структуру с настройками.
 * @param  new_volume: Новое значение громкости.
 * @param  new_id:     Новое значение адреса.
 * @retval None
 */
void update_structure(settings_t *settings, volume_t new_volume, uint8_t new_id,
                      uint8_t new_group_id) {
  settings->volume = new_volume;
  settings->addr_id = new_id;
  settings->group_id = new_group_id;
}
