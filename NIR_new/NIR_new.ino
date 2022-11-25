//дисплей 320х480
//Библиотеки
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
//#include <EEPROM.h>
#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif
//константы для экрана
// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
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
//константы
const word Nism=81;//количество точек измерений
const word HustT=1;//гистерезис уставки
//переменные
word Count=0;//счётчик для сброса автомасштабирования
float Tust=23;//температура уставки
float rT=Tust;//температура с датчика
//float rH=Tust;//влажность с датчика
float sT[Nism]={};//сохранённые значения температуры
//float sH[Nism]={};//сохранённые значения влажности
unsigned long last_time=0;//последнее время
byte TFl=0;//температурный флажок на изменение масшатаба     
TSPoint p;
//начало настройки/////////////////////////////////////////////////////
void setup(void) {
  //инициализация дисплея
  tft.reset();
  uint16_t identifier = 0x9341;
  tft.begin(identifier);
  //настройка пинов
  pinMode(13, OUTPUT);//для экрана
  pinMode(31,OUTPUT);//Мигающий светодиод (3с)
  pinMode(33,OUTPUT);//Всегда горящий светодиод
  pinMode(35,OUTPUT);//Светодиод когда работает вентилятор
  pinMode(37,OUTPUT);//Светодиод когда работает обогрев
  //настройка эерана
  tft.setRotation(1);//положение экрана
  tft.fillScreen(BLACK);//заполняем всё чёрным
  tft.setCursor(0, 0);//установка курсора в начало
  tft.setTextColor(WHITE);  tft.setTextSize(5);//выбираем цвет и размер текста
  tft.println("Get first mesuaring");//выводим на экрна слова
  tft.println("Please wait");
  //Включаем все светодиоды
  digitalWrite(31,HIGH);
  digitalWrite(33,HIGH);
  digitalWrite(35,HIGH);
  digitalWrite(37,HIGH);  
  //Проводим первые измерения, чтоб изначально построить графики
  for(int i=0;i<Nism;i++){
    rT=random(-10,60);
    if(rT>50){//Если значене больше 50 приравниваем к 50
      rT=50;
    }
    if(rT<0){
      TFl=1;
      if(rT<-50){//Если значене меньше -50 приравниваем к -50
      rT=-50;
      }
    }  
    //rH=random(20,60); 
    sT[i]=rT;  
    //sH[i]=rH; 
    }
 //Выключаем вcе светодиоды
  digitalWrite(31,LOW);
  digitalWrite(33,LOW);
  digitalWrite(35,LOW);
  digitalWrite(37,LOW);
  //Выводим на экран неизменные слова
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(RED);  tft.setTextSize(3);
  tft.println("Temp ");
  //tft.setTextColor(BLUE);  tft.setTextSize(3); 
  //tft.println("Hum  ");
  tft.setTextColor(WHITE);  tft.setTextSize(3); 
  tft.println("Tust ");
  tft.setTextColor(RED);  tft.setTextSize(1); 
  tft.setCursor(0, 100);
  tft.println("  50*C");
  //tft.setTextColor(BLUE);  tft.setTextSize(1);
  //tft.println("  100%");
  tft.setCursor(0, 300);  
  tft.setTextColor(RED);  tft.setTextSize(1);
  if(TFl==0){
    tft.println("   0*C");
    } 
  else{
    tft.println(" -50*C");
    }  
  //tft.setTextColor(BLUE);  tft.setTextSize(1);
  //tft.println("  0%");  
  tft.drawRect(39, 99, 403, 203, CYAN);//отрисовка границ графика
  //tft.fillRect(240, 10, 50, 50, RED);//кнопка повышения температуры
  //tft.fillRect(310, 10, 50, 50, BLUE);//кнопка понижения температуры
}
//Начало основного цикла/////////////////////////////////////////////////////
void loop() {
  digitalWrite(13, HIGH);//необходимо для работы экрана
  TSPoint p = ts.getPoint();//получаем значение нажатых пикселей
  digitalWrite(13, LOW);//необходимо для работы экрана
  pinMode(XM, OUTPUT);//необходимо для работы экрана
  pinMode(YP, OUTPUT);//необходимо для работы экрана
  //tft.println(' ',3);
  //tft.println(' ',3);
  //tft.print(p.x,1);
  //tft.print(p.y,1);
  
  //p.x = map(p.x, 940, 120, 320, 0);
  p.y = map(p.y, 0, 920, 480, 0);
  //tft.setCursor(150,200);
  //tft.println((p.y),1);
  //tft.setCursor(150,230);
  //tft.println((p.x),1);
  
  digitalWrite(33,HIGH);//Всегда горящий светодиод
  if (millis()-last_time>1500){
    digitalWrite(31,HIGH);//Мигающий светодиод (3с)
    if (p.y>150 && p.y<400){
    Tust=Tust-0.1;
    delay(100);
    if(Tust<-50)//ели уставка меньше -50 градусов, то приравниваем к -50
    {Tust=-50;}
  }
  if (p.y>0 && p.y<100){
    Tust=Tust+0.1;
    delay(100);
    if(Tust>50)//ели уставка больше 50 градусов, то приравниваем к 50
    {Tust=50;}
  }
  }
  if (millis()-last_time>3000){//Снимаем данные и отрисовываем раз в полторы секунды
    last_time=millis();//обнуляеем счётчик
    digitalWrite(31,LOW);//Мигающий светодиод (3с)
    ReadData(); //процедура для чтения информации с датчика DHT11
    DrawGrapth();//Процедура для отрисовки графиков и измерений            
    } 
  if(rT>=Tust+HustT ){//если считанная температура превышает установленную + гистерезис
      //включаем вентилятор
      digitalWrite(35,HIGH);
      digitalWrite(37,LOW);
    }
  else if(rT<=Tust-HustT){//если считанная температура ниже установленной - гистерезис
      //включаем обогрев
      digitalWrite(37,HIGH);
      digitalWrite(35,LOW);
    }
  else{//если всё хорошо выключаем вентилятор и обогрев
      digitalWrite(37,LOW);
      digitalWrite(35,LOW);
    }
}
//Процедуры://///////////////////////////////////////////////////
void ReadData(){//Процедура считывающая значения с датчика
  //читаем с датчика
  //rH = dht.readHumidity();//Читаем влажность
  //rT = dht.readTemperature();//Читаем температуру
  rT=random(0,60);
  if(rT>50){//если больше 50 то 50
      rT=50;
    }
  if(rT<0){//если меньше 0 поднимаем флаг маштабирования и меняем значение внизу
      TFl=1;
      Count=0;
      tft.fillRect(0, 280, 30, 300, BLACK);//заполняем нижнюю границу, где было 0*С
      tft.setTextColor(RED);  tft.setTextSize(1); 
      tft.setCursor(0, 300); 
      tft.println(" -50*C");
      if(rT<-50){//Если значене меньше -50 приравниваем к -50
        rT=-50;
      }
  }
  //rH=random(20,60);
  //Сдвигаем все значения из легенды влажности и температуры на 1 влево
  for(int i=0;i<(Nism-1);i++){
      sT[i]=sT[i+1];
      //sH[i]=sH[i+1];
    }
  //Меняем последние элементы
  sT[Nism-1]=rT;//В поледний элемент массива записываем новую температуру
  //sH[Nism-1]=rH;//В поледний элемент массива записываем новую влажность
  if(TFl==1 && Count<Nism){//если масштабируем и массив не перезаписался, то увеличиваем счётчие
    Count++;
    }
  else{//иначе убираем автомасштабирование
    TFl=0;
    Count=0;
    tft.setTextColor(RED);  tft.setTextSize(1); 
    tft.fillRect(0, 280, 30, 300, BLACK);//заполняем нижнюю границу, где было -50*С
    tft.setCursor(0, 300); 
    tft.println("   0*C");
    }
}
void DrawGrapth(){//Процедура отрисовки графиков и значений
  tft.fillRect(70, 0, 162, 80, BLACK);//Запалняем область значений чёрным квадратом
  tft.fillRect(40, 100, 401, 201, BLACK);//Запалняем область графиков чёрным квадратом
  DrawIzm();//Процедура отрисовки значений
  if(TFl==0){
  for(int i=0; i<(Nism-1); i++){//строим графики по Nism точкам. имеем по x 400px по y 200px.Относительно курсора (0;0)
    tft.drawLine(40+5*i, -4*sT[i]+300, 5*i+45, -4*sT[i+1]+300, RED);//график температуры
    //tft.drawLine(40+5*i, -sH[i]/2+200, 5*i+45, -sH[i+1]/2+200, BLUE);//график влажности
  }
  tft.drawLine(40, -4*Tust+300, 440, -4*Tust+300, WHITE);//рисуем линию уставки
  }
  else{
    for(int i=0; i<(Nism-1); i++){//строим графики по Nism точкам. имеем по x 400px по y 200px.Относительно курсора (0;0)
    tft.drawLine(40+5*i, int(-2*sT[i]+200), 5*i+45, int(-2*sT[i+1]+200), RED);//график температуры
    //tft.drawLine(40+5*i, -sH[i]/2+200, 5*i+45, -sH[i+1]/2+200, BLUE);//график влажности
  }
  tft.drawLine(40, int(-2*Tust+200), 440, int(-2*Tust+200), WHITE);//рисуем линию уставки
    }
  }
void DrawIzm(){//Процедура отрисовки значений
  tft.setCursor(0, 0);//Установка курора в левый верхний угол
  tft.setTextColor(RED);  tft.setTextSize(3);//Выбор цвета и размера текста
  tft.print("     ");//пробелы так, как слева имеем постоянные слова Temp,Hum,Tust
  tft.print(rT,1);//Выводим последнюю полученную температуру
  tft.println(" *C");//Выводим цельсий
  //tft.setTextColor(BLUE);  tft.setTextSize(3); 
  //tft.print("     ");
  //tft.print(rH,1);//Выводим последнюю полученную влажность
  //tft.println(" %");
  tft.setTextColor(WHITE);  tft.setTextSize(3); 
  tft.print("     ");
  tft.print(Tust,1);//Выводим температуру уставки
  tft.println(" *C");
  }
