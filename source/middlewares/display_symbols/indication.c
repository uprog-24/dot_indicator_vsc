// #####################################################################
// INCLUDES
// ######################################################################
#include "indication.h"
// #include "state_machine.h"
#include "SysTick_Delay.h"
#include "config.h"

// ##################################################################### DEFINES
// #######################################################################
#define DELAY_BETWEEN_STATIC_AND_DYNAMIC_ARROWS_MS 150
#define STATIC_ARROW_DELAY_MS 5000
// ####################################################################### ENUM
// #######################################################################
enum arrow_panel_status {
  APS_MOVING_UP_ARROW = MDIR_UP,
  APS_MOVING_DOWN_ARROW = MDIR_DOWN,
  /*	любая стрелка направления
   * 	движения после остановки
   */
  AS_AFTERSTOP_ARROW,
  /*	панель со стрелкой
   * 	очищена
   */
  AS_CLEAR_ARROW_PANEL
};

// ##################################################################### STRUCTS
// #######################################################################
struct floor_indication {
  symbol_code_e t_elem;
  symbol_code_e l_elem;
  symbol_code_e r_elem;
  enum arrow_panel_status arrow_status;
  uint32_t afterstop_arrow_stamp; // для организации задержки при смене стрелки
                                  // со статической на ARROWS_NONE
};
// #####################################################################
// VERIABLES
// #####################################################################
static struct floor_indication floor_displaying = {.t_elem = SYMBOL_EMPTY,
                                                   .l_elem = SYMBOL_EMPTY,
                                                   .r_elem = SYMBOL_EMPTY,
                                                   .arrow_status =
                                                       AS_CLEAR_ARROW_PANEL,
                                                   .afterstop_arrow_stamp = 0};
// ##################################################################### FP
// ############################################################################
static _Bool update_floor(struct floor_indication *fdisplaying_p,
                          symbol_code_e l_elem, symbol_code_e r_elem,
                          _Bool is_animation_allowed);
static transition_type_t
get_transition_by_direction(enum arrow_panel_status ap_status,
                            _Bool is_inverted);
// ################################################################## PUBLIC
// FUNCTIONS #################################################################

_Bool indication_routine() {
  /* отключение стрелки направления движения */
  if ((floor_displaying.arrow_status == AS_AFTERSTOP_ARROW) &&
      (floor_displaying.afterstop_arrow_stamp != 0)) {
    if (SysTick_get_millis() >= floor_displaying.afterstop_arrow_stamp)
      indication_clear_arrow();
  }
  update_LED_panel();
  return 1;
}

#if 0
_Bool indication_exit_menu() {
  transition_type_t arrow_animation;
  const symbol_code_e symbol_clear_btmp = SYMBOL_CLEAN_BITMAP;
  const transition_type_t no_animation = TRANSITION_NON;
  machine_states_t c_state;
  _Bool ret;

  c_state = state_get_state();
  if (c_state != STATE_MENU_MODE)
    return 0;
  arrow_animation =
      get_transition_by_direction(floor_displaying.arrow_status, 0);
  if (floor_displaying.t_elem != symbol_clear_btmp)
    ret = set_top_element(floor_displaying.t_elem, arrow_animation);
  else
    ret = set_top_element(symbol_clear_btmp, no_animation);
#if defined(config_SPLIT_SYMBOL)
  if (floor_displaying.l_elem == symbol_clear_btmp)
    ret &= set_bottom_block(floor_displaying.r_elem, TRANSITION_NON);
  else {
#endif
    ret &= set_bottom_left_element(floor_displaying.l_elem, no_animation);
    ret &= set_bottom_right_element(floor_displaying.r_elem, no_animation);
#if defined(config_SPLIT_SYMBOL)
  }
#endif
  return ret;
  // set_top_element(floor_displaying.t_elem, arrow_animation);
  // update_floor(&floor_displaying, floor_displaying.l_elem,
  // floor_displaying.r_elem, 0);
}
#endif

_Bool indication_set_floor(symbol_code_e l_elem, symbol_code_e r_elem,
                           _Bool is_animation_allowed) {

  /* некорректные входные значения */
  if ((l_elem >= SYMBOLS_NUMBER) || (r_elem >= SYMBOLS_NUMBER))
    return 0;
  /* аналогичные значения уже установлены */
  if ((l_elem == floor_displaying.l_elem) &&
      (r_elem == floor_displaying.r_elem))
    return 1;
  //   if (state_get_state() == STATE_DISPLAYING_FLOOR) {
  if (update_floor(&floor_displaying, l_elem, r_elem, is_animation_allowed) ==
      0)
    return 0;
  //   }
  floor_displaying.l_elem = l_elem;
  floor_displaying.r_elem = r_elem;
  return 1;
}

_Bool indication_clear_arrow() {
  const transition_type_t not_animated = TRANSITION_NON;

  if ((floor_displaying.t_elem == SYMBOL_EMPTY) &&
      (floor_displaying.arrow_status == AS_CLEAR_ARROW_PANEL))
    return 1;
  //   if (state_get_state() == STATE_DISPLAYING_FLOOR)
  if (set_top_element(SYMBOL_EMPTY, not_animated) == 0)
    return 0;
  floor_displaying.arrow_status = AS_CLEAR_ARROW_PANEL;
  floor_displaying.t_elem = SYMBOL_EMPTY;
  return 1;
}

_Bool indication_set_dynamic_arrow(symbol_code_e arrow_elem,
                                   enum movement_dir mov_dir,
                                   _Bool is_dynamic) {
  const enum movement_dir dir_max_val = MDIR_DOWN;
  transition_type_t animation_type = TRANSITION_NON;

  /* некорректные входные значения */
  if ((arrow_elem >= SYMBOLS_NUMBER) || (mov_dir > dir_max_val))
    return 0;
  /* уже установлено */
  if ((floor_displaying.t_elem == arrow_elem) &&
      (floor_displaying.arrow_status == (enum arrow_panel_status)mov_dir))
    return 1;
  if (is_dynamic == 1)
    animation_type =
        get_transition_by_direction((enum arrow_panel_status)mov_dir, 0);
  //   if (state_get_state() == STATE_DISPLAYING_FLOOR)
  if (set_top_element(arrow_elem, animation_type) == 0)
    return 0;
  floor_displaying.arrow_status = (enum arrow_panel_status)mov_dir;
  floor_displaying.t_elem = arrow_elem;
  return 1;
}

_Bool indication_set_static_arrow(symbol_code_e arrow_elem,
                                  uint16_t fade_delay_ms) {
  const transition_type_t not_animated = TRANSITION_NON;

  /* некорректные входные значения */
  if (arrow_elem >= SYMBOLS_NUMBER)
    return 0;
  /* уже установлено */
  if ((floor_displaying.t_elem == arrow_elem) &&
      (floor_displaying.arrow_status == AS_AFTERSTOP_ARROW))
    return 1;

  //   if (state_get_state() == STATE_DISPLAYING_FLOOR)
  if (set_top_element(arrow_elem, not_animated) == 0)
    return 0;
  floor_displaying.arrow_status = AS_AFTERSTOP_ARROW;
  floor_displaying.t_elem = arrow_elem;
  if (fade_delay_ms != 0)
    floor_displaying.afterstop_arrow_stamp =
        SysTick_get_timestamp((uint32_t)fade_delay_ms);
  else
    floor_displaying.afterstop_arrow_stamp = 0;
  return 1;
}

#if 1
_Bool indication_set_full_panel(symbol_code_e top_elem, symbol_code_e left_elem,
                                symbol_code_e right_elem) {
  indication_set_static_arrow(top_elem, 0);
  indication_set_floor(left_elem, right_elem, 0);
}
#endif

#if 0
_Bool indication_set_full_panel(symbol_code_e top_elem, symbol_code_e left_elem,
                                symbol_code_e right_elem, _Bool is_menu_mode) {
  //   machine_states_t state;
  _Bool ret = 0;
  /* некорректные входные значения */
  if ((left_elem >= SYMBOLS_NUMBER) || (right_elem >= SYMBOLS_NUMBER) ||
      (top_elem >= SYMBOLS_NUMBER))
    return 0;
  //   state = state_get_state();
  if (((is_menu_mode) && (state == STATE_MENU_MODE)) ||
      ((!is_menu_mode) && (state == STATE_DISPLAYING_FLOOR))) {
    set_top_element(top_elem, TRANSITION_NON);
#if defined(config_SPLIT_SYMBOL)
    if (left_elem == clean_btmp)
      set_bottom_block(right_elem, TRANSITION_NON);
    else {
#endif
      set_bottom_left_element(left_elem, TRANSITION_NON);
      set_bottom_right_element(right_elem, TRANSITION_NON);
#if defined(config_SPLIT_SYMBOL)
    }
#endif
    floor_displaying.arrow_status = AS_CLEAR_ARROW_PANEL;
    ret = 1;
  }

  return ret;
}
#endif

#if 0
_Bool indication_set_full_panel(symbol_code_e top_elem, symbol_code_e left_elem,
                                symbol_code_e right_elem) {
  indication_set_static_arrow(top_elem);
  indication_set_floor(left_elem, right_elem);
}
#endif

_Bool indication_clear_all_panels() {
  const symbol_code_e symbol_clear_btmp = SYMBOL_EMPTY;
  _Bool ret;

  ret = set_top_element(SYMBOL_EMPTY, TRANSITION_NON);
  ret &= set_bottom_left_element(SYMBOL_EMPTY, TRANSITION_NON);
  ret &= set_bottom_right_element(SYMBOL_EMPTY, TRANSITION_NON);

  floor_displaying.t_elem = symbol_clear_btmp;
  floor_displaying.l_elem = symbol_clear_btmp;
  floor_displaying.r_elem = symbol_clear_btmp;
  floor_displaying.arrow_status = AS_CLEAR_ARROW_PANEL;

  return ret;
}
// ################################################################## PRIVATE
// FUNCTIONS ################################################################
static transition_type_t
get_transition_by_direction(enum arrow_panel_status ap_status,
                            _Bool is_inverted) {
  transition_type_t ret = TRANSITION_NON;

  if (ap_status == APS_MOVING_UP_ARROW)
    ret = (is_inverted == 0) ? TRANSITION_INCREASE : TRANSITION_DECREASE;
  else if (ap_status == APS_MOVING_DOWN_ARROW)
    ret = (is_inverted == 0) ? TRANSITION_DECREASE : TRANSITION_INCREASE;
  return ret;
}

static _Bool update_floor(struct floor_indication *fdisplaying_p,
                          symbol_code_e l_elem, symbol_code_e r_elem,
                          _Bool is_animation_allowed) {
  transition_type_t transition_type = TRANSITION_NON;

  if (is_animation_allowed == 1)
    transition_type =
        get_transition_by_direction(fdisplaying_p->arrow_status, 1);
#if defined(config_SPLIT_SYMBOL)
  if (l_elem == SYMBOL_EMPTY) {
    return set_bottom_block(r_elem, transition_type);
  }
#endif
  if (fdisplaying_p->l_elem != l_elem)
    if (set_bottom_left_element(l_elem, transition_type) == 0)
      return 0;
  if (set_bottom_right_element(r_elem, transition_type) == 0)
    return 0;
  return 1;
}
// ################################################################## OLD CODE
// ################################################################
#if 0
_Bool indication_set_underfloor(symbol_table_t l_elem, uint8_t ufloor_num)
{
	/* некорректные входные значения */
	if(((ufloor_num > 9) && (ufloor_num != SYMBOL_CLEAN_BITMAP)) ||
	   (l_elem >= SYMBOL_CLEAN_BITMAP))
		return 0;
	/* если этаж уже установлен - возвращаем 1 */
	if((floor_displaying.is_underfloor == 1) &&
	   (l_elem == floor_displaying.l_elem) &&
	   (ufloor_num == floor_displaying.floor))
		return 1;
	/* возвращаем 0, если не удалось установить символы */
	if(state_get_state() == STATE_DISPLAYING_FLOOR)
		if(update_underfloor(l_elem, floor_displaying.l_elem, ufloor_num, floor_displaying.arrow, 1) == 0)
			return 0;
	/* обновляем значения если:
	 *  	-state_get_state() != STATE_DISPLAYING_FLOOR или
	 *  	-state_get_state() == STATE_DISPLAYING_FLOOR и
	 *  	update_underfloor вернул 1
	 */
	floor_displaying.is_underfloor 	= 1;
	floor_displaying.l_elem 	= l_elem;
	floor_displaying.floor 			= ufloor_num;
	return 1;
}

/*	Функция выводит на экран символы меню, если устройство находится в режиме меню
 *
 * 	@param	symbol_table_t top_symbol 				- символ, который следует вывести в верхний элемент панели
 * 			_Bool is_centered		  				- вывести элемент в lbottom_or_cent_symbol в середину нижней панели
 * 			symbol_table_t lbottom_or_cent_symbol 	- символ, который будет выводится в левый нижний элемент (при is_centered = 0) или в середину нижней панели (при is_centered = 1)
 * 			symbol_table_t rbottom_symbol   		- символ, который будет выводится в правый нижний элемент. При is_centered = 1 значение игнорируется
 *
 * 	@retVal true 									- значение символов установлены/ false - ошибка
 *
 */
_Bool indication_set_menu_symbols(symbol_table_t top_symbol, _Bool is_centered, symbol_table_t lbottom_or_cent_symbol, symbol_table_t rbottom_symbol)
{
	machine_states_t current_state = state_get_state();
	_Bool ret_val = 1;

	if(current_state != STATE_MENU_MODE)
		return 0;

	ret_val = write_panel_with_3_symbols(top_symbol, is_centered, lbottom_or_cent_symbol, rbottom_symbol);
	floor_displaying.last_state = current_state;

	return ret_val;
}

static _Bool write_panel_with_3_symbols(symbol_table_t top_symbol, _Bool is_centered, symbol_table_t lbottom_or_cent_symbol, symbol_table_t rbottom_symbol)
{
	if(!set_top_element(top_symbol, (transition_type_t)TRANSITION_NON))
		return 0;

	/* если хотим поместить символ в центр нижней панели*/
	if(is_centered)
		return set_bottom_block(lbottom_or_cent_symbol, (transition_type_t)TRANSITION_NON);

	/* если нижняя панель используется для вывода двух разрядов*/
	if(set_bottom_left_element (lbottom_or_cent_symbol, (transition_type_t)TRANSITION_NON) == 0)
		return 0;

	if(set_bottom_right_element (rbottom_symbol, (transition_type_t)TRANSITION_NON) == 0)
		return 0;

	return 1;
}

static void indication_set_static_arrow()
{
	if(floor_displaying.arrow == ARROWS_STATIC_DOWN)
		set_top_element(SYMBOL_ARROW_DOWN, TRANSITION_NON);
	else if(floor_displaying.arrow == ARROWS_STATIC_UP)
		set_top_element(SYMBOL_ARROW_UP, TRANSITION_NON);
	else if(floor_displaying.arrow == ARROWS_STATIC_BOTH)
		set_top_element(SYMBOL_ARROW_BOTH, TRANSITION_NON);
}

// устанавливаем задержку на включение статической стрелки (даже если в разряде стрелки пусто)
static _Bool indication_set_static_arrow_delay()
{
	if(set_top_element (SYMBOL_CLEAN_BITMAP, TRANSITION_NON)) {
		floor_displaying.stamp_delay_1 = SysTick_get_millis() + DELAY_BETWEEN_STATIC_AND_DYNAMIC_ARROWS_MS;
		return 1;
	}

	return 0;
}

static _Bool indication_update_arrow(arrows_t arrow)
{
	symbol_table_t 		arrow_symbol;
	transition_type_t	transition_type = TRANSITION_NON;

	switch(arrow) {
		case(ARROWS_NONE):
			arrow_symbol = SYMBOL_CLEAN_BITMAP;
			break;
		case(ARROWS_DYNAMIC_UP):
			arrow_symbol = SYMBOL_ARROW_ANIMATION_UP_DYNAMIC;
			transition_type = TRANSITION_INCREASE;
			break;
		case(ARROWS_DYNAMIC_DOWN):
			arrow_symbol = SYMBOL_ARROW_ANIMATION_DOWN_DYNAMIC;
			transition_type = TRANSITION_DECREASE;
			break;
		case(ARROWS_STATIC_DOWN):
			arrow_symbol = SYMBOL_ARROW_DOWN;
			break;
		case(ARROWS_STATIC_UP):
			arrow_symbol = SYMBOL_ARROW_UP;
			break;
		case(ARROWS_STATIC_BOTH):
			arrow_symbol = SYMBOL_ARROW_BOTH;
			break;
		default:
			return 0;
	}

	return set_top_element(arrow_symbol, transition_type);
	/* ДЛЯ УЛ/ УКЛ
	switch(arrow) {

		case(ARROWS_NONE):
			return set_top_element(SYMBOL_CLEAN_BITMAP, TRANSITION_NON);

		case(ARROWS_DYNAMIC_UP):
			return set_top_element(SYMBOL_ARROW_ANIMATION_UP_DYNAMIC, TRANSITION_INCREASE);

		case(ARROWS_DYNAMIC_DOWN):
			return set_top_element(SYMBOL_ARROW_ANIMATION_DOWN_DYNAMIC, TRANSITION_DECREASE);

		// break не добавил специально
		case(ARROWS_STATIC_DOWN):
		case(ARROWS_STATIC_UP):
		case(ARROWS_STATIC_BOTH):
			return indication_set_static_arrow_delay();

		default:
			return 0;
	}
	*/
}

static _Bool update_underfloor(symbol_table_t l_elem, symbol_table_t prev_ufloor_sign, symbol_table_t ufloor_num, arrows_t arrow, _Bool is_animation)
{
	const symbol_table_t	clean_btmp		= SYMBOL_CLEAN_BITMAP;
	transition_type_t  		transition_type = TRANSITION_NON;
	symbol_table_t			left_elem		= SYMBOL_CLEAN_BITMAP;
	/*	обновляем левый символ только если
	 *  он отличается от текущего
	 */
	if(ufloor_sign != prev_ufloor_sign)
		left_elem = ufloor_sign;
	if(is_animation)
		transition_type = get_transition_type(arrow);
	return indication_write_floor_symbols(left_elem, ufloor_num, clean_btmp, transition_type);
}

/* Для станций УЛ/ УКЛ
// Проверяет, нужно ли обновлять символ "П" при отображении подземных этажей
// 1 - не обновлять
// 0 - обовлять
static _Bool is_underfloor_symbol_ignored(uint8_t prev_floor)
{
	_Bool ret_val = 0;

	if((prev_floor == UFLOOR_UNDERFLOOR_2) || (prev_floor == UFLOOR_UNDERFLOOR_1))
		ret_val = 1;

	return ret_val;
}
*/
// Проверяет, нужно ли обновлять символ "-" при отображении подземных этажей
// 1 - не обновлять
// 0 - обовлять
static _Bool is_minus_symbol_ignored(uint8_t prev_floor)
{
	_Bool ret_val = 0;

	switch(prev_floor) {
		case(UFLOOR_MINUS_1):
			ret_val = 1;
			break;
		case(UFLOOR_MINUS_2):
			ret_val = 1;
			break;
		case(UFLOOR_MINUS_3):
			ret_val = 1;
			break;
		case(UFLOOR_MINUS_4):
			ret_val = 1;
			break;
		case(UFLOOR_MINUS_5):
			ret_val = 1;
			break;
		case(UFLOOR_MINUS_6):
			ret_val = 1;
			break;
		case(UFLOOR_MINUS_7):
			ret_val = 1;
			break;
		case(UFLOOR_MINUS_8):
			ret_val = 1;
			break;
		case(UFLOOR_MINUS_9):
			ret_val = 1;
			break;
		default:
			break;
	}

	/* Код для УЛ/ УКЛ
	if((prev_floor == UFLOOR_MINUS_FOUR)  || (prev_floor == UFLOOR_MINUS_THREE) ||
	   (prev_floor == UFLOOR_MINUS_TWO)   || (prev_floor == UFLOOR_MINUS_ONE))
			ret_val = 1;
	*/
	return ret_val;
}

static void write_symbols_for_underfloors (uint8_t current_floor, uint8_t prev_floor, symbol_table_t *left_elem, symbol_table_t *right_elem, symbol_table_t *centered_elem)
{
	uint8_t floor;

	if(current_floor == UFLOOR_ZERO_FLOOR) {
		*centered_elem		= SYMBOL_DIGIT_ZERO;
	} else {
		if(!is_minus_symbol_ignored(prev_floor))
			*left_elem		= SYMBOL_MINUS_SIGN;

		/* вычисляем номер подземного этажа */
		floor = 9 - (current_floor - UFLOOR_MINUS_9);
		*right_elem		= SYMBOL_DIGIT_ZERO + floor;
	}


/* КОД ДЛЯ УЭЛ/ УКЛ
	switch(current_floor) {

		case(UFLOOR_ZERO_FLOOR):
			*centered_elem		= SYMBOL_DIGIT_ZERO;
			break;

		case(UFLOOR_UNDERFLOOR):
			*centered_elem		= SYMBOL_UNDERFLOOR;
			break;

		case(UFLOOR_UNDERFLOOR_1):
			if(!is_underfloor_symbol_ignored(prev_floor))
				*left_elem	= SYMBOL_UNDERFLOOR;

			*right_elem		= SYMBOL_DIGIT_ONE;
			break;

		case(UFLOOR_UNDERFLOOR_2):
			if(!is_underfloor_symbol_ignored(prev_floor))
				*left_elem	= SYMBOL_UNDERFLOOR;

			*right_elem		= SYMBOL_DIGIT_TWO;
			break;

		case(UFLOOR_MINUS_ONE):
			if(!is_minus_symbol_ignored(prev_floor))
				*left_elem		= SYMBOL_MINUS_SIGN;

			*right_elem		= SYMBOL_DIGIT_ONE;
			break;

		case(UFLOOR_MINUS_TWO):
			if(!is_minus_symbol_ignored(prev_floor))
				*left_elem		= SYMBOL_MINUS_SIGN;

			*right_elem		= SYMBOL_DIGIT_TWO;
			break;

		case(UFLOOR_MINUS_THREE):
			if(!is_minus_symbol_ignored(prev_floor))
				*left_elem		= SYMBOL_MINUS_SIGN;

			*right_elem		= SYMBOL_DIGIT_THREE;
			break;

		case(UFLOOR_MINUS_FOUR):
			if(!is_minus_symbol_ignored(prev_floor))
				*left_elem		= SYMBOL_MINUS_SIGN;

			*right_elem		= SYMBOL_DIGIT_FOUR;
			break;
	}
*/
}

static inline void indication_get_floor_symbols_w_centered(uint8_t floor, symbol_table_t *left_elem, symbol_table_t *right_elem, symbol_table_t *centered_element)
{
	if(floor < 10)
		*centered_element = (symbol_table_t) floor;
#if 0
	else if(floor > MAX_FLOOR /* && (curr_floor < UFLOOR_NUMBER_OF_FLOORS )*/)
		write_symbols_for_underfloors (floor, floor_displaying.floor, left_elem, right_elem, centered_element);
#endif
	else /* if( (curr_floor < MAX_FLOOR)) */
		write_symbols_for_two_digit_floors (floor, floor_displaying.floor, left_elem, right_elem);
}
#endif
#if 0
static void write_symbols_for_two_digit_floors (uint8_t current_floor, uint8_t prev_floor, symbol_table_t *left_elem, symbol_table_t *right_elem)
{
	uint8_t prev_floor_tens = prev_floor/10;
	uint8_t curr_floor_tens = current_floor/10;

	if(prev_floor_tens != curr_floor_tens)
		*left_elem = curr_floor_tens;

	*right_elem = current_floor%10;
}

static inline _Bool indication_write_floor_symbols(symbol_table_t left_elem, symbol_table_t right_elem, symbol_table_t centered_element, transition_type_t transition_type)
{
	if(centered_element != (symbol_table_t) ~0) {
		return set_bottom_block(centered_element, transition_type);

	} else /* if(centered_element == ~0)*/ {
		if(left_elem != (symbol_table_t) ~0)
			if(!set_bottom_left_element(left_elem, transition_type))
				return 0;
		if(right_elem != (symbol_table_t) ~0)
			if(!set_bottom_right_element(right_elem, transition_type))
				return 0;
		return 1;
	}
}

	/* для УЛ/ УКЛ
	// TODO: вынести в отдельную функцию
	// обработка отложенной статической стрелки
	if(floor_displaying.stamp_delay_1 != 0) {

		if(	(floor_displaying.arrow != ARROWS_STATIC_UP)   &&
			(floor_displaying.arrow != ARROWS_STATIC_DOWN) &&
			(floor_displaying.arrow != ARROWS_STATIC_BOTH)) {
				floor_displaying.stamp_delay_1 = 0;
		} else {
			if(SysTick_get_millis() >= floor_displaying.stamp_delay_1) {
				indication_set_static_arrow();
				floor_displaying.stamp_delay_2 = SysTick_get_millis() + STATIC_ARROW_DELAY_MS;
				floor_displaying.stamp_delay_1 = 0;
			}
		}
	}

	if(floor_displaying.stamp_delay_2 != 0) {

		if(	(floor_displaying.arrow != ARROWS_STATIC_UP)   &&
			(floor_displaying.arrow != ARROWS_STATIC_DOWN) &&
			(floor_displaying.arrow != ARROWS_STATIC_BOTH)) {
				floor_displaying.stamp_delay_2 = 0;
		} else {
			if(SysTick_get_millis() >= floor_displaying.stamp_delay_2) {
				indication_set_arrow(ARROWS_NONE);
				floor_displaying.stamp_delay_2 = 0;
			}
		}
	}
*/

_Bool indication_set_dynamic_arrow(enum movement_dir arrow_dir)
{
	symbol_table_t get_arrow_by_direction(_Bool is_dynamic, enum movement_dir arrow_dir)

	if(arrow == floor_displaying.arrow)
		return 1;
/* TODO: переделать */
	if(state_get_state() == STATE_DISPLAYING_FLOOR){
		// если успешно установили стрелку, обновляем значение в переменной
		if(indication_update_arrow(dyn_arrow_dir)) {
			floor_displaying.arrow = dyn_arrow_dir;
			return 1;
		} else {
			return 0;
		}
	}
}

_Bool indication_set_static_arrow(arrows_t st_arrow_dir, uint16_t fade_delay_ms)
{




}

	/* если этаж уже установлен - возвращаем 1 */
	if((floor_displaying.is_underfloor == 0) &&
	   (floor == floor_displaying.floor))
		return 1;
	/* возвращаем 0, если не удалось установить символы */
	if(state_get_state() == STATE_DISPLAYING_FLOOR)
		if(update_floor(floor,  floor_displaying.arrow,  1) == 0)
			return 0;
	/* обновляем значения если:
	 *  	-state_get_state() != STATE_DISPLAYING_FLOOR или
	 *  	-state_get_state() == STATE_DISPLAYING_FLOOR и
	 *  	update_floor вернул 1
	 */
	floor_displaying.is_underfloor = 0;
	floor_displaying.floor 		   = floor;
	return 1;
	}

static transition_type_t get_transition_type(enum arrow_panel_status ap_status)
{
	transition_type_t ret = TRANSITION_NON;

	if(ap_status == APS_MOVING_DOWN_ARROW)
		ret = TRANSITION_INCREASE;
	else if(ap_status == APS_MOVING_UP_ARROW)
		ret = TRANSITION_DECREASE;

	return ret;
}
#endif
