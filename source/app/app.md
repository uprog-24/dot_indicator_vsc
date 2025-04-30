# Бизнес-логика программы

## **[<- Вернуться назад](../source.md)**

## 📂 **[app](../app/)**

- 📄 **[config.h](./config.h)** предназначен для задания параметров для выбранного протокола/режима, который указывается при сборке проекта [см. README.md](../../README.md);
- 📄 **[main.c](./main.c)** содержит логику работы программы (управление состоянием матрицы и состоянием меню (перечислены в [main.h](../../Core/Inc/main.h)) в главном цикле);

### protocol_selection

- 📄 <a id="protocol_selection_h"></a> **[protocol_selection.h](./protocol_selection.h)** содержит прототипы функций для работы с протоколом (с интерфейсом: CAN, UART; выводом GPIO);
- 📄 **[protocol_selection.c](./protocol_selection.c)** содержит реализацию методов [protocol_selection.h](#protocol_selection_h).
