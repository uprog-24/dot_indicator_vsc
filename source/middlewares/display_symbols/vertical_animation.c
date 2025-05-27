// ##################################################################### INCLUDES #####################################################################
#include "LED_panel_driver.h"

// ############################################################ PUBLIC FUNCTIONS ###############################################################

/*	@brief 	Формирует из двух bitmap'ов (current_bitmap_ptr и next_bitmap_ptr) новый bitmap (bitmap_to_save_ptr),
 * 			который соответствует определенной итерации анимации вверх
 *
 * @params	uint8_t bitmap_to_save_ptr[ELEMENTS_IN_BITMAP]		- финальный bitmap
 * 			uint8_t iteration_8_to_0 							- итерация анимации (значение от 0 до 8)
 * 			uint8_t current_bitmap_ptr[ELEMENTS_IN_BITMAP]		- bitmap для текущего (находится на экране) элемента
 * 			uint8_t next_bitmap_ptr[ELEMENTS_IN_BITMAP]			- bitmap для следущего (сменится на экране) элемента
 *
 * @retVal	1 - bitmap сформирован; 0 - ошибка (nullptr) или неверное значение iteiteration_0_to_8
 */
_Bool animation_scroll_up (uint8_t *bitmap_to_save_ptr, uint8_t iteration_8_to_0, uint8_t *current_bitmap_ptr, uint8_t *next_bitmap_ptr)
{
	uint32_t i;
	uint8_t replaceable, replacing, iteration;
	int8_t  shift_number;

	if((bitmap_to_save_ptr == NULL) || (current_bitmap_ptr == NULL) || (next_bitmap_ptr == NULL) )
		return 0;

	if(iteration_8_to_0 > VERTICAL_ANIMATION_CYCLES)
		return 0;

	iteration = VERTICAL_ANIMATION_CYCLES - iteration_8_to_0;

	for(i = 0; i < ELEMENTS_IN_BITMAP ; ++i) {

		replaceable  = current_bitmap_ptr[i] << iteration;
		shift_number = PANEL_NUMBER_OF_COLUMNS - iteration + 1;
		replacing    = (shift_number < 0) ? (0) : (next_bitmap_ptr[i] >> shift_number);

		bitmap_to_save_ptr[i] = replaceable | replacing;
	}

	return 1;
}

/*	@brief 	Формирует из двух bitmap'ов (current_bitmap_ptr и next_bitmap_ptr) новый bitmap (bitmap_to_save_ptr),
 * 			который соответствует определенной итерации анимации вниз
 *
 * @params	uint8_t bitmap_to_save_ptr[ELEMENTS_IN_BITMAP]		- финальный bitmap
 * 			uint8_t iteration_8_to_0							- итерация анимации (от 8 до 1)
 * 			uint8_t current_bitmap_ptr[ELEMENTS_IN_BITMAP]		- bitmap для текущего (находится на экране) элемента
 * 			uint8_t next_bitmap_ptr[ELEMENTS_IN_BITMAP]			- bitmap для следущего (сменится на экране) элемента
 *
 * @retVal	1 - bitmap сформирован; 0 - ошибка (nullptr)
 */
_Bool animation_scroll_down (uint8_t *bitmap_to_save_ptr, uint8_t iteration_8_to_0, uint8_t *current_bitmap_ptr, uint8_t * next_bitmap_ptr)
{
	uint32_t i;
	uint8_t replaceable, replacing;
	int8_t  shift_number;

	if((bitmap_to_save_ptr == NULL) || (current_bitmap_ptr == NULL) || (next_bitmap_ptr == NULL) )
		return 0;

	if(iteration_8_to_0 > VERTICAL_ANIMATION_CYCLES)
		return 0;

	for(i = 0; i < ELEMENTS_IN_BITMAP ; ++i) {

		replacing   = next_bitmap_ptr[i] << iteration_8_to_0;
		shift_number = PANEL_NUMBER_OF_COLUMNS - iteration_8_to_0 + 1;
		replaceable	 = (shift_number < 0) ? (0) : (current_bitmap_ptr[i]  >> shift_number);

		bitmap_to_save_ptr[i] = replaceable | replacing;
	}

	return 1;
}


