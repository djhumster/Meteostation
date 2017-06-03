/*
    Written by Pavel Vishnyakov
    Метеостанция
    Требуются библиотеки:
    http://arduino-info.wikispaces.com/LCD-Blue-I2C
    https://learn.adafruit.com/dht требует доп. библиотеку, ВНИМАТЕЛЬНО читать ман
    https://github.com/adafruit/RTClib
    https://github.com/mathertel/OneButton
*/
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <OneButton.h>

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
#define MENU_BTN_PIN 3  //  пин кнопки меню
#define UP_BTN_PIN 4  //  пин кнопки увеличения значений

const String VERSION = "v0.4d"; //  версия кода

DHT dht(DHTPIN, DHTTYPE);

// адрес i2c экрана как правило 0x27 или 0x3F
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

RTC_DS1307 rtc;

OneButton menu_btn(MENU_BTN_PIN, true);  // кнопка меню
OneButton up_btn(UP_BTN_PIN, true);  //  кнопка "+" увеличения значений

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
  B01110,
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

boolean light = 1;  //  состояние подсветки экрана, в перспективе и другой подсветки

unsigned long last_time = 0; // время для задержки
unsigned long last_time2 = 0;

byte h_prev;  //  предыдущие значения температуры и влажности
int t_prev;

char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
//  переменные для меню и ручного выбора даты/времени
byte menu_mode = 0, set_hour = 0, set_minute = 0, set_day = 1, set_month = 1;
int set_year = 1970;

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
  // инициализация часов
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (!rtc.isrunning()) {
    Serial.println("RTC not running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //  прописывает в RTC время компиляции скетча
  }
  /*
     подключаем функции кнопок
     Кнопка МЕНЮ:
      клик - меняет страницы меню
      двойной клик - включает/выключает меню с сохранением изменений
      длинный клик - выход из меню, без сохранения изменений
     Кнопка "+":
      клик - в меню уввеличивает значение
      двойной клик - вкл/выкл подсветку экрана (везде)
  */
  menu_btn.attachClick(m_click);
  menu_btn.attachDoubleClick(m_double_click);
  menu_btn.attachLongPressStart(m_long_press_start);
  menu_btn.attachLongPressStop(m_long_press_stop);
  up_btn.attachClick(up_click);
  up_btn.attachDoubleClick(up_double_click);

  Serial.println("Complete!");
}

void loop() {
  //  отслеживаем нажатия кнопок
  menu_btn.tick();
  up_btn.tick();

  if ((menu_mode == 0) && (millis() - last_time2 > 2000)) {
    last_time2 = millis();
    my_clock(); //  отображение времени
  }
  if ((menu_mode == 0) && (millis() - last_time > 60000)) {
    last_time = millis();
    weather();  // метеофстанция
  }
}

// --- МЕТЕОСТАНЦИЯ ---
void weather() {
  lcd.setCursor(15, 0);
  lcd.print(VERSION);

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
// --- ЧАСЫ ---
void my_clock() {
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
// --- МЕНЮ И КНОПКИ ---
void m_click() {
  if (menu_mode > 0) {
    menu_mode++;
    menu();
  }
  if (menu_mode > 6) {
    menu_mode = 1;
    menu();
  }
}

void m_double_click() {
  if (menu_mode == 0) {
    menu_mode = 1;

    DateTime now = rtc.now();

    set_hour = now.hour();
    set_minute = now.minute();
    set_day = now.day();
    set_month = now.month();
    set_year = now.year();

    menu();
  } else {
    rtc.adjust(DateTime(set_year, set_month, set_day, set_hour, set_minute, 0));
    lcd.clear();
    menu_mode = 0;
  }
}

void m_long_press_start() {
  if (menu_mode > 0) {
    lcd.clear();
    lcd.home();
    lcd.print("Exit without saving");
  }
}

void m_long_press_stop() {
  if (menu_mode > 0) {
    lcd.clear();
    menu_mode = 0;
  }
}
//  кнопка "+" увеличивает звначения в зависимости от выбранного пункта меню
void up_click() {
  if (menu_mode > 0) {
    switch (menu_mode) {
      case 1:
        if (set_hour < 23) {
          set_hour++;
        } else {
          set_hour = 0;
        }
        break;
      case 2:
        if (set_minute < 59 ) {
          set_minute++;
        } else {
          set_minute = 0;
        }
        break;
      case 3:
        if (set_day < 31) {
          set_day++;
        } else {
          set_day = 1;
        }
        break;
      case 4:
        if (set_month < 12) {
          set_month++;
        } else {
          set_month = 1;
        }
        break;
      case 5: set_year++;
        break;
    }
    menu();
  }
}
//  отрисовка меню
void menu() {
  lcd.clear();
  lcd.home();
  lcd.print("Settings");
  lcd.setCursor(15, 0);
  lcd.print(VERSION);
  lcd.setCursor(0, 2);
  switch (menu_mode) {
    case 1:
      lcd.print("Hour: ");
      add_zero(set_hour);
      lcd.print(set_hour);
      break;
    case 2:
      lcd.print("Minute: ");
      add_zero(set_minute);
      lcd.print(set_minute);
      break;
    case 3:
      lcd.print("Day: ");
      add_zero(set_day);
      lcd.print(set_day);
      break;
    case 4:
      lcd.print("Month: ");
      add_zero(set_month);
      lcd.print(set_month);
      break;
    case 5:
      lcd.print("Year: ");
      lcd.print(set_year);
      break;
    default:
      lcd.setCursor(0, 1);
      lcd.print("Double menu click to");
      lcd.setCursor(0, 2);
      lcd.print("Save & Exit or");
      lcd.setCursor(0, 3);
      lcd.print("Long click for Exit");
      break;
  }
}

void up_double_click() {
  if (light) {
    lcd.noBacklight();
    light = 0;
  } else {
    lcd.backlight();
    light = 1;
  }
}

// --- ДОПОЛНИТЕЛЬНЫЕ ФУНКЦИИ ---
/*
  функция выбора иконки изменения состояния температры/влажности
*/
byte icon(int i, byte mode) {
  switch (mode) {
    // режим выбора иконки температуры
    case 1: {
        if (t_prev == i) {
          return 1;
        }
        if (t_prev < i) {
          return 2;
        }
        if (t_prev > i) {
          return 3;
        }
      }
      break;
    // режим выбора иконки влажности
    case 2: {
        if (h_prev == i) {
          return 1;
        }
        if (h_prev < i) {
          return 2;
        }
        if (h_prev > i) {
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
  if (i > -10 && i < 10) {
    lcd.print(0);
  }
}
