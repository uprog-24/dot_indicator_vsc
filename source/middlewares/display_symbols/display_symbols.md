# Модули отображения символов

## **[<- Вернуться назад](../../middlewares/middlewares.md)**

## Модули отображения символов расположены в 📂 **[display_symbols](../display_symbols/)**

### **drawing**

- 📄 <a id="drawing_h"></a> **[drawing.h](./drawing.h)** содержит прототипы функций для работы с символами, а также общие для всех протоколов перечисления и структуры:

```c
/**
 * Общие значения направления движения (для всех протоколов и режимов,
 * определённых в config.h). У каждого протокола есть функция
 * transform_direction_to_common, которая преобразует значения направления
 * протокола в общий тип directionType.
 */
typedef enum { NO_DIRECTION, DIRECTION_UP, DIRECTION_DOWN } directionType;

/**
 * Структура содержит этаж (код) и направление движения (для протоколов).
 */
typedef struct drawing_data {
  uint16_t floor;
  directionType direction;
} drawing_data_t;

/**
 * Структура содержит код местоположения и соответствующую ему строку для
 * отображения (для протоколов).
 */
typedef struct {
  uint16_t code_location;
  char symbols[3];
} code_location_symbols_t;

/**
 * Индексы строки, которая будет отображаться на матрице.
 * Направление имеет позицию 0;
 * MSB (старший бит, первый символ) имеет позицию 1;
 * LSB (младший бит, второй символ) имеет позицию 2.
 */
enum { DIRECTION = 0, MSB = 1, LSB = 2 };
```

- 📄 **[drawing.c](./drawing.c)** содержит реализацию методов [drawing.h](#drawing_h).

### **font**

- 📄 <a id="font_h"></a> **[font.h](./font.h)** содержит прототипы функций для работы со шрифтами:

```c
/**
 * @brief  Получение кода символа из массива symbols[].
 * @param  symbol:                 Символ из symbols[] (font.c).
 * @retval Указатель на buff_code: Код символа.
 */
uint8_t *get_symbol_code(char symbol);

/**
 * @brief  Преобразование десятичного числа в двоичный массив (для получения
 *         двоичных строк символа в drawing.c).
 * @param  number:     Число для двоичной строки символа.
 * @param  binary_mas: Указатель на массив с двоичным представлением строки
 *                     символа (из font.c symbols[]).
 * @param  bin_size:   Размер двоичной строки символа.
 * @retval None
 */
void convert_number_from_dec_to_bin(uint8_t number, uint8_t *binary_mas,
                                    uint8_t bin_size);
```

- 📄 **[font.c](./font.c)** содержит реализацию методов [font.h](#font_h) и буфер с символами (шрифты), которые задаются структурой:

```c
/**
 * Параметры символа: символ и его бинарное представление.
 */
typedef struct {
  char symbol;
  uint8_t buff_code[BINARY_SYMBOL_SIZE];
} symbol_t;
```
