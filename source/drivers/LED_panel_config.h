#ifndef DOT_CORE_LED_PANEL_DRIVER_INC_LED_PANEL_CONFIG_H_
#define DOT_CORE_LED_PANEL_DRIVER_INC_LED_PANEL_CONFIG_H_

	// ####################### Настройки Панели ########################
			// выбор типа индикатора
			#define PANEL_IS_VERTICAL
			// #define PANEL_IS_HORIZONTAL		// не реализована

			// выбор типа драйвер
			//#define DRIVER_MBI5026
			#define DRIVER_STP16

	// #################################################################

	#include "LED_driver.h"

	#if defined(PANEL_IS_VERTICAL)
			#define PANEL_NUMBER_OF_COLUMNS		7
			#define PANEL_NUMBER_OF_ROWS		6
			#define	NUMBER_OF_DRIVERS			3
			#define VERTICAL_ANIMATION_CYCLES	(PANEL_NUMBER_OF_COLUMNS + 1)
	/* не реализовано */
	#elif defined(PANEL_IS_HORIZONTAL)
			#define PANEL_NUMBER_OF_COLUMNS		TO_ADD
			#define PANEL_NUMBER_OF_ROWS		TO_ADD
	#endif


#endif /* DOT_CORE_LED_PANEL_DRIVER_INC_LED_PANEL_CONFIG_H_ */
