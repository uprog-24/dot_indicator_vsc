/**
 * @file drawing.c
 */
#include "drawing.h"

#include "config.h"
#include "dot.h"
#include "font.h"

#include <stdbool.h>
#include <string.h>

#define START_INDEX_SYMBOL_ROW                                                 \
  6 ///< Код символа в двоичном представлении начинается с 6-го индекса в
    ///< строке. Отрисовка символа слева направо

/**
 * Индексы строки, которая будет отображаться на матрице.
 * Направление имеет позицию 0;
 * MSB (старший бит, первый символ) имеет позицию 1;
 * LSB (младший бит, второй символ) имеет позицию 2.
 */
typedef enum { DIRECTION = 0, MSB = 1, LSB = 2, INDEX_NUMBER } symbol_index_t;

#define ROWS 8
#define COLS 8
#define DISPLAY_WIDTH 2 * COLS
#define GAP_BETWEEN_SYMBOLS 1

#define SYMBOL_WIDTH_POSITION 1

#if DINAMIC_ARROW
#define NUMBER_CYCLES_ANIMATION ROWS // Количество циклов анимации стрелки

#else
#define NUMBER_CYCLES_ANIMATION ROWS + 1 // Количество циклов анимации стрелки

#endif

#define PAUSE_AFTER_ANIMATION_MS 10

static uint16_t combined_bitmap[ROWS]
                               [DISPLAY_WIDTH]; // общий буфер для трёх символов

/// Для хранения текущей строки матрицы
static uint8_t current_row = 0;

typedef enum { ANIMATION_NONE, ANIMATION_UP, ANIMATION_DOWN } animation_e;

/* Структура, содержащая информацию о символах для отрисовки */
typedef struct {
  symbol_code_e symbol_code;
  uint8_t current_bitmap[NUMBER_CYCLES_ANIMATION];
  uint8_t current_frame_index;
  bool need_update;
  bool is_anim_running;

  bool pause_after_animation;
  uint16_t pause_ms_counter;
} animated_symbol_t;

/* Структура для дисплея */
typedef struct {
  animated_symbol_t symbol_left;
  animated_symbol_t symbol_prev_left;

  animated_symbol_t symbol_center;
  animated_symbol_t symbol_prev_center;

  animated_symbol_t symbol_right;
  animated_symbol_t symbol_prev_right;
} symbols_display_t;

symbols_display_t symbols = {
    .symbol_left = {SYMBOL_EMPTY, {0}, 0, false, false},
    .symbol_prev_left = {SYMBOL_EMPTY, {0}, 0, false, false},

    .symbol_center = {SYMBOL_EMPTY, {0}, 0, false, false},
    .symbol_prev_center = {SYMBOL_EMPTY, {0}, 0, false, false},

    .symbol_right = {SYMBOL_EMPTY, {0}, 0, false, false},
    .symbol_prev_right = {SYMBOL_EMPTY, {0}, 0, false, false},
};

/**
 * Структура для ширин символов
 */
typedef struct {
  const uint8_t *bitmap;
  uint8_t width;
} symbol_descriptor_t;

static symbol_descriptor_t symbols_meta[NUMBER_OF_SYMBOLS];

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
    ['K'] = SYMBOL_K,
};

//======================== PRIVATE FUNCTIONS PROTO =======================
static uint8_t calculate_symbol_width(const uint8_t *symbol_bitmap_rows);
static inline symbol_code_e char_to_symbol(char ch);
static void set_symbol(animated_symbol_t *sym, symbol_code_e code);

static void shift_bitmap_up(animated_symbol_t *sym);
static void shift_bitmap_down(animated_symbol_t *sym);
static void run_animation_step(animated_symbol_t *sym,
                               void (*shift_func)(animated_symbol_t *));
static void update_animation_arrow(animated_symbol_t *sym,
                                   animated_symbol_t *prev_sym);
static void update_symbols();

static void add_symbol_bitmap(animated_symbol_t *sym, uint8_t position);
static void add_symbol_to_row_buff(uint16_t *row_data, animated_symbol_t *sym,
                                   uint8_t row, uint8_t position);

static void combine_symbols(symbols_display_t *symbols);
static void prepare_row_for_display(symbols_display_t *symbols, uint8_t row,
                                    uint16_t *row_data);

static void draw_combined_bitmap();
static void driver_draw_row_buff(uint16_t *row_data);

//======================== PUBLIC FUNCTIONS ================================
/**
 * @brief Заполнение структуры: битмап и ширина символа (вызывается в main.c
 *        перед отрисовкой!!!)
 */
void init_symbols_width() {
  for (int i = 0; i < NUMBER_OF_SYMBOLS; i++) {
    symbols_meta[i].bitmap = bitmap[i];
    symbols_meta[i].width = calculate_symbol_width(bitmap[i]);
  }
}

static void prepare_symbol_floor_for_animation(animated_symbol_t *prev_sym,
                                               animated_symbol_t *curr_sym,
                                               symbol_code_e new_symbol_code) {

  // предыдущий символ
  *prev_sym = *curr_sym;
  // prev_sym->is_anim_running = true;

  // новый символ
  set_symbol(curr_sym, new_symbol_code);
  // curr_sym->is_anim_running = true;
  curr_sym->need_update = true;

  //  symbols.symbol_prev_center = symbols.symbol_center;
  // symbols.symbol_prev_center.is_anim_running = true;

  // новый символ
  // set_symbol(&symbols.symbol_center, symbol_center_code);
  // symbols.symbol_center.is_anim_running = true;
  // symbols.symbol_center.need_update = true;
}

/**
 * @brief  Установка символа направления движения
 * @param  direction_code: Код направления (из перечисления symbol_code_e)
 */
#if DINAMIC_ARROW
void set_direction_symbol(symbol_code_e direction_code) {
  set_symbol(&symbols.symbol_left, direction_code);
}
#else
void set_direction_symbol(symbol_code_e direction_code) {
  set_symbol(&symbols.symbol_left, direction_code);
}
#endif

/**
 * @brief  Установка символов для этажей
 * @param  symbol_center_code: Код центрального символа
 * @param  symbol_right_code:  Код правого символа
 */

extern bool is_special_regime_symbol;

#if DINAMIC_ARROW
void set_floor_symbols(symbol_code_e symbol_center_code,
                       symbol_code_e symbol_right_code) {
  set_symbol(&symbols.symbol_center, symbol_center_code);
  set_symbol(&symbols.symbol_right, symbol_right_code);
}
#else
void set_floor_symbols(symbol_code_e symbol_center_code,
                       symbol_code_e symbol_right_code) {

  bool is_left_symbol_arrow =
      (symbols.symbol_left.symbol_code == SYMBOL_ARROW_UP ||
       symbols.symbol_left.symbol_code == SYMBOL_ARROW_DOWN);

  // центральный символ
  if (symbols.symbol_center.symbol_code != symbol_center_code) {

    if (!is_left_symbol_arrow) {
      set_symbol(&symbols.symbol_center, symbol_center_code);
    } else {

      prepare_symbol_floor_for_animation(&symbols.symbol_prev_center,
                                         &symbols.symbol_center,
                                         symbol_center_code);

      // для стрелки
      symbols.symbol_left.need_update = true;
    }
  }

  // правый символ
  if (symbols.symbol_right.symbol_code != symbol_right_code) {

    if (!is_left_symbol_arrow) {
      set_symbol(&symbols.symbol_right, symbol_right_code);
    } else {

      prepare_symbol_floor_for_animation(
          &symbols.symbol_prev_right, &symbols.symbol_right, symbol_right_code);

      // для стрелки
      symbols.symbol_left.need_update = true;
    }
  }

  // if (is_special_regime_symbol && is_left_symbol_arrow) {
  //   symbols.symbol_left.need_update = true;
  // }
}
#endif

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

extern volatile bool is_time_ms_for_animation_update;
/**
 * @brief Отображение символов, заранее установленных в структуру symbols
 */
#if DINAMIC_ARROW
void draw_symbols() {

  uint8_t start_pos = 0;

  // Обновление символов для анимации
  if (is_time_ms_for_animation_update) {
    is_time_ms_for_animation_update = false;

    symbols.symbol_left.need_update = true;

    update_animation_arrow(&symbols.symbol_left, &symbols.symbol_prev_left);

    // update_animation(&symbols.symbol_center, ANIMATION_NONE);
    // update_animation(&symbols.symbol_right, ANIMATION_NONE);
  }

  // Отрисовка символов
  combine_symbols(&symbols);
  draw_combined_bitmap();
}
#else

void draw_symbols() {

  bool need_update_animation = is_time_ms_for_animation_update ||
                               (symbols.symbol_left.current_frame_index == 0);

  if (need_update_animation) {
    is_time_ms_for_animation_update = false;
    update_symbols();
  }

  // uint16_t row_data[DISPLAY_WIDTH] = {0}; // Буфер для строки дисплея
  uint16_t row_data = 0;
  prepare_row_for_display(&symbols, current_row, &row_data); // Заполняем буфер
  driver_draw_row_buff(&row_data); // Отображаем строку
}
#endif

/**
 * @brief Отображение строки
 * @param matrix_string: Указатель на строку, которая будет отображаться
 */
void draw_string(char *matrix_string) {
  // Преобразуем символ char в код symbol_code_e
  set_symbols(char_to_symbol(matrix_string[0]),
              char_to_symbol(matrix_string[1]),
              char_to_symbol(matrix_string[2]));
  draw_symbols();
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
    draw_symbols();
  }
}

//======================== PRIVATE FUNCTIONS ================================
/**
 * @brief Расчет ширины символа (заполнение структуры)
 * @param symbol_bitmap_rows: Двоичное представление символа
 * @return uint8_t
 */
static uint8_t calculate_symbol_width(const uint8_t *symbol_bitmap_rows) {
  uint8_t min_col = 8;
  uint8_t max_col = 0;
  bool has_pixels = false;

  for (int row = 0; row < NUMBER_OF_ROWS; row++) {
    uint8_t row_data = symbol_bitmap_rows[row];

    if (row_data != 0) {
      has_pixels = true;

      // Найдём первую установленную единицу слева (индексы от 7 до 0)
      for (int bit_index = 0; bit_index <= 7; ++bit_index) {
        if (row_data & (1 << (7 - bit_index))) {
          if (bit_index < min_col)
            min_col = bit_index;
          break;
        }
      }

      // Найдём последнюю установленную единицу справа
      for (int bit_index = 7; bit_index >= 0; --bit_index) {
        if (row_data & (1 << (7 - bit_index))) {
          if (bit_index > max_col)
            max_col = bit_index;
          break;
        }
      }
    }
  }

  if (!has_pixels)
    return 0; // символ без 1 (Пусто)

  return (max_col - min_col + 1);
}

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
 * @brief Инициализация структуры символа
 *
 * @param sym
 * @param code
 */
#if DINAMIC_ARROW
static void set_symbol(animated_symbol_t *sym, symbol_code_e code) {

  // Символ не меняется — не сбрасываем состояние (для анимации)
  if (sym->symbol_code == code) {
    return;
  }

  symbols.symbol_prev_left = symbols.symbol_left;

  sym->symbol_code = code;
  memcpy(sym->current_bitmap, bitmap[code], ROWS);
  sym->current_frame_index = 0;
  sym->need_update = false;
  sym->is_anim_running = false;
}
#else
static void set_symbol(animated_symbol_t *sym, symbol_code_e code) {

  symbols.symbol_prev_left = symbols.symbol_left;

  sym->symbol_code = code;
  memcpy(sym->current_bitmap, bitmap[code], ROWS);
  sym->current_bitmap[NUMBER_CYCLES_ANIMATION - 1] =
      0; // для пробела при анимации
  sym->current_frame_index = 0;
  sym->need_update = false;
  sym->is_anim_running = false;
}
#endif

/**
 * @brief Сдвиг битмапа символа вверх
 *
 * @param sym
 */
static void shift_bitmap_up(animated_symbol_t *sym) {
  uint8_t first = sym->current_bitmap[0];
  for (uint8_t i = 0; i < NUMBER_CYCLES_ANIMATION - 1; i++) {
    sym->current_bitmap[i] = sym->current_bitmap[i + 1];
  }
  sym->current_bitmap[NUMBER_CYCLES_ANIMATION - 1] = first;
}

/**
 * @brief Сдвиг битмапа вверх (предыдущий и текущий символы)
 *
 * @param prev_sym
 * @param new_sym
 */
static void shift_bitmap_up_prev_new(animated_symbol_t *prev_sym,
                                     animated_symbol_t *new_sym) {
  uint8_t first = new_sym->current_bitmap[0];
  for (uint8_t i = 0; i < NUMBER_CYCLES_ANIMATION - 1; i++) {
    prev_sym->current_bitmap[i] = prev_sym->current_bitmap[i + 1];
  }
  prev_sym->current_bitmap[NUMBER_CYCLES_ANIMATION - 1] =
      new_sym->current_bitmap[new_sym->current_frame_index];
}

/**
 * @brief Сдвиг битмапа символа вниз
 *
 * @param sym
 */
static void shift_bitmap_down(animated_symbol_t *sym) {
  uint8_t last = sym->current_bitmap[NUMBER_CYCLES_ANIMATION - 1];
  for (int i = NUMBER_CYCLES_ANIMATION - 1; i > 0; i--) {
    sym->current_bitmap[i] = sym->current_bitmap[i - 1];
  }
  sym->current_bitmap[0] = last;
}

/**
 * @brief Сдвиг битмапа вниз (предыдущий и текущий символы)
 *
 * @param prev_sym
 * @param new_sym
 */
static void shift_bitmap_down_prev_new(animated_symbol_t *prev_sym,
                                       animated_symbol_t *new_sym) {
  uint8_t last = new_sym->current_bitmap[NUMBER_CYCLES_ANIMATION -
                                         new_sym->current_frame_index - 1];
  for (int i = NUMBER_CYCLES_ANIMATION - 1; i > 0; i--) {
    prev_sym->current_bitmap[i] = prev_sym->current_bitmap[i - 1];
  }
  prev_sym->current_bitmap[0] = last;
}

/**
 * @brief Запуск цикла/шага анимации (отображение предыдущего и текущего
 *        символов)
 *
 * @param sym
 * @param shift_func
 */
static void run_animation_step_prev_new(
    animated_symbol_t *prev_sym, animated_symbol_t *new_sym,
    void (*shift_func)(animated_symbol_t *, animated_symbol_t *)) {
  prev_sym->is_anim_running = true;
  new_sym->is_anim_running = true;
  shift_func(prev_sym, new_sym);
  prev_sym->current_frame_index++;
  new_sym->current_frame_index++;
}

/**
 * @brief Запуск цикла/шага анимации
 *
 * @param sym
 * @param shift_func
 */
static void run_animation_step(animated_symbol_t *sym,
                               void (*shift_func)(animated_symbol_t *)) {
  sym->is_anim_running = true;
  shift_func(sym);
  sym->current_frame_index++;
}

/**
 * @brief Сброс состояния символа для завершения анимации
 *
 * @param sym
 */
static void stop_symbol_animation(animated_symbol_t *sym) {
  sym->need_update = false;
  sym->current_frame_index = 0;
  sym->is_anim_running = false;
}

/**
 * @brief Сброс состояния символов дисплея для завершения анимации
 *
 */
void stop_animation() {
  stop_symbol_animation(&symbols.symbol_left);
  stop_symbol_animation(&symbols.symbol_prev_left);

  stop_symbol_animation(&symbols.symbol_center);
  stop_symbol_animation(&symbols.symbol_prev_center);

  stop_symbol_animation(&symbols.symbol_right);
  stop_symbol_animation(&symbols.symbol_prev_right);
}

/**
 * @brief Обновление анимации стрелки
 * @param sym:      Указатель на текущий символ (новый)
 * @param prev_sym: Указатель на предыдущий символ (для которого активна
 *                  анимация)
 */
#if DINAMIC_ARROW
static void update_animation_arrow(animated_symbol_t *sym,
                                   animated_symbol_t *prev_sym) {

  if (!sym->need_update) {
    return;
  }

  sym->need_update = false;

  // Обработка паузы
  if (sym->pause_after_animation) {
    if (sym->pause_ms_counter > 0) {
      sym->pause_ms_counter--;
      return;
    } else {
      sym->pause_after_animation = false; // Выходим из паузы
      sym->current_frame_index = 0;
    }
  }

  // Завершение анимации
  if (sym->current_frame_index >= NUMBER_CYCLES_ANIMATION) {
    sym->current_frame_index = 0;
    sym->pause_after_animation = true;
    sym->pause_ms_counter = PAUSE_AFTER_ANIMATION_MS;
    return; // Пауза началась
  }

  // Проверка кода текущего символа -> анимация/плавное завершение
  switch (sym->symbol_code) {
  case SYMBOL_ARROW_UP_ANIMATION:
    run_animation_step(sym, shift_bitmap_up);
    break;

  case SYMBOL_ARROW_DOWN_ANIMATION:
    run_animation_step(sym, shift_bitmap_down);
    break;

  case SYMBOL_EMPTY:
  case SYMBOL_ARROW_UP:
  case SYMBOL_ARROW_DOWN:
    // Если текущий символ — неанимируемый, но предыдущий ещё анимируется
    if (prev_sym->current_frame_index < NUMBER_CYCLES_ANIMATION) {
      if (prev_sym->symbol_code == SYMBOL_ARROW_UP_ANIMATION) {
        run_animation_step(prev_sym, shift_bitmap_up);
      } else if (prev_sym->symbol_code == SYMBOL_ARROW_DOWN_ANIMATION) {
        run_animation_step(prev_sym, shift_bitmap_down);
      }
    } else {
      prev_sym->is_anim_running = false;
    }
    break;
  }

#if 0
  if (sym->symbol_code == SYMBOL_ARROW_UP_ANIMATION) {
    sym->is_anim_running = true;
    shift_bitmap_up(sym);
    sym->current_frame_index++;
  } else if (sym->symbol_code == SYMBOL_ARROW_DOWN_ANIMATION) {
    sym->is_anim_running = true;
    shift_bitmap_down(sym);
    sym->current_frame_index++;
  } else if (sym->symbol_code == SYMBOL_EMPTY ||
             sym->symbol_code == SYMBOL_ARROW_UP ||
             sym->symbol_code == SYMBOL_ARROW_DOWN) {
    // Если пришёл пустой символ, но анимация ещё не завершена
    if (prev_sym->current_frame_index != NUMBER_CYCLES_ANIMATION) {
      // Продолжаем анимацию с использованием prev_symbol_code
      if (prev_sym->symbol_code == SYMBOL_ARROW_UP_ANIMATION) {
        prev_sym->is_anim_running = true;
        shift_bitmap_up(prev_sym);
      } else if (prev_sym->symbol_code == SYMBOL_ARROW_DOWN_ANIMATION) {
        prev_sym->is_anim_running = true;
        shift_bitmap_down(prev_sym);
      }
      prev_sym->current_frame_index++;
    } else {
      prev_sym->is_anim_running = false;
    }
  }
#endif
}
#else
static void update_symbols() {

  if (!symbols.symbol_left.need_update) {
    return;
  }

  // Завершение анимации (пройдено NUMBER_CYCLES_ANIMATION для сдвига битмапа)
  if (symbols.symbol_left.current_frame_index >= NUMBER_CYCLES_ANIMATION) {
    stop_animation();
    return;
  }

  bool is_symbol_arrow_up =
      (symbols.symbol_left.symbol_code == SYMBOL_ARROW_UP);
  bool is_symbol_arrow_down =
      (symbols.symbol_left.symbol_code == SYMBOL_ARROW_DOWN);

  // АНИМАЦИЯ ПРИ ДВИЖЕНИИ ВВЕРХ
  if (is_symbol_arrow_up) {

    // Стрелка
    run_animation_step(&symbols.symbol_left, shift_bitmap_up);

    // Центральный символ
    if (symbols.symbol_center.need_update) {
      run_animation_step_prev_new(&symbols.symbol_prev_center,
                                  &symbols.symbol_center,
                                  shift_bitmap_down_prev_new);
    }

    // Правый символ
    if (symbols.symbol_right.need_update) {
      run_animation_step_prev_new(&symbols.symbol_prev_right,
                                  &symbols.symbol_right,
                                  shift_bitmap_down_prev_new);
    }
  } // if (is_symbol_arrow_up)

  // АНИМАЦИЯ ПРИ ДВИЖЕНИИ ВНИЗ
  if (is_symbol_arrow_down) {

    // Стрелка
    run_animation_step(&symbols.symbol_left, shift_bitmap_down);

    // Центральный символ
    if (symbols.symbol_center.need_update) {
      run_animation_step_prev_new(&symbols.symbol_prev_center,
                                  &symbols.symbol_center,
                                  shift_bitmap_up_prev_new);
    }

    // Правый символы
    if (symbols.symbol_right.need_update) {
      run_animation_step_prev_new(&symbols.symbol_prev_right,
                                  &symbols.symbol_right,
                                  shift_bitmap_up_prev_new);
    }
  } //  if (is_symbol_arrow_down)
}
#endif // !DINAMIC_ARROW

/**
 * @brief Добавление символа в общий буфер
 *
 * @param sym
 * @param position
 */
#if DINAMIC_ARROW
static void add_symbol_bitmap(animated_symbol_t *sym, uint8_t position) {

  uint8_t *symbol_bitmap = sym->current_bitmap;

  for (int row = 0; row < ROWS; row++) {
    for (int bit = 0; bit < START_INDEX_SYMBOL_ROW; bit++) {
      // Проверяем бит в символе
      if (symbol_bitmap[row] & (1 << (START_INDEX_SYMBOL_ROW - bit))) {
        combined_bitmap[row][position + bit] = 1;
      }
    }
  }
}
#else
static void add_symbol_to_row_buff(uint16_t *row_data, animated_symbol_t *sym,
                                   uint8_t row, uint8_t position) {
  uint8_t row_bits = sym->current_bitmap[row]; // Строка bitmap символа

  for (uint8_t bit = 0; bit < START_INDEX_SYMBOL_ROW; bit++) {
    if (row_bits & (1 << (START_INDEX_SYMBOL_ROW - bit))) {
      // row_data[position + bit] = 1;
      *row_data |= (1 << (position + bit));
    }
  }
}
#endif

#if !DINAMIC_ARROW
/**
 * @brief Расчет начальной позиции для центрирования символов
 *
 * @return uint8_t
 */
static uint8_t calculate_centered_start_position() {
  uint8_t total_width = symbols_meta[symbols.symbol_left.symbol_code].width +
                        symbols_meta[symbols.symbol_center.symbol_code].width +
                        symbols_meta[symbols.symbol_right.symbol_code].width +
                        2 * GAP_BETWEEN_SYMBOLS;

  return (total_width < DISPLAY_WIDTH) ? (DISPLAY_WIDTH - total_width) / 2 : 0;
}
#endif

#if DINAMIC_ARROW
static uint8_t start_pos;
#endif

/**
 * @brief Объединение символов в общий буфер для отображения
 *
 * @param symbols
 */
#if DINAMIC_ARROW
static void combine_symbols(symbols_display_t *symbols) {
  // Очистка буфера
  memset(combined_bitmap, 0, sizeof(combined_bitmap));

  start_pos = 0;
  // c1c..c9c, cFc и др.
  if (symbols->symbol_left.symbol_code == SYMBOL_EMPTY &&
      symbols->symbol_right.symbol_code == SYMBOL_EMPTY) {

    /* Завершение анимации (отрисовка стрелки) при symbol_left.symbol_code ==
     * SYMBOL_EMPTY */
    if (symbols->symbol_prev_left.is_anim_running) {
      add_symbol_bitmap(&symbols->symbol_prev_left, 0);
    }

    add_symbol_bitmap(&symbols->symbol_center, 6);

  } else if (symbols->symbol_left.symbol_code == SYMBOL_EMPTY &&
             symbols->symbol_right.symbol_code != SYMBOL_EMPTY) {

    /* Завершение анимации (отрисовка стрелки) при symbol_left.symbol_code ==
     * SYMBOL_EMPTY */
    if (symbols->symbol_prev_left.is_anim_running) {
      add_symbol_bitmap(&symbols->symbol_prev_left, 0);
      start_pos += 6;
      add_symbol_bitmap(&symbols->symbol_center, start_pos);
    } else {
      // c10..c99, cКГ и др.
      // Сдвиг в связи с символом Пусто
      start_pos += 3 + GAP_BETWEEN_SYMBOLS;
      add_symbol_bitmap(&symbols->symbol_center, start_pos);
    }

    start_pos += symbols_meta[symbols->symbol_center.symbol_code].width +
                 GAP_BETWEEN_SYMBOLS;
    add_symbol_bitmap(&symbols->symbol_right, start_pos);

  } else {
    // Если символ 1 не SYMBOL_EMPTY -> стрелка
    if (symbols->symbol_left.symbol_code == SYMBOL_ALL_ON) {
      add_symbol_bitmap(&symbols->symbol_left, 0);
      add_symbol_bitmap(&symbols->symbol_left, 8);
    } else {

      // левый символ
      add_symbol_bitmap(&symbols->symbol_left, 0);

      start_pos += symbols_meta[symbols->symbol_left.symbol_code].width +
                   GAP_BETWEEN_SYMBOLS;

      // центральный символ
      add_symbol_bitmap(&symbols->symbol_center, start_pos);
      start_pos += symbols_meta[symbols->symbol_center.symbol_code].width +
                   GAP_BETWEEN_SYMBOLS;

      // правый символ
      add_symbol_bitmap(&symbols->symbol_right, start_pos);
    }
  }
}
#else
static void prepare_row_for_display(symbols_display_t *symbols, uint8_t row,
                                    uint16_t *row_data) {
  uint8_t start_pos = 0;

  bool is_symbol_left_empty =
      (symbols->symbol_left.symbol_code == SYMBOL_EMPTY);
  bool is_symbol_right_empty =
      (symbols->symbol_right.symbol_code == SYMBOL_EMPTY);
  bool is_symbol_left_arrow =
      (symbols->symbol_left.symbol_code == SYMBOL_ARROW_UP ||
       symbols->symbol_left.symbol_code == SYMBOL_ARROW_DOWN);

  if (is_symbol_left_empty && is_symbol_right_empty) {
    add_symbol_to_row_buff(row_data, &symbols->symbol_center, row, 6);

  } else if (is_symbol_left_empty && !is_symbol_right_empty) {
    start_pos = 3 + GAP_BETWEEN_SYMBOLS;
    add_symbol_to_row_buff(row_data, &symbols->symbol_center, row, start_pos);

    start_pos += symbols_meta[symbols->symbol_center.symbol_code].width +
                 GAP_BETWEEN_SYMBOLS;
    add_symbol_to_row_buff(row_data, &symbols->symbol_right, row, start_pos);

  } else if (is_symbol_left_arrow) {
    add_symbol_to_row_buff(row_data, &symbols->symbol_left, row, 0);

    add_symbol_to_row_buff(row_data,
                           symbols->symbol_prev_center.is_anim_running
                               ? &symbols->symbol_prev_center
                               : &symbols->symbol_center,
                           row, 7);

    add_symbol_to_row_buff(row_data,
                           symbols->symbol_prev_right.is_anim_running
                               ? &symbols->symbol_prev_right
                               : &symbols->symbol_right,
                           row, 12);

  } else {
    start_pos = calculate_centered_start_position();

    add_symbol_to_row_buff(row_data, &symbols->symbol_left, row, start_pos);
    start_pos += symbols_meta[symbols->symbol_left.symbol_code].width +
                 GAP_BETWEEN_SYMBOLS;

    add_symbol_to_row_buff(row_data, &symbols->symbol_center, row, start_pos);
    start_pos += symbols_meta[symbols->symbol_center.symbol_code].width +
                 GAP_BETWEEN_SYMBOLS;

    add_symbol_to_row_buff(row_data, &symbols->symbol_right, row, start_pos);
  }
}
#endif

extern volatile bool is_tim4_period_elapsed;
/**
 * @brief Отображение символов на матрице
 *
 */
#if DINAMIC_ARROW
static void draw_combined_bitmap() {

  // Включаем текущую строку
  set_row_state(current_row, TURN_ON);

  for (uint8_t col = 0; col < DISPLAY_WIDTH; col++) {
    // Если бит в строке буфера = 1, то включаем колонку
    if (combined_bitmap[current_row][col]) {
      set_col_state(col, TURN_ON);
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
#else
static void driver_draw_row_buff(uint16_t *row_data) {

  // Включаем текущую строку
  set_row_state(current_row, TURN_ON);

  // Включаем нужные коллонки
  for (uint8_t col = 0; col < DISPLAY_WIDTH; col++) {
    if (*row_data & (1 << col)) {
      set_col_state(col, TURN_ON);
    }
  }
  // for (uint8_t col = 0; col < DISPLAY_WIDTH; col++) {
  //   if (row_data[col]) {
  //     set_col_state(col, TURN_ON);
  //   }
  // }

  /**
   * Держим состояние строки с колонками, пока таймер не завершит
   * отсчет (1000 мкс)
   */
  if (is_tim4_period_elapsed) {
    is_tim4_period_elapsed = false;

    // Выключаем текущую строку и все колонки
    set_row_state(current_row, TURN_OFF);
    set_all_cols_state(TURN_OFF);

    // Переходим к следующей строке
    current_row++;

    // Завершаем проход по строкам дисплея
    if (current_row >= ROWS)
      current_row = 0;
  }
}
#endif

//================================= OLD CODE =======================
#if 0
static void draw_symbol_on_matrix2(animated_symbol_t *sym, uint8_t start_pos) {

  static uint8_t current_row = 0;

  // Включаем текущую строку
  set_row_state(current_row, TURN_ON);

  // Получаем значения для колонок текущей строки (строка кода символа)
  uint8_t binary_symbol_code_row = sym->current_bitmap[current_row];

  for (uint8_t i = 0; i <= START_INDEX_SYMBOL_ROW; i++) {
    // Если бит в строке символа = 1, то включаем колонку
    if ((binary_symbol_code_row >> (START_INDEX_SYMBOL_ROW - i)) & 1) {
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

/* Флаг для удержания состояния строки в течение 1 мс (максимальная яркость,
 * частота обновления матрицы 125 Гц) */
extern volatile bool is_tim4_period_elapsed;
/**
 * @brief  Отображение символа на матрице
 * @note   Построчно проходим по коду символа, удерживая состояние строки 1 мс
 *         (максимвльная яркость, частота обновления матрицы 125 Гц)
 * @param  symbol_code: Код символа для отображения (из font.h)
 * @param  start_pos:   Начальная позиция (индекс столбца матрицы) для символа
 */
static void draw_symbol_on_matrix(symbol_code_e symbol_code,
                                  uint8_t start_pos) {

  static uint8_t current_row = 0;

  // Включаем текущую строку
  set_row_state(current_row, TURN_ON);

  // Получаем значения для колонок текущей строки (строка кода символа)
  uint8_t binary_symbol_code_row = bitmap[symbol_code][current_row];

  for (uint8_t i = 0; i <= START_INDEX_SYMBOL_ROW; i++) {
    // Если бит в строке символа = 1, то включаем колонку
    if ((binary_symbol_code_row >> (START_INDEX_SYMBOL_ROW - i)) & 1) {
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

#endif