# Демонстрационный режим

Версия демо режима на ветке **_main_** текущего репозитория.

## **[<- Вернуться назад](../protocols_modes.md)**

Модуль обработки режима расположен в 📂 **[demo_mode](../demo_mode/)**.

Изначально индикатор стоит на этаже `START_FLOOR 1`, затем начинает движение вверх на этаж `FINISH_FLOOR 14`, останавливаясь по пути на этажах из `buff_stop_floors[STOP_FLOORS_BUFF_SIZE] = {7, 8, 10, 11}`. Далее индикатор возвращается на этаж `START_FLOOR 1`. Отображение каждого состояния происходит в течение `TIME_DISPLAY_STRING_DURING_MS 2000` (2 секунды) ([tim.c](../../peripherals/tim.c)).
