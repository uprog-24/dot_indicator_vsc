#ifndef DOT_CORE_LED_PANEL_DRIVER_INC_BITMAP_CONCATENATION_H_
#define DOT_CORE_LED_PANEL_DRIVER_INC_BITMAP_CONCATENATION_H_

// ############################################################ FUNCTION PROTOTYPES ###############################################################

/*	@brief 	Сформировать битмап для отправки на вертикальную панель (3 элемента)
 *
 *	@param	uint16_t (*concated_bitmap_ptr)[NUMBER_OF_DRIVERS]  - указатель на двумерный массив с итоговым bitmap'ом
 *			uint8_t top_bitmap_ptr[ELEMENTS_IN_BITMAP]			- указатель на битмап для верхнего элемента
 *			uint8_t bottom_right_bitmap_ptr[ELEMENTS_IN_BITMAP]	- указатель на битмап для нижнего правого элемента
 *			uint8_t bottom_left_bitmap_ptr[ELEMENTS_IN_BITMAP]	- указатель на битмап для нижнего левого элемента
 *
 *	@retVal	1 - bitmap сформирован; 0 - ошибка (nullptr)
 */
_Bool concat_bitmap_3_elements(uint16_t (*concated_bitmap_ptr)[NUMBER_OF_DRIVERS],
							   uint8_t top_bitmap_ptr[ELEMENTS_IN_BITMAP],
							   uint8_t bottom_right_bitmap_ptr[ELEMENTS_IN_BITMAP],
							   uint8_t bottom_left_bitmap_ptr[ELEMENTS_IN_BITMAP]);

/*	@brief 	Сформировать битмап для двух элементов для отправки на вертикальную панель (третий элемент не удаляется)
 *
 *	@param	uint16_t (*concated_bitmap_ptr)[NUMBER_OF_DRIVERS]  - указатель на двумерный массив с итоговым bitmap'ом
 *			uint8_t first_bitmap_ptr[ELEMENTS_IN_BITMAP]		- указатель на битмап для первого элемента
 *			uint8_t firs_elem_in_bitmap							- номер первого элемента в массиве (0, 1 или 2)
 *			second_bitmap_ptr[ELEMENTS_IN_BITMAP]				- указатель на битмап для второго правого элемента
 *			uint8_t second_elem_in_bitmap						- номер второго элемента в массиве (0, 1 или 2)

 *	@retVal	1 - bitmap сформирован; 0 - ошибка (nullptr)
 */
_Bool concat_bitmap_2_elements(uint16_t (*concated_bitmap_ptr)[NUMBER_OF_DRIVERS],
							   uint8_t first_bitmap_ptr[ELEMENTS_IN_BITMAP],
							   uint8_t first_elem_in_bitmap,
							   uint8_t second_bitmap_ptr[ELEMENTS_IN_BITMAP],
							   uint8_t second_elem_in_bitmap);

/*	@brief 	Сформировать битмап для одного элемента для отправки на вертикальную панель (два других элемента не удаляются)
 *
 *	@param	uint16_t (*concated_bitmap_ptr)[NUMBER_OF_DRIVERS]  - указатель на двумерный массив с итоговым bitmap'ом
 *			uint8_t bitmap_ptr[ELEMENTS_IN_BITMAP]				- указатель на битмап для элемента
 *			uint8_t elem_in_bitmap								- номер элемента в массиве (0, 1 или 2)
 *
 *	@retVal	1 - bitmap сформирован; 0 - ошибка (nullptr)
 */
_Bool concat_bitmap_1_element(uint16_t (*concated_bitmap_ptr)[NUMBER_OF_DRIVERS],
							  uint8_t * bitmap_ptr,
							  uint8_t elem_in_bitmap);


#endif /* DOT_CORE_LED_PANEL_DRIVER_INC_BITMAP_CONCATENATION_H_ */
