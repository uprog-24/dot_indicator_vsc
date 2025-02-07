
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"

static void SysTick_getDelay(uint32_t ticks);
static uint32_t SysTick_getPrescaler();

// ############################################ PUBLIC FUNCTIONS ############################################
uint32_t SysTick_get_timestamp(uint32_t delay)
{
	uint32_t millis = HAL_GetTick();
	uint32_t ret_val;

	/* защита от переполнения */
	if(delay > (0xFFFFFFFF - millis))
		ret_val = 0xFFFFFFFF;
	else
		ret_val = millis + delay;

	return ret_val;
}

uint32_t SysTick_get_millis()
{
	return HAL_GetTick();
}

void SysTick_delay_us(uint32_t us)
{
	uint32_t SysTickPrescaler = SysTick_getPrescaler();

	volatile uint32_t ticks = (SystemCoreClock / 1000000 / SysTickPrescaler) * us - 1;

	SysTick_getDelay(ticks);
}

void SysTick_delay_ms(uint32_t ms)
{
	uint32_t SysTickPrescaler = SysTick_getPrescaler();
	volatile uint32_t ticks = (SystemCoreClock / 1000 / SysTickPrescaler) * ms - 1;

	SysTick_getDelay(ticks);
}

_Bool SysTick_is_timestamp_over(uint32_t timestamp)
{
	_Bool ret_val = 0;

	if(uwTick > timestamp)
		ret_val = 1;
	return ret_val;
}
// ############################################ PRIVATE FUNCTIONS ############################################
// Лучше заменить на отсчет любым свободным таймером
static void SysTick_getDelay(uint32_t ticks)
{
	uint32_t uwTick_local 	= HAL_GetTick();
	uint32_t startValue 	= SysTick->VAL;
	uint32_t stopValue 		= 0;

	// если будем отсчитывать с переходом через 0
	if(startValue < ticks) {

		uint32_t reloadValue = SysTick->LOAD;

		stopValue = reloadValue - (ticks - startValue);
		// ждем увеличения переменной uwTick (пока счетчик не досчитает до 0)
		while(uwTick == uwTick_local);

	} else {
		stopValue = startValue - ticks;
	}

	while(SysTick->VAL > stopValue) {

		if(uwTick != uwTick_local) {
			return;
		}
	}
}

static uint32_t SysTick_getPrescaler()
{
	// CLKSOURCE: Clock source selection
	// Selects the clock source.
	// 0: AHB/8
	// 1: Processor clock (AHB)
	if( !(SysTick->CTRL & SysTick_CTRL_CLKSOURCE_Msk) ) {
		return 8U;
	} else {
		return 1U;
	}
}
