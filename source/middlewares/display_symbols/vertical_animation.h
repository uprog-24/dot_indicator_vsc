
#ifndef DOT_CORE_LED_PANEL_DRIVER_INC_VERTICAL_ANIMATION_H_
#define DOT_CORE_LED_PANEL_DRIVER_INC_VERTICAL_ANIMATION_H_

#include <inttypes.h>

_Bool animation_scroll_up (uint8_t *bitmap_to_save_ptr, uint8_t iteration_8_to_0, uint8_t *current_bitmap_ptr, uint8_t *next_bitmap_ptr);
_Bool animation_scroll_down (uint8_t *bitmap_to_save_ptr, uint8_t iteration_8_to_0, uint8_t *current_bitmap_ptr, uint8_t * next_bitmap_ptr);

#endif /* DOT_CORE_LED_PANEL_DRIVER_INC_VERTICAL_ANIMATION_H_ */
