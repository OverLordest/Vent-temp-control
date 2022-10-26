//дисплей 320х480
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
float Tust=23;//температура уставки
float rT=Tust;//температура с датчика
float rH=Tust;//влажность с датчика
float sT[11]={};//сохранённые значения температуры
//int counterT=0;
int HustT=2;//гистерезис уставки
int counter=0; //счетчик циклов     
/////////////////////////////////////////////////////////////////////////////////////
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
  //Serial.begin(9600);
  //Serial.flush();
  //Serial.println("Temp , Hum");
  
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);  tft.setTextSize(5);
  tft.println("Get first mesuaring");
  tft.println("Please wait");
  
  digitalWrite(31,HIGH);
  digitalWrite(33,HIGH);
  digitalWrite(35,HIGH);
  digitalWrite(37,HIGH);  
  
  for(int i=0;i<11;i++){
    rT=random(10,30);
    rH=random(20,60); 
    sT[i]=rT;  
    }
    
  digitalWrite(31,LOW);
  digitalWrite(33,LOW);
  digitalWrite(35,LOW);
  digitalWrite(37,LOW);
  tft.fillScreen(BLACK);
}
/////////////////////////////////////////////////////////////////////////////////////
void loop() {
  digitalWrite(33,HIGH);
  if (counter<=1){
    digitalWrite(31,HIGH);
    //Serial.println(counter);
  }
  else{
    digitalWrite(31,LOW);
    //Serial.println(counter);
    }
  if (counter>=2){
    counter=0;
    ReadData(); //процедура для чтения информации с датчика DHT11
    tft.fillScreen(BLACK);
    delay(1000);
  }
  /*if(Serial.available()>0){
     rT=Serial.parseFloat();
     tft.fillScreen(BLACK);
     Serial.print("Зашли в if");     
     Serial.println(rT);
     Serial.end();
     Serial.begin(9600);
  } */
  tft.setCursor(0, 0);
  tft.setTextColor(RED);  tft.setTextSize(3);
  tft.print("Temp ");
  tft.print(rT,1);
  tft.println(" *C");
  tft.setTextColor(BLUE);  tft.setTextSize(3); 
  tft.print("Hum  ");
  tft.print(rH,1);
  tft.println(" %");
  tft.setTextColor(WHITE);  tft.setTextSize(3); 
  tft.print("Tust ");
  tft.print(Tust,1);
  tft.println(" *C");
  DrawGrapth();

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
}

void ReadData(){
  //читаем с датчика
  //rH = dht.readHumidity();
  //rT = dht.readTemperature();
  rT=random(10,30);
  rH=random(20,60);
  
  for(int i=0;i<10;i++){
      sT[i]=sT[i+1];
    }
  sT[10]=rT;
  
  //Serial.println(rT);
  //Serial.print(',');
  //Serial.println(rH);
}

void DrawGrapth(){
  tft.setTextColor(RED);  tft.setTextSize(1); 
  tft.setCursor(0, 100);
  tft.println("  50*C");
  tft.setTextColor(BLUE);  tft.setTextSize(1);
  tft.println("  100%");
  tft.setCursor(0, 280);
  tft.setTextColor(RED);  tft.setTextSize(1); 
  tft.println(" -50*C");
  tft.setTextColor(BLUE);  tft.setTextSize(1);
  tft.println("  0%");
  
  /*tft.drawLine(40, 100, 40, 300, CYAN);
  tft.drawLine(40, 300, 440, 300, CYAN);
  tft.drawLine(40, 100, 440, 100, CYAN);
  tft.drawLine(440, 100, 440, 300, CYAN);*/
  tft.drawRect(40, 100, 401, 200, CYAN);

  for(int i=0; i<10; i++){
    tft.drawLine(40+40*i, -2*sT[i]+200, 40*i+80, -2*sT[i+1]+200, RED);
  }
  tft.drawLine(40, -2*Tust+200, 440, -2*Tust+200, WHITE);
  }
