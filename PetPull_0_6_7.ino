#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define STEP_PIN         9
#define DIR_PIN          10
#define ENABLE_PIN       11

#define B 3950 // B коэффициент
#define RESISTOR 100000 // сопротивление резистора, 200 кОм
#define THERMISTOR 100000 // номинальное сопротивление термистора, 100 кОм
#define NOMINAL 25 // номинальная температура
#define sensor A3

#define PIN_RELAY 8 // Определяем пин, используемый для подключения реле

const int switchPin = 7;  // кнопка джойстика
const int pinX = A1; // Ось X джойстика
const int pinY = A2; // Ось Y джойстика

LiquidCrystal_I2C lcd(0x27,16,2);  // Устанавливаем дисплей

unsigned long timing_sensor;
unsigned long timing_hot;
//int screen_key = 1;

void setup() {
  Serial.begin(9600);

  pinMode(STEP_PIN   , OUTPUT);       //Настраиваем пины управления
  pinMode(DIR_PIN    , OUTPUT);
  pinMode(ENABLE_PIN , OUTPUT);
  digitalWrite(ENABLE_PIN , HIGH);    //Выключаем мотор, чтобы не грелся

  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);
  pinMode(switchPin, INPUT);
  digitalWrite(switchPin, HIGH); // включаем встроенный подтягивающий резистор

  pinMode(sensor, INPUT); // датчик температуры

  pinMode(PIN_RELAY, OUTPUT); // Объявляем пин реле как выход
  digitalWrite(PIN_RELAY, HIGH); // Выключаем реле - посылаем высокий сигнал

  lcd.init();                     
  lcd.backlight();// Включаем подсветку дисплея
  lcd.cursor();
  //lcd.blink();
}

class OSCore {
  public:
  int cursorPosition_x = 1;
  int cursorPosition_y = 0;
  bool joystick_button = false;
  int ScreenSize_x = 16;
  int ScreenSize_y = 2;
  int pX;
  int pY;
  int xVal = 0;
  int yVal = 0;
  int screen_key = 1;
  int screen_number = 3;
  float temperature_sensor;
  int temp = 0;
  int speed = 0;
  int arr_screen[2][16] =  
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

  void controlStick () {
    int buttonState = !digitalRead(switchPin);
    int x = analogRead(pinX); // считываем аналоговое значение оси Х
    int y = analogRead(pinY); // считываем аналоговое значение оси Y
    xVal = map(x, 0, 1023, 0, 3);
    yVal = map(y, 0, 1023, 0, 3);

    if (buttonState == HIGH) {           // функция нажатия на кнопку
        joystick_button = true;
      }
      else {
        joystick_button = false;
      }
  }
  void cursorControl () {
      if (xVal == 2 && yVal == 1) { // функция движения вправо
        if (cursorPosition_x != ScreenSize_x-1){
          for (int i = cursorPosition_x + 1; i < ScreenSize_x; i++) {
            if (arr_screen[cursorPosition_y][i] != 0) {
               cursorPosition_x = i;
               lcd.setCursor(cursorPosition_x, cursorPosition_y);
               break;
            }
          }
        }
        delay(200);
      }
      else if (xVal == 0 && yVal == 1) { // функция движения влево
        if (cursorPosition_x != 0){
          for (int i = cursorPosition_x - 1; i >= 0; i--) {
            if (arr_screen[cursorPosition_y][i] != 0) {
               cursorPosition_x = i;
               lcd.setCursor(cursorPosition_x, cursorPosition_y);
               break;
            }
          }
        }
        delay(200);
      }
      else if (xVal == 1 && yVal == 0) { // функция движения вверх
         if (cursorPosition_y != 0){
          for (int i = cursorPosition_y - 1; i >= 0; i--) {
            if (arr_screen[i][cursorPosition_x] != 0) {
               cursorPosition_y = i;
               lcd.setCursor(cursorPosition_x, cursorPosition_y);
               break;
            }
          }
        }
        delay(200);
      }
      else if (xVal == 1 && yVal == 2) { // функция движения вниз
        if (cursorPosition_y != ScreenSize_y-1){
          for (int i = cursorPosition_y + 1; i < ScreenSize_y; i++) {
            if (arr_screen[i][cursorPosition_x] != 0) {
               cursorPosition_y = i;
               lcd.setCursor(cursorPosition_x, cursorPosition_y);
               break;
            }
          }
        }
        delay(200);
      }
  }

  void Button (int x, int y, char ch, int id) {
    arr_screen[y][x] = id;
    lcd.setCursor(x, y);
    lcd.print(ch);
  }

  int events () {
    bool returnValue = true;
    Serial.println('1');
    if (joystick_button) {
      switch(arr_screen[cursorPosition_y][cursorPosition_x]){
        case 1:
          if (screen_key != 1)
            screen_key--;
          returnValue = false;
          break;
        case 2:
          if (screen_key != screen_number)
            screen_key++;
          returnValue = false;
          break;
        case 3:
          if (temp > 0)
            temp -= 1;
          returnValue = true;
          break;
        case 4:
          if (temp < 250)
            temp += 1;
          returnValue = true;
          break;
        case 5:
        if (temp > 4)
            temp -= 5;
          returnValue = true;
          break;
        case 6:
          if (temp < 250-4)
            temp += 5;
          returnValue = true;
          break;
        case 7:
        if (speed > 0)
            speed -= 1;
          returnValue = true;
          break;
        case 8:
          if (speed < 100)
            speed += 1;
          returnValue = true;
          break;
        case 9:
        if (speed > 4)
            speed -= 5;
          returnValue = true;
          break;
        case 10:
          if (speed < 96)
            speed += 5;
          returnValue = true;
          break;
      }
      delay(200);
      return returnValue;
    }
  }

  void clear () {
    lcd.clear();
    for (int i = 0; i < 2; i++)
      for (int j = 0; j < 16; j++)
        arr_screen[i][j] = 0;
  }

  void drawCounter (int x, int y, int number) {
    if (number < 10) {
      lcd.setCursor(x, y);
      lcd.print("  ");
      lcd.setCursor(x+2, y);
      lcd.print(number);
    }
    else if (number < 100) {
      lcd.setCursor(x, y);
      lcd.print(" ");
      lcd.setCursor(x+1, y);
      lcd.print(number);
    }
    else if (number > 99) {
      lcd.setCursor(x, y);
      lcd.print(number);
    }
    lcd.setCursor(cursorPosition_x, cursorPosition_y);
  }

  void engineOperationMode () {
    if (speed > 0) {
      digitalWrite(ENABLE_PIN , LOW);       //Включаем мотор
      digitalWrite(DIR_PIN    , LOW);      //Задаем направление вращения
    }
    else {
      digitalWrite(ENABLE_PIN , HIGH);        //Выключаем движок, чтобы не грелся
    }
  }
  void engineManagement () {
      digitalWrite(STEP_PIN    , HIGH);     //Крутим движок
      delayMicroseconds(2300 - 23 * speed); //Крутим движок
      digitalWrite(STEP_PIN    , LOW);      //Крутим движок
  }
  void temperatureSensor () {
    int t = analogRead(sensor);
    float tr = 1023.0 / t - 1;
    tr = RESISTOR / tr;

    temperature_sensor = tr / THERMISTOR;
    temperature_sensor = log(temperature_sensor);
    temperature_sensor /= B;
    temperature_sensor += 1.0 / (NOMINAL + 273.15);
    temperature_sensor = 1.0 / temperature_sensor;
    temperature_sensor -= 273.15;
  }

  void relayControl () {
    if (temperature_sensor < temp - 3)
      digitalWrite(PIN_RELAY, LOW); // Включаем реле - посылаем низкий уровень сигнала
    else if (temperature_sensor >= temp - 2)
      digitalWrite(PIN_RELAY, HIGH); // Отключаем реле - посылаем высокий уровень сигнала
  }
};
class Draw {
  public:
  OSCore OSC;

  void setChar (int x, int y, char ch) { 
    lcd.setCursor(x, y);
    lcd.print(ch);
  } 
};

class BasicScreen {
  public:
    byte checking_the_cycle_X;
    byte checking_the_cycle_Y;

    OSCore OSC;
    Draw Draw;

    void basicScreen () {
      OSC.controlStick();
      OSC.cursorControl();
    }
    void drawingButtons () {
      OSC.Button(0, 0, '<', 1);
      OSC.Button(15, 0, '>', 2);
    }
};

class Screen : public BasicScreen {
  public:

  void main () {
    int checking_temp = OSC.temperature_sensor;
    drawingButtons();
    Draw.setChar(5, 0, 't');
    Draw.setChar(8, 1, '%');
    Draw.setChar(7, 0, '>');
    OSC.drawCounter(9, 0, OSC.temp);
    Draw.setChar(12, 0, 't');
    lcd.setCursor(OSC.cursorPosition_x, OSC.cursorPosition_y);
    do {
      OSC.engineOperationMode();
      OSC.drawCounter(5, 1, OSC.speed);
      checking_the_cycle_X = OSC.cursorPosition_x;
      checking_the_cycle_Y = OSC.cursorPosition_y;
      checking_temp = OSC.temperature_sensor;

      do {
        OSC.engineManagement();
        OSC.temperatureSensor();
        if (millis() - timing_hot > 500) {
          OSC.relayControl();
          timing_hot = millis();
        }
        if (millis() - timing_sensor > 1000) {
          OSC.drawCounter(2, 0, OSC.temperature_sensor);
          timing_sensor = millis();
        }
        basicScreen();
      } while (checking_the_cycle_X == OSC.cursorPosition_x && checking_the_cycle_Y == OSC.cursorPosition_y && !OSC.joystick_button);

    } while (OSC.events());
  }

  void setting () {
    drawingButtons();
    Draw.setChar(0, 1, '-');
    Draw.setChar(15, 1, '+');
    OSC.Button(2, 0, '1', 3);
    OSC.Button(13, 0, '1', 4);
    OSC.Button(4, 0, '5', 5);
    OSC.Button(11, 0, '5', 6);
    OSC.Button(2, 1, '1', 7);
    OSC.Button(13, 1, '1', 8);
    OSC.Button(4, 1, '5', 9);
    OSC.Button(11, 1, '5', 10);
    Draw.setChar(9, 0, 't');
    Draw.setChar(9, 1, '%');
    
    lcd.setCursor(OSC.cursorPosition_x, OSC.cursorPosition_y);
    do {
      OSC.engineOperationMode();
      checking_the_cycle_X = OSC.cursorPosition_x;
      checking_the_cycle_Y = OSC.cursorPosition_y;
      OSC.drawCounter(6, 0, OSC.temp);
      OSC.drawCounter(6, 1, OSC.speed);

      do {
        OSC.engineManagement();
        OSC.temperatureSensor();
        if (millis() - timing_hot > 500) {
          OSC.relayControl();
          timing_hot = millis();
        }
        basicScreen();
      } while (checking_the_cycle_X == OSC.cursorPosition_x && checking_the_cycle_Y == OSC.cursorPosition_y && !OSC.joystick_button);
      Serial.println('2');
    } while (OSC.events());
  }

  void description () {
    drawingButtons();
    lcd.setCursor(1, 0);
    lcd.print("Version: 0.6.9");
    lcd.setCursor(1, 1);
    lcd.print("Author: GGammi");
    lcd.setCursor(OSC.cursorPosition_x, OSC.cursorPosition_y);
    do {
      OSC.engineOperationMode();
      checking_the_cycle_X = OSC.cursorPosition_x;
      checking_the_cycle_Y = OSC.cursorPosition_y;

      do {
        OSC.engineManagement();
        OSC.temperatureSensor();
        if (millis() - timing_hot > 500) {
          OSC.relayControl();
          timing_hot = millis();
        }
        basicScreen();
      } while (checking_the_cycle_X == OSC.cursorPosition_x && checking_the_cycle_Y == OSC.cursorPosition_y && !OSC.joystick_button);

    } while (OSC.events());
  }
  
  void listOfScreens () {
    do {
      OSC.clear();
      switch (OSC.screen_key){
        case 1:
          main();
          break;
        case 2:
          setting();
          break;
        case 3:
          description();
          break;
      }
    } while (true);
  }
  
};

void loop() {
  Screen Screen;
  Screen.listOfScreens();
}