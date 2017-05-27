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

const String VERSION = "v0.3d"; //  версия кода

DHT dht(DHTPIN, DHTTYPE);

// адрес i2c экрана как правило 0x27 или 0x3F
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

RTC_DS1307 rtc;

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
}

void loop() {
  if (millis() - last_time > 10000) {
    last_time = millis() - 1;

    DateTime now = rtc.now();
    
    byte h = dht.readHumidity();  //влажность
    int t = dht.readTemperature();  //температура

    if (isnan(h) || isnan(t)) {
      lcd.home();
      lcd.print("Failed!");
      Serial.println("ERROR while reading from DHT sensor");
      return;
    }
    // убрать line, если НЕ нужно перемещать строки
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    add_zero(t);
    lcd.print(t);
    lcd.write((byte)0);
    lcd.print("C ");
    lcd.write(icon(t, 1));
    lcd.setCursor(0, 1);
    lcd.print("Humi: ");
    add_zero(h);
    lcd.print(h);
    lcd.print(" % ");
    lcd.write(icon(h, 2));
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
    lcd.setCursor(15, 0);
    lcd.print(VERSION);

    h_prev = h;
    t_prev = t;
  }
}
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
