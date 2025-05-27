#ifndef __LED_PANEL_DRIVER_H
#define __LED_PANEL_DRIVER_H

// #####################################################################
// INCLUDES
// #####################################################################

#include "LED_panel_config.h"
#include "inttypes.h"
#include "stddef.h"
#include "stm32f103xb.h"
// #include "vertical_animation.h"
#include "font.h" // == bitmap.h

#define NUMBER_OF_SYMBOLS SYMBOLS_NUMBER // количество символов в bitmap'е
#define ELEMENTS_IN_BITMAP                                                     \
  PANEL_NUMBER_OF_ROWS // количество элементов в bitmap'e равно количеству рядов

// ##################################################################### TYPEDEF
// #####################################################################
typedef enum {
  TRANSITION_INCREASE,
  TRANSITION_DECREASE,
  TRANSITION_NON,
} transition_type_t;

typedef enum {
  ANIMATION_VERTICAL_CONTINOUS, // уход вверх/ вниз, до отмены
  ANIMATION_VERTICAL_SINGLE, // уход вверх/ вниз, одиночный
  ANIMATION_NONE
} animation_type_t;

// ##################################################################### FP
// #####################################################################
_Bool set_top_element(symbol_code_e symbol, transition_type_t change_type);
_Bool set_bottom_left_element(symbol_code_e symbol,
                              transition_type_t change_type);
_Bool set_bottom_right_element(symbol_code_e symbol,
                               transition_type_t change_type);
_Bool set_bottom_block(symbol_code_e symbol, transition_type_t change_type);
void update_LED_panel();

#endif //__LED_PANEL_DRIVER_H
