

#ifndef INC_MBI5026_DRIVER_H_
#define INC_MBI5026_DRIVER_H_

#include "stddef.h"
#include "stm32f103xb.h"

void LED_driver_set_pins(GPIO_TypeDef* GPIO_LE, uint32_t Pin_LE,
                         GPIO_TypeDef* GPIO_OE, uint32_t Pin_OE);
void LED_driver_send_buffer(uint16_t* buffer_ptr, size_t length);

void LED_driver_impulse_to_latch();

#endif /* INC_MBI5026_DRIVER_H_ */
