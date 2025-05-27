// #####################################################################
// INCLUDES
// ######################################################################
// #include "NVRAM.h"
#include "SysTick_Delay.h"
#include "assert.h"
#include "config.h"
#include "font.h"
#include "indication.h"
#include "main.h"
// #include "sounds.h"
// #include "state_machine.h"
#include "stddef.h"
#include "stdint.h"
#include "string.h"
// ##################################################################### DEFINES
// #######################################################################
typedef enum {
  ARROWS_NONE = 0,
  ARROWS_DYNAMIC_DOWN,
  ARROWS_DYNAMIC_UP,
  ARROWS_STATIC_DOWN,
  ARROWS_STATIC_UP,
  ARROWS_STATIC_BOTH
} arrows_t;
// ##################################################################### STRUCTS
// #######################################################################
struct indication_frame {
  uint8_t floor;
  uint32_t delay_ms;
  arrows_t arrow;
  // floor_direction_gongs_t gong;
};

// ################################################################## PUBLIC
// FUNCTIONS #################################################################
#define LENGHT_1 3000
#define LENGHT_2 1500
#define LENGHT_3 1000
#define LENGHT_4 6000

static _Bool set_floor(uint8_t floor) {
  symbol_code_e l_symbol, r_symbol;

  l_symbol = floor / 10;
  r_symbol = floor % 10;
  if (l_symbol == 0)
    l_symbol = SYMBOL_EMPTY;
  return indication_set_floor(l_symbol, r_symbol, 1);
}

static _Bool set_arrow(arrows_t arrow) {
  const uint16_t fade_delay_ms = LENGHT_4 >> 1;

  switch (arrow) {
  case (ARROWS_DYNAMIC_DOWN):
    return indication_set_dynamic_arrow(SYMBOL_ARROW_ANIMATION_DOWN_DYNAMIC,
                                        MDIR_DOWN, (_Bool)1);
  case (ARROWS_DYNAMIC_UP):
    return indication_set_dynamic_arrow(SYMBOL_ARROW_ANIMATION_UP_DYNAMIC,
                                        MDIR_UP, (_Bool)1);
  case (ARROWS_STATIC_DOWN):
    return indication_set_static_arrow(SYMBOL_ARROW_DOWN, fade_delay_ms);
  case (ARROWS_STATIC_UP):
    return indication_set_static_arrow(SYMBOL_ARROW_UP, fade_delay_ms);
  case (ARROWS_STATIC_BOTH):
    return indication_set_static_arrow(SYMBOL_ARROW_BOTH, fade_delay_ms);
  default:
    return indication_clear_arrow();
  }
}

const uint8_t bitmaps[3][6] = {
    {0B00000000, 0B00000000, 0B00000010, 0B11111110, 0B01000010, 0B00000000},
    // '1'
    {0B00000000, 0B01100010, 0B10010010, 0B10001010, 0B10000110, 0B01000010},
    // '2'
    {0B00000000, 0B10001100, 0B11010010, 0B10100010, 0B10000010, 0B10000100}
    // '3'
};

void demo_loop() {
  const struct indication_frame frames[] = {
      {1, LENGHT_3, ARROWS_NONE},          {1, LENGHT_4, ARROWS_STATIC_UP},
      {1, LENGHT_1, ARROWS_DYNAMIC_UP},    {2, LENGHT_1, ARROWS_DYNAMIC_UP},
      {3, LENGHT_1, ARROWS_DYNAMIC_UP},    {4, LENGHT_2, ARROWS_DYNAMIC_UP},
      {4, LENGHT_3, ARROWS_NONE},          {4, LENGHT_4, ARROWS_STATIC_UP},
      {4, LENGHT_1, ARROWS_DYNAMIC_UP},    {5, LENGHT_1, ARROWS_DYNAMIC_UP},
      {6, LENGHT_1, ARROWS_DYNAMIC_UP},    {7, LENGHT_1, ARROWS_DYNAMIC_UP},
      {8, LENGHT_1, ARROWS_DYNAMIC_UP},    {9, LENGHT_1, ARROWS_DYNAMIC_UP},
      {10, LENGHT_1, ARROWS_DYNAMIC_UP},   {11, LENGHT_1, ARROWS_DYNAMIC_UP},
      {12, LENGHT_2, ARROWS_DYNAMIC_UP},   {12, LENGHT_3, ARROWS_NONE},
      {12, LENGHT_4, ARROWS_STATIC_DOWN},  {12, LENGHT_4, ARROWS_STATIC_DOWN},
      {11, LENGHT_1, ARROWS_DYNAMIC_DOWN}, {10, LENGHT_1, ARROWS_DYNAMIC_DOWN},
      {9, LENGHT_1, ARROWS_DYNAMIC_DOWN},  {8, LENGHT_1, ARROWS_DYNAMIC_DOWN},
      {7, LENGHT_1, ARROWS_DYNAMIC_DOWN},  {6, LENGHT_1, ARROWS_DYNAMIC_DOWN},
      {5, LENGHT_1, ARROWS_DYNAMIC_DOWN},  {4, LENGHT_1, ARROWS_DYNAMIC_DOWN},
      {3, LENGHT_1, ARROWS_DYNAMIC_DOWN},  {2, LENGHT_1, ARROWS_DYNAMIC_DOWN},
      {1, LENGHT_2, ARROWS_DYNAMIC_DOWN},  {1, LENGHT_4, ARROWS_STATIC_BOTH},
      {1, LENGHT_3, ARROWS_NONE},          {1, LENGHT_4, ARROWS_NONE}};
  const size_t arr_len = sizeof(frames) / sizeof(struct indication_frame);
  static uint32_t next_timestamp = 0;
  static uint8_t next_element = 0;
  uint32_t stamp;

  stamp = SysTick_get_millis();
  if (stamp >= next_timestamp) {
    assert(next_element < arr_len);
    next_timestamp = stamp + frames[next_element].delay_ms;
    set_arrow(frames[next_element].arrow);
    set_floor(frames[next_element].floor);
    // update_LED_panel();
    // if (frames[next_element].gong != 0)
    //   sounds_set_gong(frames[next_element].gong);
    if (++next_element >= arr_len)
      next_element = 0;
  }

  // send_bitmap_to_display();

  // #include "LED_driver.h"
  //
  //  extern const uint8_t bitmap[NUMBER_OF_SYMBOLS + 1]
  //                             [ELEMENTS_IN_BITMAP];  // bitmap
  //
  //  uint8_t *num = (uint8_t *)bitmap[5];
  uint8_t spi_tx_buffer[2];

#if 0
  for (int i = 0; i < 6; i++) {
    if (i % 2 == 1) {
      spi_tx_buffer[1] = num[i];
      //      uint16_t packed_data = (spi_tx_buffer[0] << 8) | spi_tx_buffer[1];
      //      LED_driver_send(packed_data);
      software_SPI_sendByte(spi_tx_buffer[1]);
    } else {
      spi_tx_buffer[0] = num[i];  // Старший байт
      software_SPI_sendByte(spi_tx_buffer[0]);
    }
  }

  // импульс на защелку
  LED_driver_impulse_to_latch();
  // включаем светодиоды
  LED_driver_start_indication();

#endif
}
