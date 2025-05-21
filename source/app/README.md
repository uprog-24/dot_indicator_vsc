# Бизнес-логика программы

## **[<- Вернуться назад](../README.md)**

## 📂 **[app](../app/)**

- 📄 **[config.h](./config.h)** предназначен для задания параметров для выбранного протокола/режима, который указывается при сборке проекта [см. README.md](../../README.md);
- 📄 **[main.c](./main.c)** содержит логику работы программы: инициализация индикатора, прием и обработка данных от СУЛ, работа с меню.

### protocol_selection

- 📄 <a id="protocol_selection_h"></a> **[protocol_selection.h](./protocol_selection.h)** содержит прототипы функций для работы с протоколом (инициализация, старт, стоп обработки);
- 📄 **[protocol_selection.c](./protocol_selection.c)** содержит реализацию методов [protocol_selection.h](#protocol_selection_h).
