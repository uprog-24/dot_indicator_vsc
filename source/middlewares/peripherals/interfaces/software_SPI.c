
// ################################################ INCLUDES ###############################################
#include "SysTick_Delay.h"
//#include "GPIO_driver.h"
#include "software_SPI.h"

// ################################################ STRUCTS ################################################
typedef struct
{
	GPIO_TypeDef * 	GPIO;
	uint32_t		Pin;
} GPIO_pin_t;

// ############################################# VERIABLES #################################################
static GPIO_pin_t	MOSI_pin 	= { 0 };
static GPIO_pin_t	CLK_pin 	= { 0 };

extern uint32_t SystemCoreClock;

// ##################################### STATIC FUNCTIONS PROTOTYPES ########################################
static void software_SPI_writeStructures(GPIO_pin_t * struct_ptr, GPIO_TypeDef * GPIO, uint32_t Pin);
static inline void MOSI_toHigh();
static inline void MOSI_toLow();
static inline void CLK_toHigh();
static inline void CLK_toLow();

// ############################################ PUBLIC FUNCTIONS ############################################

/* @brief функция принимает и записывает указатель на хэндлер пинов
 *
 *
 */
void software_SPI_addPins(GPIO_TypeDef * GPIO_MOSI, uint32_t Pin_MOSI, GPIO_TypeDef * GPIO_CLK, uint32_t Pin_CLK)
{
	software_SPI_writeStructures(&MOSI_pin, GPIO_MOSI, Pin_MOSI);
	software_SPI_writeStructures(&CLK_pin,  GPIO_CLK,  Pin_CLK);
}
/* @brief функция принимает бит для выставления на линию
 *
 *
 */
void software_SPI_sendByte(uint8_t byteToSend)
{
	CLK_toLow();
	//GPIO_resetPin(CLK_pin.GPIO, CLK_pin.Pin);
	// счетчик бит в посылке
	for(int i = 0; i < 8; ++i)
	{
		// выставляем текущий бит на линию MOSI
		if( (byteToSend & (1 << i)) )
			MOSI_toHigh();
		else
			MOSI_toLow();

		SysTick_delay_us(BIT_DURATION/2);
		CLK_toHigh();
		SysTick_delay_us(BIT_DURATION/2);
		CLK_toLow();
	}

	MOSI_toLow();
}

void software_SPI_sendTwoBytes(uint16_t twoBytesToSend)
{

}

// ########################################### STATIC FUNCTIONS #############################################

static void software_SPI_writeStructures(GPIO_pin_t * struct_ptr, GPIO_TypeDef * GPIO, uint32_t Pin)
{
	struct_ptr->GPIO 	= GPIO;
	struct_ptr->Pin 	= Pin;
}

static inline void MOSI_toHigh()
{
	MOSI_pin.GPIO->BSRR = MOSI_pin.Pin;
}

static inline void MOSI_toLow()
{
	MOSI_pin.GPIO->BRR = MOSI_pin.Pin;
}

static inline void CLK_toHigh()
{
	CLK_pin.GPIO->BSRR = CLK_pin.Pin;
}

static inline void CLK_toLow()
{
	CLK_pin.GPIO->BRR = CLK_pin.Pin;
}

//-------------------------------------------------------------------------------------------------------------

