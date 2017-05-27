/*
    Written by Pavel Vishnyakov
    Метеостанция
    Требуются библиотеки:
    http://arduino-info.wikispaces.com/LCD-Blue-I2C
    https://learn.adafruit.com/dht
    https://github.com/adafruit/RTClib
    последняя требует доп. библиотеку, ВНИМАТЕЛЬНО читать мануал.
*/
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"

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

const String VERSION = "v0.4d"; //  версия кода

DHT dht(DHTPIN, DHTTYPE);

// адрес i2c экрана как правило 0x27 или 0x3F
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

RTC_DS1307 rtc;

//  знак температуры
byte temp_ico[8] = {
  B00100,
  B01010,
  B01010,
  B01110,
  B01110,
  B11111,
  B11111,
  B01110
};
//  знак влажности
byte humi_ico[8] = {
  B00100,
  B00100,
  B01010,
  B01010,
  B10001,
  B10001,
  B10001,
  B01110
};
// знак барометр
byte bar_ico[8] = {
  B00100,
  B01010,
  B01010,
  B01010,
  B01010,
  B10001,
  B11111,
  B01110
};
// знак Цельсия
byte degree[8] = {
  B00011,
  B00011,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
// тире
byte dash[8] = {
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000
};
// стрелки вверх
byte arrowUp[8] = {
  B00000,
  B00100,
  B01110,
  B11111,
  B00000,
  B00100,
  B01110,
  B11111
};
// стрелки вниз
byte arrowDown[8] = {
  B00000,
  B11111,
  B01110,
  B00100,
  B00000,
  B11111,
  B01110,
  B00100
};

unsigned long last_time = 0; // время для задержки
unsigned long last_time2 = 0;

byte h_prev;  //  предыдущие значения температуры и влажности
int t_prev;

char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

void setup() {
  Serial.begin(9600);
  Serial.println("LCD start...");

  lcd.begin(20, 4);  // инициальзация экрана и включение подсветки
  lcd.backlight();
  lcd.clear();
  lcd.createChar(0, degree);  //  записываем символы в память экрана
  lcd.createChar(1, dash);
  lcd.createChar(2, arrowUp);
  lcd.createChar(3, arrowDown);
  lcd.createChar(4, temp_ico);
  lcd.createChar(5, humi_ico);
  lcd.createChar(6, bar_ico);

  Serial.println("DHT start...");
  dht.begin(); //  инициальзация датчика влажности

  Serial.println("RTC start...");
  rtc.begin(); // инициализация часов
  if (!rtc.isrunning()) {
    Serial.println("RTC not started!");
    while (1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //  прописывает в RTC время компиляции скетча

  Serial.println("Complete!");
  lcd.setCursor(15, 0);
  lcd.print(VERSION);
}

void loop() {
  // --- ЧАСЫ ---
  if (millis() - last_time2 > 2000) {
    last_time2 = millis() - 1;

    DateTime now = rtc.now();

    lcd.setCursor(0, 3);
    add_zero(now.day());
    lcd.print(now.day(), DEC);
    lcd.print(" ");
    lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
    lcd.print(" ");
    add_zero(now.hour());
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    add_zero(now.minute());
    lcd.print(now.minute(), DEC);
    lcd.print(" ");
    add_zero(now.month());
    lcd.print(now.month(), DEC);
    lcd.print('/');
    lcd.print(now.year(), DEC);
  }
  // --- МЕТЕОСТАНЦИЯ ---
  if (millis() - last_time > 60000) {
    last_time = millis() - 1;

    byte h = dht.readHumidity();  //влажность
    int t = dht.readTemperature();  //температура

    if (isnan(h) || isnan(t)) {
      lcd.home();
      lcd.print("Failed!");
      Serial.println("ERROR while reading from DHT sensor");
      return;
    }

    lcd.setCursor(0, 0);
    lcd.write((byte)4);
    lcd.print(" ");
    add_zero(t);
    lcd.print(t);
    lcd.write((byte)0);
    lcd.print("C ");
    lcd.write(icon(t, 1));
    lcd.setCursor(0, 1);
    lcd.write((byte)5);
    lcd.print(" ");
    add_zero(h);
    lcd.print(h);
    lcd.print(" % ");
    lcd.write(icon(h, 2));
    lcd.print(" ");
    lcd.write((byte)6);
    lcd.print(" 123 mm");

    h_prev = h;
    t_prev = t;
  }
}
// --- ДОПОЛНИТЕЛЬНЫЕ ФУНКЦИИ ---
/*
  функция выбора иконки изменения состояния температры/влажности
*/
byte icon(int i_cur, byte mode) {
  switch (mode) {
    // режим выбора иконки температуры
    case 1: {
        if (t_prev == i_cur) {
          return 1;
        }
        if (t_prev < i_cur) {
          return 2;
        }
        if (t_prev > i_cur) {
          return 3;
        }
      }
      break;
    // режим выбора иконки влажности
    case 2: {
        if (h_prev == i_cur) {
          return 1;
        }
        if (h_prev < i_cur) {
          return 2;
        }
        if (h_prev > i_cur) {
          return 3;
        }
      }
      break;
  }
}
/*
   функция добавления 0 перед однозначным числом
*/
void add_zero(int i) {
  if (i < 10) {
    lcd.print(0);
  }
}
