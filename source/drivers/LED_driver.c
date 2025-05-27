// ################################################ INCLUDES
// ###############################################
#include "LED_driver.h"
#include <SysTick_Delay.h>
#include "LED_panel_driver.h"
#include "LED_panel_config.h"
#include "main.h"
#include "software_SPI.h"

//#define config_MU_IT_05_10
//#define config_MU_IT_05_10
#define config_MU_IT_06_10
/*
 * 		Структура пакета для одного регистра MBI5026
 *		Для одного из разрядов МЮ.ИТ.05.10, МЮ.ИТ.05.10:
 *	    |----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
 * 		| 15 | 14 | 13 | 12 | 11 | 10 | 09 | 08 | 07 | 06 | 05 | 04 | 03
 *| 02 | 01 | 00 |
 * 		|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
 *		| XX | XX | A6 | A5 | A4 | A3 | A2 | A1 | XX | C7 | C6 | C5 | C4
 *| C3 | C2 | C1 |
 *		|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
 *
 *		Для МЮ.ИТ.05.10 и двух разрядов МЮ.ИТ.05.10, МЮ.ИТ.05.10:
 *	    |----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
 * 		| 15 | 14 | 13 | 12 | 11 | 10 | 09 | 08 | 07 | 06 | 05 | 04 | 03
 *| 02 | 01 | 00 |
 * 		|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
 *		| XX | XX | XX | A5 | A4 | A3 | A2 | A1 | XX | C7 | C6 | C5 | C4
 *| C3 | C2 | C1 |
 *		|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
 *
 *		Ax - состояние колонки (активный уровень: единица)
 *		Cx - состояние строки  (активный уровень: единица)
 */

// ################################################ STRUCTS
// ################################################
typedef struct {
  GPIO_TypeDef *GPIO;
  uint32_t Pin;
} GPIO_pin_t;

// ############################################### VERIABLES
// ################################################
static GPIO_pin_t OE_pin = {0};
static GPIO_pin_t LE_pin = {0};

// ##################################### STATIC FUNCTIONS PROTOTYPES
// ########################################
static void LED_driver_LE_toHigh();
static void LED_driver_LE_toLow();
static void LED_driver_OE_toHigh();
static void LED_driver_OE_toLow();
// static void LED_driver_start_indication();
// static void LED_driver_impulse_to_latch();

// ############################################ PUBLIC FUNCTIONS
// ############################################

/*	@brief	 Добавить пины, с которых будут управляться входы микросхемы
 * MBI5026 LE и OE
 *
 */
void LED_driver_set_pins(GPIO_TypeDef *GPIO_LE, uint32_t Pin_LE,
                         GPIO_TypeDef *GPIO_OE, uint32_t Pin_OE) {
  LE_pin.GPIO = GPIO_LE;
  LE_pin.Pin = Pin_LE;

  OE_pin.GPIO = GPIO_OE;
  OE_pin.Pin = Pin_OE;
}

/*	@brief	 Отправить 16 бит на драйвер
 *
 */
void LED_driver_send_buffer(uint16_t *buffer_ptr, size_t length) {
  size_t i;

  union {
    uint16_t buff16;
    uint8_t buff8[2];
  } buff_union;

  for (i = 0; i < length; ++i) {
    buff_union.buff16 = buffer_ptr[i];
    // отправляем младший байт
    software_SPI_sendByte(buff_union.buff8[0]);
    // отправляем старший байт
    software_SPI_sendByte(buff_union.buff8[1]);
  }

  // импульс на защелку
  LED_driver_impulse_to_latch();
  // включаем светодиоды
  LED_driver_start_indication();
}

void LED_driver_send(uint16_t data) {
  //	size_t i;
  //
  //	union {
  //		uint16_t 	buff16;
  //		uint8_t 	buff8[2];
  //	} buff_union;

  uint8_t spi_tx_buffer[2];

  spi_tx_buffer[0] = (data >> 8) & 0xFF; // Старший байт
  spi_tx_buffer[1] = data & 0xFF;        // Младший байт

  software_SPI_sendByte(spi_tx_buffer[0]);
  // отправляем старший байт
  software_SPI_sendByte(spi_tx_buffer[1]);

  //	for(i = 0; i < length; ++i)
  //	{
  //		buff_union.buff16 = buffer_ptr[i];
  //		// отправляем младший байт
  //		software_SPI_sendByte(spi_tx_buffer[0]);
  //		// отправляем старший байт
  //		software_SPI_sendByte(spi_tx_buffer[1]);
  //	}

  // импульс на защелку
  LED_driver_impulse_to_latch();
  // включаем светодиоды
  LED_driver_start_indication();
}

// ############################################ PRIVATE FUNCTIONS
// ############################################

static inline void LED_driver_LE_toHigh() { LE_pin.GPIO->BSRR = LE_pin.Pin; }

static inline void LED_driver_LE_toLow() { LE_pin.GPIO->BRR = LE_pin.Pin; }

static inline void LED_driver_OE_toHigh() { OE_pin.GPIO->BSRR = OE_pin.Pin; }

static inline void LED_driver_OE_toLow() { OE_pin.GPIO->BRR = OE_pin.Pin; }

void LED_driver_start_indication() {
#if defined(DRIVER_MBI5026)
  int i;
  // Импровизированный ШИМ, чтобы ограничить силу тока
  // установленно эксперементально, чтобы держало силу тока ~0.15А
  for (i = 0; i < 100; ++i) {
    LED_driver_OE_toLow();
    SysTick_delay_us(4);
    LED_driver_OE_toHigh();
    SysTick_delay_us(1);
  }
#elif defined(DRIVER_STP16)
  LED_driver_OE_toLow();
  SysTick_delay_us(2000);
  LED_driver_OE_toHigh();
#endif
}

void LED_driver_impulse_to_latch() {
  LED_driver_LE_toHigh();
  SysTick_delay_us(BIT_DURATION / 2);
  LED_driver_LE_toLow();
}
