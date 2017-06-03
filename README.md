# Meteostation
![Прототип](https://github.com/djhumster/Meteostation/blob/master/proto.JPG?raw=true)

     Кнопка МЕНЮ (S1):
      клик - меняет страницы меню
      двойной клик - включает/выключает меню с сохранением изменений
      длинный клик - выход из меню, без сохранения изменений
     Кнопка "+" (S2):
      клик - в меню уввеличивает значение
      двойной клик - вкл/выкл подсветку экрана (вне меню)
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
### Требуются библиотеки:
[New Liquid Crystal](http://arduino-info.wikispaces.com/LCD-Blue-I2C)  
[DHT-sensor-library](https://learn.adafruit.com/dht)  
> последняя требует доп. библиотеку - [Adafruit Unified Sensor Driver](https://github.com/adafruit/Adafruit_Sensor), доступна в 
**диспетчере библиотек Arduino IDE**  
> ![Arduino IDE](https://cdn-learn.adafruit.com/assets/assets/000/038/115/medium800/weather_Screen_Shot_2016-12-26_at_12.52.24.png?1482753413)

[Adafruit RTClib](https://github.com/adafruit/RTClib/)
### Макетная схема:  
![Breadboard1](https://github.com/djhumster/Meteostation/blob/master/Meteostation_bb.png?raw=true)
### Принципиальная схема:  
![Sheme](https://github.com/djhumster/Meteostation/blob/master/Meteostation_sheme.png?raw=true)
