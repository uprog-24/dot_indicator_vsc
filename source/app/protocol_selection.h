/**
 * @file    protocol_selection.h
 * @brief   Этот файл содержит прототипы функций для файла protocol_selection.c
 */
#ifndef __PROTOCOL_SELECTION_H__
#define __PROTOCOL_SELECTION_H__

/**
 * @brief  Инициализация интерфейса для протокола.
 * @param  None
 * @retval None
 */
void protocol_init();

/**
 * @brief  Запуск обработки протокола (запуск интерфейса).
 * @param  None
 * @retval None
 */
void protocol_start();

/**
 * @brief  Обработка данных протокола, если интерфейс подключен, иначе
 *         отображается "--".
 * @param  None
 * @retval None
 */
void protocol_process_data();

/**
 * @brief  Остановка обработки протокола.
 * @param  None
 * @retval None
 */
void protocol_stop();

#endif /*__ PROTOCOL_SELECTION_H__ */
