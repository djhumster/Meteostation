/*
    Written by Pavel Vishnyakov
    Метеостанция
    Требуются библиотеки:
    http://arduino-info.wikispaces.com/LCD-Blue-I2C
    https://learn.adafruit.com/dht требует доп. библиотеку, ВНИМАТЕЛЬНО читать ман
  !!!    https://github.com/adafruit/RTClib
    https://github.com/mathertel/OneButton
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <DHT.h>
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

OneButton menu_btn(MENU_BTN_PIN, true);  // кнопка меню, параметр true для INPUT_PULLUP
OneButton up_btn(UP_BTN_PIN, true);  //  кнопка "+" увеличения значений

const byte charBitmap[][8] {
  //  знак температуры
  {B00100, B01010, B01010, B01110, B01110, B11111, B11111, B01110},
  //  знак влажности
  {B00100, B00100, B01010, B01010, B10001, B10001, B10001, B01110},
  // знак барометр
  {B01110, B01010, B01010, B01010, B01010, B10001, B11111, B01110},
  // знак Цельсия
  {B00011, B00011, B00000, B00000, B00000, B00000, B00000, B00000},
  // тире
  {B00000, B00000, B00000, B11111, B11111, B00000, B00000, B00000},
  // стрелки вверх
  {B00000, B00100, B01110, B11111, B00000, B00100, B01110, B11111},
  // стрелки вниз
  {B00000, B11111, B01110, B00100, B00000, B11111, B01110, B00100},
  //  призрак
  {B00000, B01110, B11111, B10101, B11111, B11111, B10101, B00000}
};

boolean light = 1;  //  состояние подсветки экрана, в перспективе и другой подсветки

byte h_prev;  //  предыдущие значения температуры и влажности
int t_prev;

//  переменные для меню и ручного выбора даты/времени
byte menu_mode = 0;
tmElements_t tmE; //  адский формат даты/времени

//  для таймеров
unsigned long ltm1, ltm2;

void setup() {
  Serial.begin(9600);
  Serial.println("LCD start...");

  lcd.begin(20, 4);  // инициальзация экрана и включение подсветки
  lcd.backlight();
  lcd.clear();
  //  записываем символы в память экрана
  int charBitmapSize = (sizeof(charBitmap ) / sizeof (charBitmap[0]));
  for ( int i = 0; i < charBitmapSize; i++ )
  {
    lcd.createChar ( i, (byte *)charBitmap[i] );
  }

  Serial.println("DHT start...");
  dht.begin(); //  инициальзация датчика влажности

  setSyncProvider(RTC.get); //  синхронизация времени с RTC

  switch (timeStatus()) {
    case timeSet: Serial.println("RTC has set the system time");
      break;
    case timeNeedsSync: Serial.println("Time set, but unable to sync with the RTC");
      break;
    case timeNotSet:
      Serial.println("Time didn't set yet");
      Serial.println("Setting default time");
      setTime(1, 0, 0, 1, 1, 2017);
      break;
  }
  /*
     подключаем функции кнопок
     Кнопка МЕНЮ:
      клик (в меню) - меняет страницы меню
      двойной клик - включает/выключает меню с сохранением изменений
      длинный клик (в меню) - выход из меню, без сохранения изменений
     Кнопка "+":
      клик (в меню) - +1 к значение
      двойной клик (вне меню) - вкл/выкл подсветку экрана
      двойной клик (в меню) - +5 к значению, -1 для года
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

  if (menu_mode == 0 && ltm1 - millis() >= 1000) {
    ltm1 = millis();
    my_clock();
  }
  if (menu_mode == 0 && ltm2 - millis() >= 60000) {
    ltm2 = millis();
    weather();
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

  lcd.home();
  lcd.write((byte)0);
  lcd.print(" ");
  two_digits(t);
  lcd.write((byte)3);
  lcd.print("C ");
  lcd.write(icon(t, 1));
  lcd.setCursor(0, 1);
  lcd.write((byte)1);
  lcd.print(" ");
  two_digits(h);
  lcd.print(" % ");
  lcd.write(icon(h, 2));
  lcd.print(" ");
  lcd.write((byte)2);
  lcd.print(" --- mm");

  h_prev = h;
  t_prev = t;
}
// --- ЧАСЫ ---
void my_clock() {
  time_t tm = now();

  lcd.setCursor(0, 3);
  two_digits(day(tm));
  lcd.print(" ");
  lcd.print(dayShortStr(weekday()));
  lcd.print(" ");
  two_digits(hour(tm));
  //  мигаем разделителем часов и минут
  if (second(tm) & 1) {
    lcd.print(':');
  } else {
    lcd.print(' ');
  }
  two_digits(minute(tm));
  lcd.print(" ");
  two_digits(month(tm));
  lcd.print('/');
  lcd.print(year(tm));
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
    time_t tm = now();
    breakTime(tm, tmE);
    menu();
  } else {
    tmE.Second = 0;
    RTC.write(tmE);
    setTime(tmE.Hour, tmE.Minute, tmE.Second, tmE.Day, tmE.Month, tmYearToCalendar(tmE.Year));
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
    weather();
  }
}
//  кнопка "+" увеличивает звначения в зависимости от выбранного пункта меню
void up_click() {
  if (menu_mode > 0) {
    switch (menu_mode) {
      case 1:
        if (tmE.Hour < 23 ) {
          tmE.Hour++;
        } else {
          tmE.Hour = 0;
        }
        break;
      case 2:
        if (tmE.Minute < 59 ) {
          tmE.Minute++;
        } else {
          tmE.Minute = 0;
        }
        break;
      case 3:
        if (tmE.Day < 31) {
          tmE.Day++;
        } else {
          tmE.Day = 1;
        }
        break;
      case 4:
        if (tmE.Month < 12) {
          tmE.Month++;
        } else {
          tmE.Month = 1;
        }
        break;
      case 5: tmE.Year++;
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
      two_digits(tmE.Hour);
      break;
    case 2:
      lcd.print("Minute: ");
      two_digits(tmE.Minute);
      break;
    case 3:
      lcd.print("Day: ");
      two_digits(tmE.Day);
      break;
    case 4:
      lcd.print("Month: ");
      two_digits(tmE.Month);
      break;
    case 5:
      lcd.print("Year: ");
      lcd.print(tmYearToCalendar(tmE.Year));
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
  switch (menu_mode) {
    case 0:
      if (light) {
        lcd.noBacklight();
        light = 0;
      } else {
        lcd.backlight();
        light = 1;
      }
      return;
    case 1:
      if (tmE.Hour < 23 ) {
        tmE.Hour += 5;
      } else {
        tmE.Hour = 0;
      }
      break;
    case 2:
      if (tmE.Minute < 59 ) {
        tmE.Minute += 5;
      } else {
        tmE.Minute = 0;
      }
      break;
    case 3:
      if (tmE.Day < 31) {
        tmE.Day += 5;
      } else {
        tmE.Day = 1;
      }
      break;
    case 4:
      if (tmE.Month < 12) {
        tmE.Month += 5;
      } else {
        tmE.Month = 1;
      }
      break;
    case 5: tmE.Year--;
      break;
  }
  menu();
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
          return 4;
        }
        if (t_prev < i) {
          return 5;
        }
        if (t_prev > i) {
          return 6;
        }
      }
      break;
    // режим выбора иконки влажности
    case 2: {
        if (h_prev == i) {
          return 4;
        }
        if (h_prev < i) {
          return 5;
        }
        if (h_prev > i) {
          return 6;
        }
      }
      break;
  }
}
/*
  функция добавления 0 перед однозначным числом
*/
void two_digits(int i) {
  if (i > -10 && i < 10) {
    lcd.print(0);
  }
  lcd.print(i);
}
