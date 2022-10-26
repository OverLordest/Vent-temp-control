
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
//#include <EEPROM.h>
#include "DHT.h"

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
float rT;//температура с датчика
float rH;//влажность с датчика
float Tust=23;//температура уставки
int HustT=2;//гистерезис уставки
int counter=0; //счетчик циклов

void setup(void) {
  //инициализация дисплея
  uint16_t identifier = tft.readID();
  identifier=0x9341;
  tft.begin(identifier);
  //настройка пинов
  pinMode(31,OUTPUT);
  pinMode(33,OUTPUT);
  pinMode(35,OUTPUT);
  pinMode(37,OUTPUT);
  Serial.begin(9600);
  Serial.flush();
  Serial.println("Temp , Hum");
  tft.fillScreen(BLACK);
}

void loop() {
  digitalWrite(33,HIGH);
  if (counter<=15){
    digitalWrite(31,HIGH);
    //Serial.println(counter);
  }
  else{
    digitalWrite(31,LOW);
    //Serial.println(counter);
    }
  if (counter>=30){
    counter=0;
    ReadData(); //процедура для чтения информации с датчика DHT11
    tft.fillScreen(BLACK);
  }
  /*if(Serial.available()>0){
     rT=Serial.parseFloat();
     tft.fillScreen(BLACK);
     Serial.print("Зашли в if");     
     Serial.println(rT);
     Serial.end();
     Serial.begin(9600);
  } */
  //Serial.flush();
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);  tft.setTextSize(4);
  tft.print("Temp ");
  tft.print(rT,1);
  tft.println(" *C"); 
  tft.print("Hum  ");
  tft.print(rH,1);
  tft.println(" %"); 

  if(rT>=Tust+HustT ){
    //включаем вентилятор
    digitalWrite(35,HIGH);
    digitalWrite(37,LOW);
    }
  else if(rT<=Tust-HustT){
    //включаем обогрев
    digitalWrite(37,HIGH);
    digitalWrite(35,LOW);
    }
  else{
    digitalWrite(37,LOW);
    digitalWrite(35,LOW);
    }
    counter++; //увеличиваем значение счетчика циклов программы
  //delay(10); //задержка 10 мсек между циклами программы
}

void ReadData(){
  //читаем с датчика
  //rH = dht.readHumidity();
  //rT = dht.readTemperature();
  rT=random(10,30);
  rH=random(20,60);
  Serial.println(rT);
  Serial.print(',');
  Serial.println(rH);
}
