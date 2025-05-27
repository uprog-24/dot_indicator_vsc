#ifndef DOT_CORE_INDICATION_INDICATION_API_H_
#define DOT_CORE_INDICATION_INDICATION_API_H_

/*	Библиотека предоставляет два режима доступа к панели:
 * 		- раздельный доступ к панелям, отвечающим за этаж + отдельный
 * доступ к панели, отвечающей за стрелку. В этом режиме доступна анимация,
 * 		  предшествующие значения сохраняются
 * 		- управление всеми панелями одновременно. Анимация недоступна,
 * состояние панелей не сохраняется. Применяется для вывода спецрежимов и меню.
 * 		  Также может использоваться для очистки панели
 */

// #####################################################################
// INCLUDES
// ######################################################################
#include "LED_panel_driver.h"
#include "config.h"
// ##################################################################### TYPEDEF
// #######################################################################
typedef enum {
  UFLOOR_MINUS_9 = 40 + 1,
  UFLOOR_MINUS_8,
  UFLOOR_MINUS_7,
  UFLOOR_MINUS_6,
  UFLOOR_MINUS_5,
  UFLOOR_MINUS_4,
  UFLOOR_MINUS_3,
  UFLOOR_MINUS_2,
  UFLOOR_MINUS_1,
  UFLOOR_ZERO_FLOOR,
  /*  КОД ДЛЯ УЛ/ УКЛ
   * 	UFLOOR_UNDERFLOOR_3,
          UFLOOR_UNDERFLOOR_2,
          UFLOOR_UNDERFLOOR_1,
          UFLOOR_UNDERFLOOR,
          UFLOOR_MINUS_FOUR,
          UFLOOR_MINUS_THREE,
          UFLOOR_MINUS_TWO,
          UFLOOR_MINUS_ONE,
  */
  UFLOOR_NUMBER_OF_FLOORS // не использовать и не трогать!
} ufloors_t;

#if 0
typedef enum	{
	ARROWS_NONE	= 0,
	ARROWS_DYNAMIC_DOWN,
	ARROWS_DYNAMIC_UP,
	ARROWS_STATIC_DOWN,
	ARROWS_STATIC_UP,
	ARROWS_STATIC_BOTH
} arrows_t;
#endif

enum movement_dir {
  MDIR_UP,
  MDIR_DOWN,
};

#if 0
enum move_direction
{
	MDIR_STOPPED,
	MDIR_MOVEMENT_UP,
	MDIR_MOVEMENT_DOWN,
	MDIR_STOP_THEN_UP,
	MDIR_STOP_THEN_DOWN,
	MDIR_STOP_THEN_ANY
};
#endif

// ################################################################## PUBLIC
// FUNCTIONS #################################################################
/*	Функция вызывает отрисовку экрана.
 *  Также функция регирует на изменение режима
 *  работы и учавствует в задержке выдачи стрелки
 * 	направления движения
 *
 * 	@param	NONE
 *
 * 	@retVal false - ошибка при обновлении режима
 */
_Bool indication_routine();
/*	@brief	Функция устанавливает номер этажа,
 * 			который будет выводиться на LED - панель.
 *
 *	@param	l_elem	- левый символ этажа
 *			r_elem  - правый символ этажа
 *			is_animation_allowed:
 *					  0 - не анимировать переход
 *					  1 - можно анимировать переход
 *
 *	@retVal true - значение этажа установлено/
 *			false - ошибка (некорректный номер этажа)
 */
_Bool indication_set_floor(symbol_code_e l_elem, symbol_code_e r_elem,
                           _Bool is_animation_allowed);
/*	@brief	Функция очищает все
 *			панели индикатора
 *
 *	@param	none
 *
 *	@retVal 1 - индикатор очищен/
 *			0 - ошибка
 */
_Bool indication_clear_all_panels();
/*	@brief	Функция устанавливает элементы,
 * 			которые будут выводиться на LED
 * 			- панель в режиме меню.
 *
 *	@param	top_elem  	- верхний элемент панели
 *			left_elem 	- нижний левый элемент панели
 *			right_elem  - нижний правый
 *
 *	@retVal true -  значение адреса установлено/
 *			false - ошибка (некорректный номер адреса)
 */
// _Bool indication_set_full_panel(symbol_code_e top_elem, symbol_code_e
// left_elem,
//                                 symbol_code_e right_elem, _Bool
//                                 is_menu_mode);
_Bool indication_set_full_panel(symbol_code_e top_elem, symbol_code_e left_elem,
                                symbol_code_e right_elem);
/*	@brief	Функция очищает панель
 * 			индикатора, на которой отображается
 * 			стрелка
 *
 *	@param	none
 *
 *	@retVal 1 - стрелка очищена/
 *			0 - ошибка
 */
_Bool indication_clear_arrow();
/*	@brief	Функция устанавливает динамическую
 * 			стрелку или очищает поле со стрелкой
 * 			если dyn_arrow_dir = ARROWS_NONE
 *
 *	@param	arrow_elem 	- значение для динамической
 *					 	  стрелки
 *			mov_dir		- направление движения лифта
 *						  (необходимо для анимации)
 *			is_dynamic	- 1 - анимировать стрелку;
 *						  0 - запрет на анимацию
 *
 *	@retVal 1 - значение стрелки установлено/
 *			0 - ошибка
 */
_Bool indication_set_dynamic_arrow(symbol_code_e arrow_elem,
                                   enum movement_dir mov_dir, _Bool is_dynamic);
/*	@brief	Функция устанавливает статическую
 * 			стрелку или очищает поле со стрелкой
 * 			если st_arrow_dir = ARROWS_NONE
 *
 *	@param	arrow_elem 		- значение статической
 *							  стрелки
 *			fade_delay_ms	- (1 - 0xFFFF) - время в мс,
 *							  по прошествии которого
 *							  нужно очистить
 *стрелку. (0) - если статическую стрелку очищать не нужно
 *
 *	@retVal 1 - значение стрелки установлено/
 *			0 - ошибка (некорректный номер стрелки)
 *
 */
_Bool indication_set_static_arrow(symbol_code_e arrow_elem,
                                  uint16_t fade_delay_ms);
/*	@brief	Функция устанавливает на панели
 * 			значения этажа и стрелки.
 * 			Запускается из режима меню.
 *
 *	@param	none
 *
 *	@retVal 1 - знаечение панели установлено/
 *			0 - ошибка
 */
_Bool indication_exit_menu();

#endif /* DOT_CORE_INDICATION_INDICATION_API_H_ */
