/*
 *	Вывод информации на панель:
 * 		1) На панель передаются данные, которые лежат в подготовленном
 *для отправки двумерном массиве
 *concated_bitmap[PANEL_NUMBER_OF_ROWS][NUMBER_OF_DRIVERS], где
 * 					 	- PANEL_NUMBER_OF_ROWS -
 *количество рядов в матрице. Каждый элемент соответствует одному ряду.
 * 					 	- NUMBER_OF_DRIVERS    -
 *количество драйверов, из которых состоит матрица.
 *
 * 		2) Для каждого драйвера определена структура led_panel_element,
 *которая содержит информацию о текущем статусе и указатель на текущий битмап;
 * 		3) concated_bitmap собирается из трех битмапов, которые
 *соответствуют символу, который выводится на каждый элемент панели. 4) для
 *сборки concated_bitmap используется битмап, на который ссылкается указатель
 *element_bitmap_ptr в структуре led_panel_element 5) если символ выводится без
 *анимации, то значению указателя element_bitmap_ptr присваивается значение из
 *стандартного битмапа (bitmapStandart.c), которая соответствует выводимому
 *символу (см. схему ниже) 6) если символ выводится c анимацией, то значению
 *указателя element_bitmap_ptr присваивается значение массива bitmap_animation,
 *в который кладется битмап для текущей итерации анимации (см. схему ниже)
 *
 * 									+-------------------------------+
 *нижняя панель  +------------------------+ |	    ELEMENT_BITMAP_PTR      | <
 *< < < < < < < |  bitmap_tmp_Xx***		|
 * 									+-------------------------------+
 *без анимации   +------------------------+ ^                        ^ ^
 *^ с анимацией	^						 ^ без анимации
 *										^
 *^
 * 			             +------------------------+
 *+------------------------+ | = bitmap_animation*    |	   | = bitmap[symbol]**
 *|
 * 			             +------------------------+
 *+------------------------+
 *
 * 			             *	   bitmap_animation находится в
 *структуре led_panel_element
 * 			             **    глобальная переменная, описана в
 *файле bitmapStandart
 * 			             ***   bitmap_tmp_Xx (X - (L или R); x -
 *номер буфера) находятся в структуре vertical_led_panel
 *
 * 		Формирование текущего bitmap'а для анимации:
 * 		1) для анимации типа Continues, bitmap bitmap_animation
 *формируется с помощью анимирования символа, на который указывает указатель
 *next_symbol_bitmap_ptr 2) для анимации типа Single, bitmap bitmap_animation
 *формируется с помощью анимирования двух символов, на которые указывает
 *указатели next_symbol_bitmap_ptr и prev_symbol_bitmap_ptr
 *
 * 		Формирование текущего bitmap'а для режима СОВМЕСТНОГО
 *ИСПОЛЬЗОВАНИЯ НИЖНЕЙ ПАНЕЛИ: 1) Без анимации: чтобы поместить изображение в
 *центре, стандартный битмап разбивается (отдельно для нижней правой и отдельно
 *для нижней левой). "Разбитый" битмап содержится в bitmap_tmp_Lx и
 *bitmap_tmp_Rx 2) С анимацией: переменным next_symbol_bitmap_ptr и
 *prev_symbol_bitmap_ptr присвааются значения bitmap_tmp_Xx
 *
 */

// #####################################################################
// INCLUDES
// #####################################################################
#include "LED_panel_driver.h"
#include "SysTick_Delay.h"
#include "bitmap_concatenation.h"
#include "config.h"
#include "vertical_animation.h"
// #include "font.h"
// ##################################################################### DEFINES
// #####################################################################
#define ANIMATION_CYCLE_MS 60
// #####################################################################
// VERIABLES
// #####################################################################
extern const uint8_t bitmap[NUMBER_OF_SYMBOLS + 1]
                           [ELEMENTS_IN_BITMAP]; // bitmap
// ##################################################################### STRUCTS
// #####################################################################

/*	Структура, описывающая статус анимации.
 *  Содержит дополнительные указатели для реализации анимации.
 */
struct animation_status {
  uint8_t *next_symbol_bitmap_ptr; // указатель на битмап с текущим символом
  uint8_t
      *prev_symbol_bitmap_ptr; // указатель на битмап с предшествующим символом
  animation_type_t animation_type; // тип анимации
  transition_type_t current_transition; // текущий статус анимации
  uint8_t animation_cycle; // если производится анимация, здесь записывается ее
                           // стадия. 0 - отсутствие активной анимации
  uint32_t time_stamp; // временная метка для анимации
};

/* структура, описывающая элемент led - панели
 * панелей 3:
 * 		- для вертикального: верхняя, нижняя левая и нижняя правая
 *		- для горизонтального: левая, центральная и правая
 */
struct led_panel_element {
  uint8_t *element_bitmap_ptr; // указатель на bitmap элемента, который
                               // подлежит отображению в текущий момент времени
  struct animation_status
      animation; // структура, содержащая информацию об анимации
  _Bool is_update; // необходимость обновления битмапа для панели
  _Bool is_panel_mode; // использование нижних элементов в качестве одного
  uint8_t
      bitmap_animation[PANEL_NUMBER_OF_ROWS]; // временный битмап для анимации
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

  uint8_t
      bitmap_tmp_L1[PANEL_NUMBER_OF_ROWS]; // временный битмап для размещения
                                           // символа, который используется в
                                           // режиме panel_mode
  uint8_t
      bitmap_tmp_L2[PANEL_NUMBER_OF_ROWS]; // временный битмап для размещения
                                           // символа, который используется в
                                           // режиме panel_mode

  uint8_t
      bitmap_tmp_R1[PANEL_NUMBER_OF_ROWS]; // временный битмап для размещения
                                           // символа, который используется в
                                           // режиме panel_mode
  uint8_t
      bitmap_tmp_R2[PANEL_NUMBER_OF_ROWS]; // временный битмап для размещения
                                           // символа, который используется в
                                           // режиме panel_mode

  uint16_t concated_bitmap[PANEL_NUMBER_OF_ROWS]
                          [NUMBER_OF_DRIVERS]; // буффер с сообщениями, которые
                                               // отправляются на светодиоды
};

// ##################################################################### PV
// #####################################################################

static struct vertical_led_panel led_panel = {
    .top_element = {(uint8_t *)bitmap[SYMBOL_EMPTY],
                    {(uint8_t *)bitmap[SYMBOL_EMPTY],
                     (uint8_t *)bitmap[SYMBOL_EMPTY],
                     ANIMATION_VERTICAL_CONTINOUS, TRANSITION_NON, 0, 0},
                    0,
                    0,
                    {0}},
    .bottom_left_element = {(uint8_t *)bitmap[SYMBOL_EMPTY],
                            {(uint8_t *)bitmap[SYMBOL_EMPTY],
                             (uint8_t *)bitmap[SYMBOL_EMPTY],
                             ANIMATION_VERTICAL_SINGLE, TRANSITION_NON, 0, 0},
                            0,
                            0,
                            {0}},
    .bottom_right_element = {(uint8_t *)bitmap[SYMBOL_EMPTY],
                             {(uint8_t *)bitmap[SYMBOL_EMPTY],
                              (uint8_t *)bitmap[SYMBOL_EMPTY],
                              ANIMATION_VERTICAL_SINGLE, TRANSITION_NON, 0, 0},
                             0,
                             0,
                             {0}},
    .bitmap_tmp_L1 = {0},
    .bitmap_tmp_L2 = {0},
    .bitmap_tmp_R1 = {0},
    .bitmap_tmp_R2 = {0},
    .concated_bitmap = {0}};

// ##################################################################### FP
// #####################################################################

static void update_element_animation(struct led_panel_element *element,
                                     uint8_t position);
static void update_element(struct led_panel_element *element, uint8_t position);
static _Bool set_element(struct led_panel_element *element,
                         symbol_code_e symbol, transition_type_t change_type);
static void stop_animation(struct led_panel_element *element);
static _Bool start_animation(struct led_panel_element *element,
                             symbol_code_e symbol,
                             transition_type_t change_type);
static void bottom_block_split_symbol(symbol_code_e symbol);
void send_bitmap_to_display();

// ############################################################ PUBLIC FUNCTIONS
// ###############################################################

/* @brief 	Установить символ, который будет выводится на верхий элемент led
 * - панели.
 *
 * @param 	symbol_table_t symbol 			- символ, который
 * требуется вывести на экарн transition_type_t change_type	- тип анимации,
 * которая будет воспроизводится при смене символа
 *
 * @retVal	false - ошибка входных параметров/ true - установка символа
 * произошла успешна
 */
_Bool set_top_element(symbol_code_e symbol, transition_type_t change_type) {
  if ((symbol >= SYMBOLS_NUMBER) || (change_type > TRANSITION_NON))
    return 0;

  if (!set_element(&led_panel.top_element, symbol, change_type))
    return 0;

  return 1;
}

/* @brief 	Установить символ, который будет выводится на нижний левый
 * элемент led - панели.
 *
 * @param 	symbol_table_t symbol 			- символ, который
 * требуется вывести на экарн transition_type_t change_type	- тип анимации,
 * которая будет воспроизводится при смене символа
 *
 * @retVal	false - ошибка входных параметров/ true - установка символа
 * произошла успешна
 */
_Bool set_bottom_left_element(symbol_code_e symbol,
                              transition_type_t change_type) {
  if ((symbol >= SYMBOLS_NUMBER) || (change_type > TRANSITION_NON))
    return 0;

  // если на момент вызова функции используется нижняя панель
  if (!set_element(&led_panel.bottom_left_element, symbol, change_type))
    return 0;

  return 1;
}

/* @brief 	Установить символ, который будет выводится на нижний правый
 * элемент led - панели.
 *
 * @param 	symbol_table_t symbol 			- символ, который
 * требуется вывести на экарн transition_type_t change_type	- тип анимации,
 * которая будет воспроизводится при смене символа
 *
 * @retVal	false - ошибка входных параметров/ true - установка символа
 * произошла успешна
 */
_Bool set_bottom_right_element(symbol_code_e symbol,
                               transition_type_t change_type) {
  if ((symbol >= SYMBOLS_NUMBER) || (change_type > TRANSITION_NON))
    return 0;

  if (!set_element(&led_panel.bottom_right_element, symbol, change_type))
    return 0;

  // led_panel.panel_elements.lower_right_element.is_sharing_regime = 0;
  // led_panel.bottom_left_element.is_sharing_regime  = 0;

  return 1;
}

/* @brief 	Установить символ, который будет выводится в середине нижней
 * части панели (bottom_block_mode)
 *
 * @param 	symbol_table_t symbol 			- символ, который
 * требуется вывести на экарн transition_type_t change_type	- тип анимации,
 * которая будет воспроизводится при смене символа
 *
 * @retVal	false - ошибка входных параметров/ true - установка символа
 * произошла успешна
 */
_Bool set_bottom_block(symbol_code_e symbol, transition_type_t change_type) {
  if ((symbol >= SYMBOLS_NUMBER) || (change_type > TRANSITION_NON))
    return 0;

  // обновляем нижний левый элемент
  if (!set_element(&led_panel.bottom_left_element, symbol, change_type))
    return 0;

  // обновляем нижний правый элемент
  if (!set_element(&led_panel.bottom_right_element, symbol, change_type))
    return 0;

  // разбиваем полученный символ
  bottom_block_split_symbol(symbol);

  return 1;
}

/*	@brief 	вывести изображение на панель (динамическая индикация)
 * 	@param	NONE
 * 	@retVal	NONE
 */
void update_LED_panel() {
  update_element(&led_panel.top_element, (uint8_t)0);
  update_element(&led_panel.bottom_left_element, (uint8_t)1);
  update_element(&led_panel.bottom_right_element, (uint8_t)2);
  send_bitmap_to_display();
}

// ############################################################ PRIVATE
// FUNCTIONS ###############################################################

static void stop_animation(struct led_panel_element *element) {
  element->element_bitmap_ptr = element->animation.next_symbol_bitmap_ptr;
  element->animation.current_transition = TRANSITION_NON;
  element->is_update = 1;
}

// 1 - анимация запущена; 0 - ошибка (анимация для элемента запрещена)
static _Bool start_animation(struct led_panel_element *element,
                             symbol_code_e symbol,
                             transition_type_t change_type) {
  if (element->animation.animation_type == ANIMATION_NONE)
    return 0;

  if (element->animation.animation_type == ANIMATION_VERTICAL_SINGLE)
    element->animation.prev_symbol_bitmap_ptr = element->element_bitmap_ptr;

  element->animation.current_transition = change_type;
  element->animation.animation_cycle = VERTICAL_ANIMATION_CYCLES;
  element->animation.next_symbol_bitmap_ptr = (uint8_t *)bitmap[symbol];
  element->element_bitmap_ptr = element->bitmap_animation;
  element->animation.time_stamp = SysTick_get_millis();

  return 1;
}

static _Bool set_element(struct led_panel_element *element,
                         symbol_code_e symbol, transition_type_t change_type) {
  // если для элемента уже воспроизводится анимация, останавливаем ее
  if (element->animation.current_transition != TRANSITION_NON) {
    if (element->animation.animation_type == ANIMATION_VERTICAL_CONTINOUS)
      stop_animation(element);
    else
      return 0;
  }

  // смена символа без анимации
  if (change_type == TRANSITION_NON) {
    element->element_bitmap_ptr = (uint8_t *)bitmap[symbol];

    // смена символа с анимацией
  } else {
    if (!start_animation(element, symbol, change_type))
      return 0;
  }

  element->is_update = 1;
  return 1;
}

static void update_animation_cycle_num(struct led_panel_element *element) {
  // если отработали последнюю итерацию анимации
  if (element->animation.animation_cycle == 0) {
    if (element->animation.animation_type == ANIMATION_VERTICAL_SINGLE) {
      stop_animation(element);

    } else if (element->animation.animation_type ==
               ANIMATION_VERTICAL_CONTINOUS) {
      element->animation.animation_cycle = VERTICAL_ANIMATION_CYCLES;
      element->animation.time_stamp =
          SysTick_get_millis() + (10 * ANIMATION_CYCLE_MS);
    }

    return;
  }

  // если пока не отработали последнюю итерацию анимации
  element->animation.animation_cycle--;
  element->animation.time_stamp = SysTick_get_millis() + ANIMATION_CYCLE_MS;
}

static void update_element_animation(struct led_panel_element *element,
                                     uint8_t position) {
  // анимация с "увеличением"
  if (element->animation.current_transition == TRANSITION_INCREASE) {
    if (element->animation.animation_type == ANIMATION_VERTICAL_CONTINOUS)
      animation_scroll_up(element->element_bitmap_ptr,
                          element->animation.animation_cycle,
                          element->animation.next_symbol_bitmap_ptr,
                          element->animation.next_symbol_bitmap_ptr);
    else if (element->animation.animation_type == ANIMATION_VERTICAL_SINGLE)
      animation_scroll_up(element->element_bitmap_ptr,
                          element->animation.animation_cycle,
                          element->animation.prev_symbol_bitmap_ptr,
                          element->animation.next_symbol_bitmap_ptr);
    // анимация с "уменьшением"
  } else if (element->animation.current_transition == TRANSITION_DECREASE) {
    if (element->animation.animation_type == ANIMATION_VERTICAL_CONTINOUS)
      animation_scroll_down(element->element_bitmap_ptr,
                            element->animation.animation_cycle,
                            element->animation.next_symbol_bitmap_ptr,
                            element->animation.next_symbol_bitmap_ptr);
    else if (element->animation.animation_type == ANIMATION_VERTICAL_SINGLE)
      animation_scroll_down(element->element_bitmap_ptr,
                            element->animation.animation_cycle,
                            element->animation.prev_symbol_bitmap_ptr,
                            element->animation.next_symbol_bitmap_ptr);
  }

  update_animation_cycle_num(element);
}

static void update_element(struct led_panel_element *element,
                           uint8_t position) {
  if (element->animation.current_transition > TRANSITION_NON) {
    element->animation.current_transition = TRANSITION_NON;

  } else if (element->animation.current_transition == TRANSITION_NON) {
    if (element->is_update)
      concat_bitmap_1_element(led_panel.concated_bitmap,
                              element->element_bitmap_ptr, position);

    element->is_update = 0;
  } else {
    if (element->animation.time_stamp <= SysTick_get_millis()) {
      update_element_animation(element, position);
      concat_bitmap_1_element(led_panel.concated_bitmap,
                              element->element_bitmap_ptr, position);
    }
  }
}

static uint8_t *get_free_buff_left() {
  uint8_t *tmp = NULL;

  if (led_panel.bottom_left_element.animation.current_transition !=
      TRANSITION_NON)
    tmp = led_panel.bottom_left_element.animation.prev_symbol_bitmap_ptr;
  else
    tmp = led_panel.bottom_left_element.element_bitmap_ptr;

  return ((tmp == led_panel.bitmap_tmp_L1) ? (led_panel.bitmap_tmp_L2)
                                           : (led_panel.bitmap_tmp_L1));
}

static uint8_t *get_free_buff_right() {
  uint8_t *tmp = NULL;

  if (led_panel.bottom_right_element.animation.current_transition !=
      TRANSITION_NON)
    tmp = led_panel.bottom_right_element.animation.prev_symbol_bitmap_ptr;
  else
    tmp = led_panel.bottom_right_element.element_bitmap_ptr;

  return ((tmp == led_panel.bitmap_tmp_R1) ? (led_panel.bitmap_tmp_R2)
                                           : (led_panel.bitmap_tmp_R1));
}

static void bottom_block_split_symbol(symbol_code_e symbol) {
  uint32_t i;
  uint8_t *symbol_bitmap_ptr = (uint8_t *)bitmap[symbol];
  uint8_t *left_element_bitmap;
  uint8_t *right_element_bitmap;

  if (led_panel.bottom_left_element.animation.current_transition !=
      led_panel.bottom_right_element.animation.current_transition)
    led_panel.bottom_left_element.animation.current_transition =
        led_panel.bottom_right_element.animation.current_transition =
            TRANSITION_NON;

  left_element_bitmap = get_free_buff_left();
  right_element_bitmap = get_free_buff_right();

  for (i = 1; i < 3; ++i) {
    right_element_bitmap[i + 3] = symbol_bitmap_ptr[i];
  }

  for (i = 3; i < ELEMENTS_IN_BITMAP; ++i) {
    left_element_bitmap[i - 3] = symbol_bitmap_ptr[i];
  }

  if (led_panel.bottom_left_element.animation.current_transition ==
      TRANSITION_NON) {
    led_panel.bottom_left_element.element_bitmap_ptr = left_element_bitmap;
    led_panel.bottom_right_element.element_bitmap_ptr = right_element_bitmap;
  } else {
    led_panel.bottom_left_element.animation.next_symbol_bitmap_ptr =
        left_element_bitmap;
    led_panel.bottom_right_element.animation.next_symbol_bitmap_ptr =
        right_element_bitmap;
  }
}

void send_bitmap_to_display() {
  int i;
#ifdef config_SPLIT_SYMBOL
  i = 0;
#elif
  i = 1;
#endif

  for (i; i < PANEL_NUMBER_OF_ROWS; ++i)
    LED_driver_send_buffer(led_panel.concated_bitmap[i],
                           (size_t)NUMBER_OF_DRIVERS);
}
