#ifndef INC_SOFTWARE_SPI_H_
#define INC_SOFTWARE_SPI_H_

// ################################################ INCLUDES ################################################
#include "stm32f103xb.h"

// ################################################ DEFINES ################################################
#define BIT_DURATION	2

// ################################################## FP ###################################################

void software_SPI_addPins(GPIO_TypeDef * GPIO_MOSI, uint32_t Pin_MOSI, GPIO_TypeDef * GPIO_CLK, uint32_t Pin_CLK );
void software_SPI_sendByte(uint8_t byteToSend);



#endif /* INC_SOFTWARE_SPI_H_ */
