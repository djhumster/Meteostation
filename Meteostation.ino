/*
    Written by Pavel Vishnyakov
    Метеостанция v0.2a
    Требуются библиотеки:
    http://arduino-info.wikispaces.com/LCD-Blue-I2Chttp://arduino-info.wikispaces.com/LCD-Blue-I2C
    https://learn.adafruit.com/dhthttps://learn.adafruit.com/dht
    последняя требует доп. библиотеку, ВНИМАТЕЛЬНО читать мануал.
*/
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// задаем тип датчика влажности
//#define DHTTYPE DHT22
#define DHTTYPE DHT11

/*
    Подключаем пин 1 (слева) сенсора к 5V
    ВНИМАНИЕ: если используется микроконтроллер с 3.3V логикой (типа Arduino Due) пин 1 подключаем
    к 3.3V вместо 5V
    Подключаем пин 2 сенсора к любому цифровому пину (это DHTPIN)
    Пин 3 не используется
    Подключаем пин 4 сенсора к земле GND
    Подключаем резистор 10 КОм между пинами 1 и 2 сенсора
*/
#define DHTPIN 2 // пин микроконтроллера к которому подключен DHT11/DHT22

#define LCD_CHAR 20 // сколько символов на строку экрана
#define LCD_LINE 4 // сколько строк экран

DHT dht(DHTPIN, DHTTYPE);

// адрес i2c экрана как правило 0x27 или 0x3F
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// знак Цельсия
byte degree[8] = {
  B00010,
  B00101,
  B00010,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

unsigned long last_time = 0; // время для задержки
boolean l = 0;
byte l1, l2;

void setup() {
  lcd.begin(LCD_CHAR, LCD_LINE);  // инициальзация экрана и включение подсветки
  lcd.backlight();
  lcd.clear();
  lcd.createChar(0, degree);  //  записываем символ Цельсия в память экрана

  dht.begin();  //  инициальзация датчика влажности
}

void loop() {
  if (millis() - last_time > 5000) {
    last_time = millis() - 1;
    lcd.clear();

    float h = dht.readHumidity();  //влажность
    float t = dht.readTemperature();  //температура

    if (isnan(h) || isnan(t)) {
      lcd.home();
      lcd.print("Failed to read from sensor!");
      return;
    }

    if (l == 1) {
      l = 0;
      l1 = 0;
      l2 = 1;
    } else {
      l = 1;
      l1 = 2;
      l2 = 3;
    }
    
    lcd.setCursor(0, l1);
    lcd.print("Temp: ");
    lcd.print(t);
    lcd.print(" ");
    lcd.write((byte)0);
    lcd.print("C");
    lcd.setCursor(0, l2);
    lcd.print("Humidity: ");
    lcd.print(h);
    lcd.print(" %");
  }
}
