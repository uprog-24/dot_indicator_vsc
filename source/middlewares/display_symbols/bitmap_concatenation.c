
// ##################################################################### INCLUDES #####################################################################
#include "LED_panel_driver.h"

// ############################################################ PUBLIC FUNCTIONS ###############################################################

_Bool concat_bitmap_3_elements(uint16_t (*concated_bitmap_ptr)[NUMBER_OF_DRIVERS], uint8_t top_bitmap_ptr[ELEMENTS_IN_BITMAP],
									  uint8_t bottom_right_bitmap_ptr[ELEMENTS_IN_BITMAP], uint8_t bottom_left_bitmap_ptr[ELEMENTS_IN_BITMAP])
{
	int j;
	uint16_t row_flag; // переменная для размещения бита, отвечающего за включение ряда на панели

	if((concated_bitmap_ptr == NULL)     || (top_bitmap_ptr == NULL) ||
	   (bottom_right_bitmap_ptr == NULL) || (bottom_left_bitmap_ptr == NULL))
			return 0;

	// построчная развертка
	for(j = 0; j < PANEL_NUMBER_OF_ROWS; ++j) {
		row_flag = 1 << (2 + j);
		if(concated_bitmap_ptr[j] == NULL)
			return 0;
		// записываем bitmap для верхнего разряда
		concated_bitmap_ptr[j][0] =  row_flag + (top_bitmap_ptr[j] << 8);
		// записываем bitmap для правого нижнего раряда
		concated_bitmap_ptr[j][1] =  row_flag + (bottom_right_bitmap_ptr[j] << 8);
		// записываем bitmap для левого нижнего раряда
		concated_bitmap_ptr[j][2] =  row_flag + (bottom_left_bitmap_ptr[j] << 8);
	}

	return 1;
}

_Bool concat_bitmap_2_elements(uint16_t (*concated_bitmap_ptr)[NUMBER_OF_DRIVERS], uint8_t first_bitmap_ptr[ELEMENTS_IN_BITMAP],
						   uint8_t first_elem_in_bitmap, uint8_t second_bitmap_ptr[ELEMENTS_IN_BITMAP], uint8_t second_elem_in_bitmap)
{
	int j;
	uint16_t row_flag;

	if((concated_bitmap_ptr == NULL) || (first_bitmap_ptr == NULL) ||
		(second_bitmap_ptr == NULL))
			return 0;

	if((first_elem_in_bitmap > 2) || (second_elem_in_bitmap > 2))
		return 0;

	// построчная развертка
	for(j = 0; j < PANEL_NUMBER_OF_ROWS; ++j) {
		if(concated_bitmap_ptr[j] == NULL)
			return 0;
		row_flag = 1 << (2 + j);
		// правый нижний раряд
		concated_bitmap_ptr[j][first_elem_in_bitmap]  =  row_flag + (first_bitmap_ptr[j] << 8);
		// левый нижний разряд
		concated_bitmap_ptr[j][second_elem_in_bitmap] =  row_flag + (second_bitmap_ptr[j] << 8);
	}

	return 1;
}

_Bool concat_bitmap_1_element(uint16_t (*concated_bitmap_ptr)[NUMBER_OF_DRIVERS], uint8_t bitmap_ptr[ELEMENTS_IN_BITMAP], uint8_t elem_in_bitmap)
{
	int j;
	uint16_t row_flag;

	if((concated_bitmap_ptr == NULL) || (elem_in_bitmap > 2))
			return 0;

	// построчная развертка
	for(j = 0; j < PANEL_NUMBER_OF_ROWS; ++j) {
		if(concated_bitmap_ptr[j] == NULL)
			return 0;
		row_flag 	= 1 << (2 + j);
		concated_bitmap_ptr[j][elem_in_bitmap] = row_flag + (bitmap_ptr[j] << 8);
	}
	return 1;
}
