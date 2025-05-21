# Протоколы и режимы для точечного индикатора

## **[<- Вернуться назад](../../middlewares/README.md)**

## Модули протоколов и режимов расположены в 📂 **[protocols_modes](./)** <br>

Для выбора протокола/режима требуется указать значение для параметра -DUSE_MODE (указано ниже) при [сборке проекта](../../../README.md).

Протоколы и соответствующие им значения для -DUSE_MODE:

1. **[НКУ-SD7](./nku_sd7/README.md)** -> USE_PROTOCOL_NKU_SD7

Режимы и соответствующие им значения для -DUSE_MODE:

1. **[Тестовый режим](./test_mode/README.md)** -> USE_TEST_MODE
2. **[Демонстрационный режим](./demo_mode/README.md)** -> USE_DEMO_MODE

## Протоколы

Каждый из протоколов включает в себя общие зависимости:

```c
#include "buzzer.h"  # Звуковые оповещения (гонг)
#include "config.h"  # Конфиги для протокола
#include "drawing.h" # Отображение строки на матрице
#include "tim.h"     # Звуковые оповещения (старт-стоп звука)
```

Есть экземпляр общей для протоколов структуры из **[drawing.h](../../middlewares/display_symbols/drawing.h)**:

```c
/// Структура с данными для отображения (direction, floor).
static drawing_data_t drawing_data = {0, 0};
```

В реализации протоколы имеют следующие функции:

1. Обработка полученных данных: символы и звуки. Функция содержит блок для кабинного и этажного индикатора.

```c
void process_data_X()
```

2. Функции для настройки гонга прибытия и спец. режимов:

```c
static void setting_gong()
static void cabin_indicator_special_regime()
static void floor_indicator_special_regime()
```
