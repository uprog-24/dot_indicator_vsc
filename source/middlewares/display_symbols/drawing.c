/**
 * @file drawing.c
 */
#include "drawing.h"

#include "config.h"
#include "dot.h"
#include "font.h"

#include <stdbool.h>
#include <string.h>

#define BINARY_SYMBOL_CODE_SIZE 6 ///< Binary symbol code size (number of bits)
#define MIN_POSITION_COLUMN                                                    \
  0 ///< Minimum index of position of column for symbol
#define MAX_POSITION_COLUMN                                                    \
  11 ///< Maximum index of position of column for symbol

/**
 * Индексы строки, которая будет отображаться на матрице.
 * Направление имеет позицию 0;
 * MSB (старший бит, первый символ) имеет позицию 1;
 * LSB (младший бит, второй символ) имеет позицию 2.
 */
typedef enum { DIRECTION = 0, MSB = 1, LSB = 2, INDEX_NUMBER } symbol_index_t;

/* Структура, содержащая коды символов для отрисовки */
typedef struct {
  symbol_code_e symbol_code_1;
  symbol_code_e symbol_code_2;
  symbol_code_e symbol_code_3;
} displayed_symbols_t;

static displayed_symbols_t symbols = {
    .symbol_code_1 = SYMBOL_EMPTY,
    .symbol_code_2 = SYMBOL_EMPTY,
    .symbol_code_3 = SYMBOL_EMPTY,
};

/**
 * @brief Установка символа в структуру по индексу
 *
 * @param index: Индекс символа
 * @param symbol Код символа из перечисления symbol_code_e
 */
static void set_symbol_at(symbol_index_t index, symbol_code_e symbol) {
  if (index >= INDEX_NUMBER)
    return;

  switch (index) {
  case DIRECTION:
    symbols.symbol_code_1 = symbol;
    break;
  case MSB:
    symbols.symbol_code_2 = symbol;
    break;
  case LSB:
    symbols.symbol_code_3 = symbol;
    break;
  }
}

/**
 * @brief  Установка символа направления движения
 * @param  direction_code: Код направления (из перечисления symbol_code_e)
 */
void set_direction_symbol(symbol_code_e direction_code) {
  set_symbol_at(DIRECTION, direction_code);
}

/**
 * @brief  Установка символов для этажей
 * @param  left_symbol_code:  Код символа 1
 * @param  right_symbol_code: Код символа 2
 */
void set_floor_symbols(symbol_code_e left_symbol_code,
                       symbol_code_e right_symbol_code) {
  set_symbol_at(MSB, left_symbol_code);
  set_symbol_at(LSB, right_symbol_code);
}

/**
 * @brief  Установка символов (направление + этаж)
 * @param  s1_code:  Код символа 1
 * @param  s2_code:  Код символа 2
 * @param  s3_code:  Код символа 3
 */
void set_symbols(symbol_code_e s1_code, symbol_code_e s2_code,
                 symbol_code_e s3_code) {
  set_direction_symbol(s1_code);
  set_floor_symbols(s2_code, s3_code);
}

/**
 * @brief  Setting structure with type drawing_data_t
 * @param  drawing_data: Pointer to the structure with type drawing_data_t
 * @param  floor:         Floor
 * @param  direction:     Direction with type directionType
 * @retval None
 */
void drawing_data_setter(drawing_data_t *drawing_data, uint8_t floor,
                         directionType direction) {
  drawing_data->floor = floor;
  drawing_data->direction = direction;
}
#if DOT_SPI
#include "LED_driver.h"

#if 1
struct led_panel_element {
  uint8_t *element_bitmap_ptr; // указатель на bitmap элемента, который
                               // подлежит отображению в текущий момент времени
  // struct animation_status
  //     animation;        // структура, содержащая информацию об анимации
  _Bool is_update; // необходимость обновления битмапа для панели
  _Bool is_panel_mode; // использование нижних элементов в качестве одного
  // uint8_t
  //     bitmap_animation[PANEL_NUMBER_OF_ROWS];  // временный битмап для
  //     анимации
};

/*
 * 	Структура описывает led - панель:
 * 		- 3 элемента панели в структуре panel_elements: верхняя, нижняя
 * левая и нижняя правая
 * 		- битмап в структуре panel_bitmap, который отправляется на
 * драйвера
 * 		- 4 дополнительных ячейки для битмапа
 */
struct vertical_led_panel {
  struct led_panel_element
      top_element; // управление в отдельности верхним элементом
  struct led_panel_element
      bottom_left_element; // управление в отдельности нижним левым элементом
  struct led_panel_element
      bottom_right_element; // управление в отдельности нижним правым элементом

  uint8_t bitmap_tmp_L1[NUMBER_OF_ROWS]; // временный битмап для размещения
                                         // символа, который используется в
                                         // режиме panel_mode
  uint8_t bitmap_tmp_L2[NUMBER_OF_ROWS]; // временный битмап для размещения
                                         // символа, который используется в
                                         // режиме panel_mode

  uint8_t bitmap_tmp_R1[NUMBER_OF_ROWS]; // временный битмап для размещения
                                         // символа, который используется в
                                         // режиме panel_mode
  uint8_t bitmap_tmp_R2[NUMBER_OF_ROWS]; // временный битмап для размещения
                                         // символа, который используется в
                                         // режиме panel_mode

  uint16_t concated_bitmap[NUMBER_OF_ROWS]
                          [NUMBER_OF_DRIVERS]; // буффер с сообщениями, которые
                                               // отправляются на светодиоды
};

static struct vertical_led_panel led_panel = {
    .top_element = {(uint8_t *)bitmap[SYMBOL_ARROW_UP], 0, 0},
    .bottom_left_element = {(uint8_t *)bitmap[SYMBOL_EMPTY], 0, 0},
    .bottom_right_element = {(uint8_t *)bitmap[SYMBOL_3], 0, 0},
    .bitmap_tmp_L1 = {0},
    .bitmap_tmp_L2 = {0},
    .bitmap_tmp_R1 = {0},
    .bitmap_tmp_R2 = {0},
    .concated_bitmap = {0}};

// #include "LED_panel_driver.h"

// ############################################################ PUBLIC FUNCTIONS
// ###############################################################

_Bool concat_bitmap_3_elements(
    uint16_t (*concated_bitmap_ptr)[NUMBER_OF_DRIVERS],
    uint8_t top_bitmap_ptr[ELEMENTS_IN_BITMAP],
    uint8_t bottom_right_bitmap_ptr[ELEMENTS_IN_BITMAP],
    uint8_t bottom_left_bitmap_ptr[ELEMENTS_IN_BITMAP]) {
  int j;
  uint16_t row_flag; // переменная для размещения бита, отвечающего за включение
                     // ряда на панели

  if ((concated_bitmap_ptr == NULL) || (top_bitmap_ptr == NULL) ||
      (bottom_right_bitmap_ptr == NULL) || (bottom_left_bitmap_ptr == NULL))
    return 0;

  // построчная развертка
  for (j = 0; j < NUMBER_OF_ROWS; ++j) {
    row_flag = 1 << (2 + j);
    if (concated_bitmap_ptr[j] == NULL)
      return 0;
    // записываем bitmap для верхнего разряда
    concated_bitmap_ptr[j][0] = row_flag + (top_bitmap_ptr[j] << 8);
    // записываем bitmap для правого нижнего раряда
    concated_bitmap_ptr[j][1] = row_flag + (bottom_right_bitmap_ptr[j] << 8);
    // записываем bitmap для левого нижнего раряда
    concated_bitmap_ptr[j][2] = row_flag + (bottom_left_bitmap_ptr[j] << 8);
  }

  return 1;
}

_Bool concat_bitmap_2_elements(
    uint16_t (*concated_bitmap_ptr)[NUMBER_OF_DRIVERS],
    uint8_t first_bitmap_ptr[ELEMENTS_IN_BITMAP], uint8_t first_elem_in_bitmap,
    uint8_t second_bitmap_ptr[ELEMENTS_IN_BITMAP],
    uint8_t second_elem_in_bitmap) {
  int j;
  uint16_t row_flag;

  if ((concated_bitmap_ptr == NULL) || (first_bitmap_ptr == NULL) ||
      (second_bitmap_ptr == NULL))
    return 0;

  if ((first_elem_in_bitmap > 2) || (second_elem_in_bitmap > 2))
    return 0;

  // построчная развертка
  for (j = 0; j < NUMBER_OF_ROWS; ++j) {
    if (concated_bitmap_ptr[j] == NULL)
      return 0;
    row_flag = 1 << (2 + j);
    // правый нижний раряд
    concated_bitmap_ptr[j][first_elem_in_bitmap] =
        row_flag + (first_bitmap_ptr[j] << 8);
    // левый нижний разряд
    concated_bitmap_ptr[j][second_elem_in_bitmap] =
        row_flag + (second_bitmap_ptr[j] << 8);
  }

  return 1;
}

_Bool concat_bitmap_1_element(
    uint16_t (*concated_bitmap_ptr)[NUMBER_OF_DRIVERS],
    uint8_t bitmap_ptr[ELEMENTS_IN_BITMAP], uint8_t elem_in_bitmap) {
  int j;
  uint16_t row_flag;

  if ((concated_bitmap_ptr == NULL) || (elem_in_bitmap > 2))
    return 0;

  // построчная развертка
  for (j = 0; j < NUMBER_OF_ROWS; ++j) {
    if (concated_bitmap_ptr[j] == NULL)
      return 0;
    row_flag = 1 << (2 + j);
    concated_bitmap_ptr[j][elem_in_bitmap] = row_flag + (bitmap_ptr[j] << 8);
  }
  return 1;
}

static void update_element(struct led_panel_element *element,
                           uint8_t position) {

  if (element->is_update)
    concat_bitmap_1_element(led_panel.concated_bitmap,
                            element->element_bitmap_ptr, position);

  element->is_update = 0;
}

void update_LED_panel() {
  update_element(&led_panel.top_element, (uint8_t)0);
  update_element(&led_panel.bottom_left_element, (uint8_t)1);
  update_element(&led_panel.bottom_right_element, (uint8_t)2);
  send_bitmap_to_display();
}

void send_bitmap_to_display() {
  int i;
#ifdef config_SPLIT_SYMBOL
  i = 0;
#else
  i = 1;
#endif

  for (i; i < NUMBER_OF_ROWS; ++i)
    LED_driver_send_buffer(led_panel.concated_bitmap[i],
                           (size_t)NUMBER_OF_DRIVERS);
}

//=====================================================INDICATION

struct floor_indication {
  symbol_code_e t_elem;
  symbol_code_e l_elem;
  symbol_code_e r_elem;
  // enum arrow_panel_status		arrow_status;
  // uint32_t		 			afterstop_arrow_stamp;
  // // для организации задержки при смене стрелки со статической на ARROWS_NONE
};
// #####################################################################
// VERIABLES
// #####################################################################
static struct floor_indication floor_displaying = {
    .t_elem = SYMBOL_EMPTY, .l_elem = SYMBOL_EMPTY, .r_elem = SYMBOL_EMPTY,
    // .arrow_status = AS_CLEAR_ARROW_PANEL,
    // .afterstop_arrow_stamp = 0
};

static _Bool set_element(struct led_panel_element *element,
                         symbol_code_e symbol) {
  // если для элемента уже воспроизводится анимация, останавливаем ее
  // if (element->animation.current_transition != TRANSITION_NON) {
  //   if (element->animation.animation_type == ANIMATION_VERTICAL_CONTINOUS)
  //     stop_animation(element);
  //   else
  //     return 0;
  // }

  // смена символа без анимации
  // if (change_type == TRANSITION_NON) {
  element->element_bitmap_ptr = (uint8_t *)bitmap[symbol];

  // смена символа с анимацией
  // } else {
  //   if (!start_animation(element, symbol, change_type)) return 0;
  // }

  element->is_update = 1;
  return 1;
}

_Bool set_bottom_left_element(symbol_code_e symbol) {
  if ((symbol > SYMBOLS_NUMBER))
    return 0;

  // если на момент вызова функции используется нижняя панель
  if (!set_element(&led_panel.bottom_left_element, symbol))
    return 0;

  return 1;
}

_Bool set_top_element(symbol_code_e symbol) {
  if (symbol > SYMBOLS_NUMBER)
    return 0;

  if (!set_element(&led_panel.top_element, symbol))
    return 0;

  return 1;
}

static uint8_t *get_free_buff_left() {
  uint8_t *tmp = NULL;
  tmp = led_panel.bottom_left_element.element_bitmap_ptr;

  return ((tmp == led_panel.bitmap_tmp_L1) ? (led_panel.bitmap_tmp_L2)
                                           : (led_panel.bitmap_tmp_L1));
}

static uint8_t *get_free_buff_right() {
  uint8_t *tmp = NULL;

  tmp = led_panel.bottom_right_element.element_bitmap_ptr;

  return ((tmp == led_panel.bitmap_tmp_R1) ? (led_panel.bitmap_tmp_R2)
                                           : (led_panel.bitmap_tmp_R1));
}

static void bottom_block_split_symbol(symbol_code_e symbol) {
  uint32_t i;
  uint8_t *symbol_bitmap_ptr = (uint8_t *)bitmap[symbol];
  uint8_t *left_element_bitmap;
  uint8_t *right_element_bitmap;

  // if (led_panel.bottom_left_element.animation.current_transition !=
  //     led_panel.bottom_right_element.animation.current_transition)
  //   led_panel.bottom_left_element.animation.current_transition =
  //       led_panel.bottom_right_element.animation.current_transition =
  //           TRANSITION_NON;

  left_element_bitmap = get_free_buff_left();
  right_element_bitmap = get_free_buff_right();

  for (i = 1; i < 3; ++i) {
    right_element_bitmap[i + 3] = symbol_bitmap_ptr[i];
  }

  for (i = 3; i < ELEMENTS_IN_BITMAP; ++i) {
    left_element_bitmap[i - 3] = symbol_bitmap_ptr[i];
  }

  led_panel.bottom_left_element.element_bitmap_ptr = left_element_bitmap;
  led_panel.bottom_right_element.element_bitmap_ptr = right_element_bitmap;
}

_Bool set_bottom_right_element(symbol_code_e symbol) {
  if ((symbol > SYMBOLS_NUMBER))
    return 0;

  if (!set_element(&led_panel.bottom_right_element, symbol))
    return 0;

  // led_panel.panel_elements.lower_right_element.is_sharing_regime = 0;
  // led_panel.bottom_left_element.is_sharing_regime  = 0;

  return 1;
}

_Bool set_bottom_block(symbol_code_e symbol) {
  if ((symbol > SYMBOLS_NUMBER))
    return 0;

  // обновляем нижний левый элемент
  if (!set_element(&led_panel.bottom_left_element, symbol))
    return 0;

  // обновляем нижний правый элемент
  if (!set_element(&led_panel.bottom_right_element, symbol))
    return 0;

  // разбиваем полученный символ
  bottom_block_split_symbol(symbol);

  return 1;
}

static _Bool update_floor(struct floor_indication *fdisplaying_p,
                          symbol_code_e l_elem, symbol_code_e r_elem) {

#if defined(config_SPLIT_SYMBOL)
  if (l_elem == SYMBOL_EMPTY) {
    return set_bottom_block(r_elem);
  }
#endif
  if (fdisplaying_p->l_elem != l_elem)
    if (set_bottom_left_element(l_elem) == 0)
      return 0;
  if (set_bottom_right_element(r_elem) == 0)
    return 0;
  return 1;
}

_Bool indication_set_static_arrow(symbol_code_e arrow_elem) {

  /* некорректные входные значения */
  if (arrow_elem > SYMBOLS_NUMBER)
    return 0;

  /* уже установлено */
  if ((floor_displaying.t_elem == arrow_elem))
    return 1;

  // if (state_get_state() == STATE_DISPLAYING_FLOOR)
  if (set_top_element(arrow_elem) == 0)
    return 0;
  return 1;
}

_Bool indication_clear_arrow() {
  const symbol_code_e symbol_clean_btmp = SYMBOL_EMPTY;

  // if (floor_displaying.t_elem == symbol_clean_btmp)
  //   return 1;
  if (set_top_element(symbol_clean_btmp) == 0)
    return 0;
  floor_displaying.t_elem = symbol_clean_btmp;
  return 1;
}

_Bool indication_set_floor(symbol_code_e l_elem, symbol_code_e r_elem) {

  /* некорректные входные значения */
  if ((l_elem > SYMBOLS_NUMBER) || (r_elem > SYMBOLS_NUMBER))
    return 0;
  /* аналогичные значения уже установлены */
  if ((l_elem == floor_displaying.l_elem) &&
      (r_elem == floor_displaying.r_elem))
    return 1;
  // if(state_get_state() == STATE_DISPLAYING_FLOOR) {
  if (update_floor(&floor_displaying, l_elem, r_elem) == 0)
    return 0;
  // }
  floor_displaying.l_elem = l_elem;
  floor_displaying.r_elem = r_elem;
  return 1;
}

_Bool indication_set_full_panel(symbol_code_e top_elem, symbol_code_e left_elem,
                                symbol_code_e right_elem) {
  indication_set_static_arrow(top_elem);
  indication_set_floor(left_elem, right_elem);
}
#endif

void STP16_SendData(uint16_t register1, uint16_t register2,
                    uint16_t register3) {
  uint8_t spi_tx_buffer[6];

  spi_tx_buffer[0] = (register3 >> 8) & 0xFF; // 3-й регистр (старший байт)
  spi_tx_buffer[1] = register3 & 0xFF; // 3-й регистр (младший байт)

  spi_tx_buffer[2] = (register2 >> 8) & 0xFF; // 2-й регистр (старший байт)
  spi_tx_buffer[3] = register2 & 0xFF; // 2-й регистр (младший байт)

  spi_tx_buffer[4] = (register1 >> 8) & 0xFF; // 1-й регистр (старший байт)
  spi_tx_buffer[5] = register1 & 0xFF; // 1-й регистр (младший байт)

  software_SPI_sendByte(spi_tx_buffer[0]);
  software_SPI_sendByte(spi_tx_buffer[1]);

  software_SPI_sendByte(spi_tx_buffer[2]);
  software_SPI_sendByte(spi_tx_buffer[3]);

  software_SPI_sendByte(spi_tx_buffer[4]);
  software_SPI_sendByte(spi_tx_buffer[5]);

  // Импульс LE (защелкивание данных)
  LED_driver_impulse_to_latch();
  // включаем светодиоды
  LED_driver_start_indication();
}

#if 1
uint16_t buffer[3];
void display_symbols_spi() {

#if 0
  for (int row = 0; row < 6; row++) {
    uint16_t reg1 = 0, reg2 = 0, reg3 = 0;

    // Формируем данные колонок (bitmap)
    uint8_t col1 = bitmap[symbols.symbol_code_3][row];
    uint8_t col2 = bitmap[symbols.symbol_code_2][row];
    uint8_t col3 = bitmap[symbols.symbol_code_1][row];

    reg1 = col1;
    reg2 = col2;
    reg3 = col3;

    // Маска строки
    uint16_t row_mask = (1 << (7 + row));

    // if (symbols.symbol_code_3 == SYMBOL_EMPTY) {
    //   if (row != 0)
    //     reg2 |= (1 << (7 + row));
    //   if (13 + row < 16 && row != 0)
    //     reg1 = reg2 | (1 << (13 + row));
    //   else
    //     reg1 = 0;
    // } else {
    if (reg1 != 0)
      reg1 |= row_mask;
    if (reg2 != 0)
      reg2 |= row_mask;
    // }

    if (reg3 != 0)
      reg3 |= row_mask;

    // Собираем в буфер
    buffer[0] = reg3;
    buffer[1] = reg2;
    buffer[2] = reg1;

    // Отправляем буфер
    LED_driver_send_buffer(buffer, 3);
  }
#endif

#if 1
  for (int row = 0; row < 6; row++) {
    uint16_t register1 = 0, register2 = 0, register3 = 0;

    // === Символ 1 на register1 ===
    for (int col = 0; col < 8; col++) {
      if (bitmap[symbols.symbol_code_3][row] & (1 << (7 - col))) {
        register1 |= (1 << (7 - col)); // Колонки 0–6
      }
    }

    // === Символ 2 на register2 ===
    for (int col = 0; col < 8; col++) {
      if (bitmap[symbols.symbol_code_2][row] & (1 << (7 - col))) {
        register2 |= (1 << (7 - col));
      }
    }

    // === Символ 3 на register3 ===
    for (int col = 0; col < 8; col++) {
      if (bitmap[symbols.symbol_code_1][row] & (1 << (7 - col))) {
        register3 |= (1 << (7 - col));
      }
    }

    // === Формируем маску строки ===
    uint16_t row_mask_r1 = 0, row_mask_r2 = 0, row_mask_r3 = 0;

    // register1: строки 8–13
    // if (row >= 0 && row <= 5) {
    // row_mask_r1 = (1 << (10 + row));
    // }

    // register2: строки 8–12
    // if (row >= 0 && row <= 4) {
    // row_mask_r2 = (1 << (10 + row));
    // }

    // register3: строки 8–12
    // if (row >= 0 && row <= 4) {
    // row_mask_r3 = (1 << (10 + row));
    // }

    // === Центрирование, если символ 2 пустой ===
    // if (symbols.symbol_code_2 == SYMBOL_EMPTY) {
    //   if (row != 0) {
    //     // Добавим строку чуть в другое место, визуальный центр
    //     register2 |= (1 << (7 + row)); // Бит 8..12 — допустимо
    //     if ((13 + row) < 16) {
    //       register1 |= (1 << (13 + row));
    //     }
    //   }
    // } else {
    //   // Включаем строки на всех регистрах
    //   register1 |= row_mask_r1;
    //   register2 |= row_mask_r2;
    // }

    // register3 |= row_mask_r3;

    // Включаем строку (OUT8 - OUT12 или OUT8 - OUT13)
    uint16_t row_mask = (1 << (10 + row));

    /** Центрирование символа для register1 и register2 */
    if (symbols.symbol_code_3 == SYMBOL_EMPTY) {

      if (row != 0)
        register2 |= (1 << (7 + row)); // Включаем строку на 2-м чипе

      if (13 + row < 16) {
        if (row != 0)
          register1 = register2 | (1 << (13 + row));
      } else {
        register1 = 0;
      }

    } else {
      if (register1 != 0)
        register1 |= row_mask; // Включаем строку на 1-м чипе
      if (register2 != 0)
        register2 |= row_mask; // Включаем строку на 2-м чипе
    }

    if (register3 != 0)
      register3 |= row_mask; // Включаем строку на 3-м чипе

    // === Отправка данных ===
    STP16_SendData(register1, register2, register3);
  }
#endif
}
#endif

#if 0
void display_symbols_spi() {

  uint16_t row_flag;
#if 1
  for (int row = 0; row < 6; row++) // Перебираем строки
  {
    uint16_t register1 = 0, register2 = 0, register3 = 0;
    // row_flag = 1 << (2 + j);

    // Символ 1 на регистре 1
    for (int col = 0; col < 7; col++) {
      if (bitmap[symbols.symbol_code_1][row] & (1 << (7 - col))) {
        register1 |= (1 << (7 - col));
      }
    }

    // Символ 2 на регистре 2
    for (int col = 0; col < 7; col++) {
      if (bitmap[symbols.symbol_code_2][row] & (1 << (7 - col))) {
        register2 |= (1 << (7 - col));
      }
    }

    // Символ 3 на регистре 3
    for (int col = 1; col < 8; col++) {
      if (bitmap[symbols.symbol_code_3][row] & (1 << (8 - col))) {
        register3 |= (1 << (8 - col));
      }
    }

    // Включаем строку (OUT8 - OUT12 или OUT8 - OUT13)
    uint16_t row_mask = (1 << (8 + row));

    /** Центрирование символа для register1 и register2 */
    if (symbols.symbol_code_2 == SYMBOL_EMPTY) {

      if (row != 0)
        register2 |= (1 << (7 + row)); // Включаем строку на 2-м чипе

      if (13 + row < 16) {
        if (row != 0)
          register1 = register2 | (1 << (13 + row));
      } else {
        register1 = 0;
      }

    } else {
      register1 |= row_mask; // Включаем строку на 1-м чипе
      register2 |= row_mask; // Включаем строку на 2-м чипе
    }

    register3 |= row_mask; // Включаем строку на 3-м чипе

    // Отправляем данные
    STP16_SendData(register1, register2, register3);
  }
#endif

#if 0
  for (int row = 0; row < 6; row++) // по строкам в повернутом виде
  {
    uint16_t register1 = 0, register2 = 0, register3 = 0;

    for (int col = 0; col < 7; col++) {
      // Поворачиваем символ — берем бит по [5 - row][col], формируем
      // rotated_row[col] с битами row

      // Символ 1
      if (get_symbol_code(matrix_string[LSB])[5 - col] & (1 << row)) {
        register1 |= (1 << col);
      }
      // Символ 2
      if (get_symbol_code(matrix_string[MSB])[5 - col] & (1 << row)) {
        register2 |= (1 << col);
      }
      // Символ 3
      if (get_symbol_code(matrix_string[DIRECTION])[5 - col] & (1 << row)) {
        register3 |= (1 << col);
      }
    }

    // Включаем строку (OUT8 - OUT12 или OUT8 - OUT13)
    uint16_t row_mask = (1 << (10 + row));

    /** Центрирование символа для register1 и register2 */
    if (matrix_string[LSB] == 'c') {

      if (row != 0)
        register2 |= (1 << (7 + row)); // Включаем строку на 2-м чипе

      if (13 + row < 16) {
        if (row != 0)
          register1 = register2 | (1 << (13 + row));
      } else {
        register1 = 0;
      }

    } else {
      register1 |= row_mask; // Включаем строку на 1-м чипе
      register2 |= row_mask; // Включаем строку на 2-м чипе
    }

    register3 |= row_mask; // Включаем строку на 3-м чипе

    // Отправляем в драйверы
    STP16_SendData(register1, register2, register3);
  }
#endif
}

#endif

#if 0
#define PANEL_NUMBER_OF_ROWS 6
#define NUMBER_OF_DRIVERS 3

void display_symbols_spi(char *matrix_string) {
  uint16_t concated_bitmap[PANEL_NUMBER_OF_ROWS][NUMBER_OF_DRIVERS] = {0};
  uint16_t row_flag;

  // Формируем данные для каждого ряда и драйвера
  for (int row = 0; row < PANEL_NUMBER_OF_ROWS; row++) {
    row_flag = 1 << (2 + row);

    uint16_t register1 = 0, register2 = 0, register3 = 0;

    // Формируем register1 (символ LSB)
    for (int col = 0; col < 7; col++) {
      if (get_symbol_code(matrix_string[LSB])[row] & (1 << (6 - col))) {
        register1 |= (1 << (6 - col));
      } else {
        register1 &= ~(1 << (6 - col)); // явный сброс бита
      }
    }

    // Формируем register2 (символ MSB)
    for (int col = 0; col < 7; col++) {
      if (get_symbol_code(matrix_string[MSB])[row] & (1 << (6 - col))) {
        register2 |= (1 << (6 - col));
      } 
      // else {
      //   register2 &= ~(1 << (6 - col));
      // }
    }

    // Формируем register3 (символ DIRECTION)
    for (int col = 0; col < 7; col++) {
      if (get_symbol_code(matrix_string[DIRECTION])[row] & (1 << (6 - col))) {
        register3 |= (1 << (6 - col));
      } 
      // else {
      //   register3 &= ~(1 << (6 - col));
      // }
    }

    // Центрирование символов (по вашему коду)
    if (matrix_string[LSB] == 'c') {
      if (row != 0) {
        register2 |= (1 << (7 + row));
      }

      if (13 + row < 16) {
        if (row != 0) {
          register1 = register2 | (1 << (13 + row));
        }
      } else {
        register1 = 0;
      }
    } else {
      register1 |= row_flag;
      register2 |= row_flag;
    }

    register3 |= row_flag;

    // Записываем в массив, сдвигая данные в старшие 8 бит
    concated_bitmap[row][0] = row_flag | (register1 << 8);
    concated_bitmap[row][1] = row_flag | (register2 << 8);
    concated_bitmap[row][2] = row_flag | (register3 << 8);
  }

  // Отправляем сформированный массив построчно
  for (int row = 0; row < PANEL_NUMBER_OF_ROWS; row++) {
    STP16_SendData(concated_bitmap[row][0], concated_bitmap[row][1],
                   concated_bitmap[row][2]);
  }
}
#endif

#define SYMBOL_TABLE_SIZE 128

// Маппинг символов char в код symbol_code_e
static const symbol_code_e char_to_symbol_table[SYMBOL_TABLE_SIZE] = {
    ['0'] = SYMBOL_0,    ['1'] = SYMBOL_1,     ['2'] = SYMBOL_2,
    ['3'] = SYMBOL_3,    ['4'] = SYMBOL_4,     ['5'] = SYMBOL_5,
    ['6'] = SYMBOL_6,    ['7'] = SYMBOL_7,     ['8'] = SYMBOL_8,
    ['9'] = SYMBOL_9,    ['S'] = SYMBOL_S,     ['I'] = SYMBOL_I,
    ['D'] = SYMBOL_D,    ['-'] = SYMBOL_MINUS, [' '] = SYMBOL_EMPTY,
    ['.'] = SYMBOL_DOT,  ['c'] = SYMBOL_EMPTY, ['V'] = SYMBOL_V,
    ['L'] = SYMBOL_L,    ['C'] = SYMBOL_C,     ['E'] = SYMBOL_E,
    ['+'] = SYMBOL_PLUS, ['p'] = SYMBOL_P,     ['g'] = SYMBOL_G,
    ['K'] = SYMBOL_K,    ['U'] = SYMBOL_U_BIG,
};

/**
 * @brief Преобразование символа char в код symbol_code_e
 * @param ch
 * @return symbol_code_e
 */
static inline symbol_code_e char_to_symbol(char ch) {
  if (ch < SYMBOL_TABLE_SIZE) {
    return char_to_symbol_table[(unsigned char)ch];
  } else {
    return SYMBOL_EMPTY;
  }
}

/**
 * @brief Отображение строки
 * @param matrix_string: Указатель на строку, которая будет отображаться
 */
void draw_string(char *matrix_string) {
  // Преобразуем символ char в код symbol_code_e
  set_symbols(char_to_symbol(matrix_string[0]),
              char_to_symbol(matrix_string[1]),
              char_to_symbol(matrix_string[2]));
  display_symbols_spi();
}

extern volatile bool is_time_ms_for_display_str_elapsed;
/**
 * @brief  Отображение строки на матрице в течение
 *         TIME_DISPLAY_STRING_DURING_MS (определено в tim.c)
 * @note   Для протоколов при запуске индикатора
 * @param  matrix_string: Указатель на строку, которая будет отображаться
 */
void display_string_during_ms(char *matrix_string) {
  is_time_ms_for_display_str_elapsed = false;

  while (!is_time_ms_for_display_str_elapsed) {
    draw_string(matrix_string);
  }

  // Очищаем поля структуры с символами
  set_symbols(SYMBOL_EMPTY, SYMBOL_EMPTY, SYMBOL_EMPTY);
}

/**
 * @brief  Отображение строки на матрице в течение
 *         TIME_DISPLAY_STRING_DURING_MS (определено в tim.c)
 * @note   Для DEMO_MODE
 */
void display_symbols_during_ms() {
  is_time_ms_for_display_str_elapsed = false;

  while (!is_time_ms_for_display_str_elapsed) {
    display_symbols_spi();
  }
}

#else

extern volatile bool is_tim4_period_elapsed;

/**
 * @brief  Draw the symbol on matrix starting with start_pos in range
 *         [MIN_POSITION_COLUMN, MAX_POSITION_COLUMN]
 * @note   1. Get the symbol code from symbols[];
 *         2. Go by current_row, TURN_ON current_row, TURN_OFF others;
 *         3. Get binary_symbol_code_row for current_row from cur_symbol_code[];
 *         4. Go by all columns and set TURN_ON if
 *            binary_symbol_code_row[num_bit] = 1 else set TURN_OFF;
 *         5. TURN_OFF all columns and rows after process current_row in order
 *            to avoid the effect of shadow (turning on unnecessary LEDs).
 * @param  symbol:    Symbol from symbols[] (font.c)
 * @param  start_pos: Start position (index of column) for symbol
 * @param  shift:     Shift by Y for animation of symbol movement
 * @retval None
 */
static void draw_symbol_on_matrix(char symbol, uint8_t start_pos,
                                  uint8_t shift) {

  uint8_t *cur_symbol_code = get_symbol_code(symbol);
  if (cur_symbol_code == NULL)
    return;

  static uint8_t current_row = 0;

  // Включаем текущую строку
  set_row_state(current_row, TURN_ON);

  // Получаем значения для колонок текущей строки
  uint8_t binary_symbol_code_row[BINARY_SYMBOL_SIZE];
  if (current_row + shift < ROWS) {
    convert_number_from_dec_to_bin(cur_symbol_code[current_row + shift],
                                   binary_symbol_code_row,
                                   BINARY_SYMBOL_CODE_SIZE);
  } else {
    memset(binary_symbol_code_row, 0, BINARY_SYMBOL_SIZE);
  }

  // Включаем колонку, если бит = 1
  for (uint8_t i = 0; i < 7; i++) {
    uint8_t current_col = BINARY_SYMBOL_CODE_SIZE - i;
    if (binary_symbol_code_row[current_col] == 1) {
      set_col_state(start_pos + i, TURN_ON);
    }
  }

  /**
   * Держим состояние строки с колонками, пока таймер не завершит
   * отсчет (1000 мкс)
   */
  if (is_tim4_period_elapsed) {
    is_tim4_period_elapsed = false;

    // Переходим к следующей строке
    current_row++;

    // Выключаем предыдущую строку
    if (current_row) {
      set_row_state(current_row - 1, TURN_OFF);
    }
    // Выключаем все колонки
    set_all_cols_state(TURN_OFF);

    // Завершаем проход по строкам
    if (current_row >= ROWS) {
      current_row = 0;
    }
  }
}

/**
 * @brief  Check type of start symbol, special: 'c', '>', '<', '+' or not
 * @param  matrix_string: Pointer to the matrix_string that will be displayed on
 *         matrix
 * @retval None
 */
static bool is_start_symbol_special(char *matrix_string) {
  return (matrix_string[DIRECTION] == 'c' || matrix_string[DIRECTION] == '>' ||
          matrix_string[DIRECTION] == '<' || matrix_string[DIRECTION] == '+' ||
          matrix_string[DIRECTION] == '-' || matrix_string[DIRECTION] == 'p');
}

/**
 * @brief  Draw matrix_string (without special start symbol - matrix_string[0]:
 *         'c','>', '<', '+') on matrix
 * @param  matrix_string: Pointer to the matrix_string that will be displayed on
 *                        matrix
 * @retval None
 */
static void draw_symbols(char *matrix_string) {
  if (strlen(matrix_string) == 3) { // 3 symbols, font_width = 4

    // draw DIRECTION symbol
    if (matrix_string[DIRECTION] == 'V' || matrix_string[DIRECTION] == 'K' ||
        matrix_string[MSB] == 'K') { // font_width = 5
      draw_symbol_on_matrix(matrix_string[DIRECTION], 0, 0);
    } else {
      draw_symbol_on_matrix(matrix_string[DIRECTION], 1, 0);
    }

    // draw MSB symbol
    if (matrix_string[DIRECTION] == 'U' && matrix_string[MSB] == 'K') {
      draw_symbol_on_matrix(matrix_string[MSB], 5, 0);
    } else {
      draw_symbol_on_matrix(matrix_string[MSB], 6, 0);
    }

    // draw LSB symbol
    if (matrix_string[MSB] == '.') { // version
      draw_symbol_on_matrix(matrix_string[LSB], 8, 0);
    } else {
      draw_symbol_on_matrix(matrix_string[LSB], 11, 0);
    }
  }

  if (strlen(matrix_string) == 4) {
    draw_symbol_on_matrix(matrix_string[0], 0, 0);
    draw_symbol_on_matrix(matrix_string[1], 5, 0);
    draw_symbol_on_matrix(matrix_string[2], 8, 0);
    draw_symbol_on_matrix(matrix_string[3], 11, 0);
  }
}

/**
 * @brief  Draw matrix_string (with special start symbol matrix_string[0]: 'c',
 *         '>', '<', '+') on matrix
 * @param  matrix_string: Pointer to the matrix_string that will be displayed on
 *                        matrix
 * @retval None
 */
static void draw_special_symbols(char *matrix_string) {
  // stop floor 1..9: c1c
  if (matrix_string[DIRECTION] == 'c' && matrix_string[LSB] == 'c') {
    draw_symbol_on_matrix(matrix_string[DIRECTION], 0, 0);
    draw_symbol_on_matrix(matrix_string[LSB], 0, 0);
    draw_symbol_on_matrix(matrix_string[MSB], 6, 0);

  } else if (matrix_string[DIRECTION] ==
             'c') { // stop floor c10..c99 and c-1..c-9

    draw_symbol_on_matrix(matrix_string[DIRECTION], 0, 0);
    if (matrix_string[MSB] == '1' || matrix_string[MSB] == 'I') {
      draw_symbol_on_matrix(matrix_string[MSB], 5, 0);
      draw_symbol_on_matrix(matrix_string[LSB], 9, 0);
    } else if (matrix_string[MSB] != '-') {
      draw_symbol_on_matrix(matrix_string[MSB], 4, 0);
      // "cKg" перегруз
      if (matrix_string[MSB] == 'K' && matrix_string[LSB] == 'g') {
        draw_symbol_on_matrix(matrix_string[LSB], 10, 0);
      } else {
        draw_symbol_on_matrix(matrix_string[LSB], 9, 0);
      }
    }

    if (matrix_string[MSB] == '-' && matrix_string[LSB] != '-') {
      draw_symbol_on_matrix(matrix_string[MSB], 4, 0);
      draw_symbol_on_matrix(matrix_string[LSB], 8, 0);
    }

    // "c--" interface is not connected
    if (matrix_string[MSB] == '-' && matrix_string[LSB] == '-') {
      draw_symbol_on_matrix(matrix_string[MSB], 9, 0);
      draw_symbol_on_matrix(matrix_string[LSB], 4, 0);
    }
  } else if (matrix_string[DIRECTION] == '>' ||
             matrix_string[DIRECTION] == '<' ||
             matrix_string[DIRECTION] == '+' ||
             matrix_string[DIRECTION] == '-' ||
             matrix_string[DIRECTION] == 'p') { // in moving up/down: >10 or >1c

    if (matrix_string[DIRECTION] == '-' || matrix_string[DIRECTION] == 'p') {
      draw_symbol_on_matrix(matrix_string[DIRECTION], 1, 0);
    } else {
      draw_symbol_on_matrix(matrix_string[DIRECTION], 0, 0);
    }

    draw_symbol_on_matrix(matrix_string[MSB], 6, 0);

    // font_width = 3 for '1' and '-'
    if (matrix_string[MSB] == '1' || matrix_string[MSB] == '-') {
      draw_symbol_on_matrix(matrix_string[LSB], 10, 0);
    } else if (matrix_string[MSB] != '-') {
      draw_symbol_on_matrix(matrix_string[LSB], 11, 0);
    }
  }
}

/**
 * @brief  Draw matrix_string on matrix depend on the type of matrix_string
 * @param  matrix_string: Pointer to the matrix_string that will be displayed on
 *                        matrix
 * @retval None
 */
void draw_string_on_matrix(char *matrix_string) {

  if (is_start_symbol_special(matrix_string)) {
    draw_special_symbols(matrix_string);
  } else {
    draw_symbols(matrix_string);
  }
}
#endif