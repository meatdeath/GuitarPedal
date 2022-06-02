# GuitarPedal

За основу взята схема из комментария к статье по ссылке
https://radioprog.ru/post/533

Схема сохранена в файле Source_schematic.jpg
Вместо графического дисплея подключен текстовый двустрочный HD44780 совместимый дисплей с I2C адаптером. Вместо енкодера используются 4 кнопки на пинах D4-D7.
* D4 - Left
* D5 - Right
* D6 - Enter
* D7 - Exit
* D12 - педаль включения-выключения

Примеры экранов:
![alt text](https://github.com/meatdeath/GuitarPedal/blob/main/Screen_examples/IMG_20220601_212732.jpg)
![alt text](https://github.com/meatdeath/GuitarPedal/blob/main/Screen_examples/IMG_20220601_212746.jpg)
![alt text](https://github.com/meatdeath/GuitarPedal/blob/main/Screen_examples/IMG_20220601_212823.jpg)

Полезные ссылки:
https://arduinoplus.ru/gitarnaya-pedal-arduino-uno/
...