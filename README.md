# Meteostation
![Прототип](https://github.com/djhumster/Meteostation/blob/master/proto.JPG?raw=true)

     Кнопка МЕНЮ:
      клик (в меню) - меняет страницы меню
      двойной клик - включает/выключает меню с сохранением изменений
      длинный клик (в меню) - выход из меню, без сохранения изменений
     Кнопка "+":
      клик (в меню) - +1 к значение
      двойной клик (вне меню) - вкл/выкл подсветку экрана
      двойной клик (в меню) - +5 к значению, -1 для года
      
### Компоненты:  
|Кол-во|Наименование|
|------|------|
|1|плата Arduino Uno (Rev3)|
|1|датчик влажности DHT11 или DHT22|
|1|LCD символьный экран с платой i2c шины (LCM2004 IIC)|
|1|Резистор 10 кОм|
|1|Макетная плата|  
|1|Модуль часов реального времени I2C RTC DS1307|
|2|Кнопка тактовая|
|1|Модуль Барометра BMP180|

### Требуются библиотеки:
[New Liquid Crystal](http://arduino-info.wikispaces.com/LCD-Blue-I2C)

[DHT-sensor-library](https://learn.adafruit.com/dht) и 
[Adafruit BMP085](https://github.com/adafruit/Adafruit_BMP085_Unified) 
> последние 2 требуют доп. библиотеку - [Adafruit Unified Sensor Driver](https://github.com/adafruit/Adafruit_Sensor), доступна в 
**диспетчере библиотек Arduino IDE**  
> ![Arduino IDE](https://cdn-learn.adafruit.com/assets/assets/000/038/115/medium800/weather_Screen_Shot_2016-12-26_at_12.52.24.png?1482753413)

[Time + DS1307RTC](https://playground.arduino.cc/Code/Time)
>      Time - библиотека времени
>      DS1307RTC - библиотека для модуля DS1307/DS1337/DS3231
>      TimeAlarms (сейчас не используется) - можно добавить будильники/таймеры событий

[OneButton](https://github.com/mathertel/OneButton) - работа с кнопками

### Макетная схема:  
![Breadboard1](https://github.com/djhumster/Meteostation/blob/master/Meteostation_bb.png?raw=true)
### Принципиальная схема:  
![Sheme](https://github.com/djhumster/Meteostation/blob/master/Meteostation_sheme.png?raw=true)
