/**
 * @file    flash.h
 * @brief   Этот файл содержит прототипы функций для файла flash.c
 */
#ifndef __FLASH_H__
#define __FLASH_H__

#include "buzzer.h"

/**
 * Структура для хранения настроек индикатора.
 */
typedef struct {
  uint8_t addr_id;  // Адрес индикатора
  volume_t volume;  // Уровень громкости бузера
  uint8_t group_id; // Доп. параметр
} settings_t;

/**
 * @brief  Чтение адреса индикатора и уровня громкости бузера из Flash-памяти в
 *         структуру.
 * @param  settings: Указатель на структуру с настройками.
 * @retval status:   HAL Status.
 */
HAL_StatusTypeDef read_settings(settings_t *settings);

/**
 * @brief  Перезапись настроек во flash, если есть изменения.
 * @param  settings: Указатель на структуру с настройками.
 * @retval None
 */
void overwrite_settings(settings_t *settings);

/**
 * @brief  Обновление настроек в структуре.
 * @param  settings:   Указатель на структуру с настройками.
 * @param  new_volume: Новое значение громкости.
 * @param  new_id:     Новое значение адреса.
 * @retval None
 */
void update_structure(settings_t *settings, volume_t new_volume, uint8_t new_id,
                      uint8_t new_group_id);

#endif /*__ BUTTON_H__ */
