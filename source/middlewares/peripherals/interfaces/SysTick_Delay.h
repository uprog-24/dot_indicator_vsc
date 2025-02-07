
#ifndef INC_SYSTICK_DELAY_H_
#define INC_SYSTICK_DELAY_H_

#include <inttypes.h>

void SysTick_delay_us(uint32_t us);
void SysTick_delay_ms(uint32_t ms);
uint32_t SysTick_get_millis();
uint32_t SysTick_get_timestamp(uint32_t delay);
_Bool SysTick_is_timestamp_over(uint32_t timestamp);
#endif /* INC_SYSTICK_DELAY_H_ */
