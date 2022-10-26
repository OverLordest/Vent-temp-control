#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <EEPROM.h>

#define DHTPIN 7 // номер пина, к которому будет подсоединен датчик (DHT11 например)
LiquidCrystal_I2C lcd(0x3f,16,2);   // Задаем адрес и размерность дисплея. (LCD 1602 I2C например)
DHT dht(DHTPIN, DHT11); //инциализация датчика температуры и влажности

//служебные переменные
int counter=0; //счетчик циклов
String strOut=""; //строка для вывода на дисплей
float rh; //для считывания влажности
float rt; //для считывания температуры
float h=0; //для хранения усредненного значения влажности
float t=0; //для хранения усредненного значения температуры
String out_h="--.--"; //строка для вывода влажности
String out_t="--.--"; //строка для вывода темературы

//количество необходимых измерений для вычисления среднего значения
#define measureCntMax 5

//количество произведенных измерений
int measureCnt=0;

String tiktak="";//для отображения процесса работы в виде мигающей справа на экране точки, чтобы понимать что устройство не зависло и производит измерения


//температура включения и выключения отопления по умолчанию
float tempHeatOn=14.00;
float tempHeatOff=20.00;
//состояние обогревателя
byte HeatState=0; //0 - обогрев выключен, 1 - обогрев включен

//влажность включения и выключения вытяжки по умолчанию
float humVentOn=55.00;
float humVentOff=45.00;
byte VentActive=1;

//состояние вентиляции
byte VentState=0; //0 - вытяжка выключена, 1 - вытяжка включена


int mode=0; //режим экрана и ввода с кнопок
//0 - отображает значения температуры и влажности
//1 - экран настройки температуры включения обогрева
//2 - экран настройки температуры отключения обогрева
//3 - экран настройки влажности включения вентиляции
//4 - экран настройки влажности отключения вентиляции
//5 - экран настройки включения вентилятора
//7 - отображение текущих параметров влажности
//8 - отображение текущих температурных параметров
//6 - служебное значение, по которому сбрасываем режим на 0 и сохраняем настройки


//предыдущее значение кнопок
byte Btn1LastState=-1; //для 1 кнопки
byte Btn2LastState=-1; //для 2 кнопки

//время удержания кнопок
int Hold1Time=0; //для 1 кнопки
int Hold2Time=0; //для 2 кнопки


void setup()
{
  //РЕЛЕ:
  pinMode(4,OUTPUT); //реле 2, разъём 2 - сюда подключен нагреватель
  pinMode(9,OUTPUT); //реле 1, разъем 1 - сюда подключен вентилятор вытяжки
  //ЭКРАН:
  pinMode(14, OUTPUT); //переключаем аналоговый вход A0 в режим цифрового вывода с номером D14
  digitalWrite(14, LOW);
  pinMode(15, OUTPUT); //переключаем аналоговый вход A1 в режим цифрового вывода - D15 
  //КНОПКИ:
  pinMode(5,INPUT); //настраиваем цифровой вход 5 на ввод
  pinMode(8,INPUT); //настраиваем цифровой вход 8 на ввод
  digitalWrite(5, HIGH); //подтягиваем резистор на вход 5
  digitalWrite(8, HIGH); //подтягиваем резистор на вход 8
  
  lcd.init(); // Инициализация lcd             
  lcd.backlight();// Включаем подсветку
  delay(100);
  //выводим приветственный текст на дисплей
  lcd.setCursor(0, 0); // Устанавливаем курсор в начало 1 строки
  lcd.print("<<< Start >>>"); // Выводим текст
  delay(100);

  //запускаем датчик DHT11
  dht.begin();

  //объявляем 4 переменные для чтения настроек из памяти EEPROM
  float e1;//температура вклчючения
  float e2;//температура выключения
  float e3;//влажность включения
  float e4;//влажность выключения
  byte v1;
  
  //читаем настройки из памяти
  EEPROM.get(0, e1);
  EEPROM.get(20, e2);
  EEPROM.get(40, e3);
  EEPROM.get(60, e4);
  EEPROM.get(200, v1);  
  if (v1>1){v1=0;} 
  
  //Поочередно покажем считанные настройки (либо их отсутствие)
  lcd.setCursor(0, 0);
  lcd.print(MakeString("Read Temp On"));
  lcd.setCursor(0, 1);
  if (isnan(e1)){
    lcd.print(MakeString("EEPROM EMPTY"));
  }
  else{
    lcd.print(MakeString(String(e1)));
    tempHeatOn=e1;
  }
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print(MakeString("Read Temp Off"));
  lcd.setCursor(0, 1);
  if (isnan(e2)){
    lcd.print(MakeString("EEPROM EMPTY"));
  }
  else{
    lcd.print(MakeString(String(e2)));
    tempHeatOff=e2;
  }
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print(MakeString("Read Hum On"));
  lcd.setCursor(0, 1);
  if (isnan(e3)){
    lcd.print(MakeString("EEPROM EMPTY"));
  }
  else{
    lcd.print(MakeString(String(e3)));
    humVentOn=e3;
  }
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print(MakeString("Read Hum Off"));
  lcd.setCursor(0, 1);
  if (isnan(e4)){
    lcd.print(MakeString("EEPROM EMPTY"));
  }
  else{
    lcd.print(MakeString(String(e4)));
    humVentOff=e4;
  }
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print(MakeString("Vent 1 Active"));
  lcd.setCursor(0, 1);
  if (isnan(v1)){
    lcd.print(MakeString("EEPROM EMPTY"));
  }
  else{
    lcd.print(MakeString(String(v1)));
    VentActive=v1;
  }
  delay(1000);

  

  //отобразим служебное сообщение, пока производится первое измерения данных до вывода на экран
  lcd.setCursor(0, 0);
  lcd.print(MakeString("Starting Sensor"));
  lcd.setCursor(0, 1);
  lcd.print(MakeString("Please wait"));
}


void loop()
{
  //измеряем температуру и влажность каждые 2 секунды
  if (counter>=200 && mode==0){
    counter=0;
    ReadData(); //процедура для чтения информации с датчика DHT11
  }

  //читаем кнопки
  byte Btn2=digitalRead(8);
  byte Btn1=digitalRead(5);

  //1.если удерживаюися обе кнопки
  if (Btn2==0 && Btn1==0 && Btn1LastState==0 && Btn2LastState==0){
    counter=0;
    Hold1Time++;
    Hold2Time++;
    //если удерживаются обе кнопки более 2 секунд то переходим на следующий экран
    if (Hold1Time>=200 && Hold2Time>=200){
      Hold1Time=0;
      Hold2Time=0;
      if (mode>6){mode=0;}
      mode++;
      //сохраним настройки
      if (mode==6){
        mode=0;
        EEPROM.put(0, tempHeatOn);
        EEPROM.put(20, tempHeatOff);
        EEPROM.put(40, humVentOn);
        EEPROM.put(60, humVentOff);
        EEPROM.put(200, VentActive);      
        //выведем сообщение об успешном сохранении настроек
        lcd.setCursor(0, 0);
        lcd.print(MakeString("Settings"));
        lcd.setCursor(0, 1);
        lcd.print(MakeString("Saved"));
        delay(2000);
      }
      DrawLCD();
    }
  }
  //2.короткое нажатие 1 кнопки
  else if (Btn1==0 && Btn1LastState==1 && Btn2==1){
    counter=0;
    switch (mode){
      case 0:
        mode=7;
        DrawLCD();
        break;
      case 1:
        tempHeatOn+=0.1;
        DrawLCD();  
        break;
      case 2:
        tempHeatOff+=0.1;
        DrawLCD();  
        break;
      case 3:
        humVentOn++;
        DrawLCD();  
        break;
      case 4:
        humVentOff++;
        DrawLCD();  
        break;
      case 5:
        VentActive=VentActive+1;
        if (VentActive>1){VentActive=0;}
        DrawLCD();
        break;         
      case 7:
        mode=0;
        DrawLCD();
        break;
      case 8:
        mode=7;
        DrawLCD();
        break;
    }
  }
  //3.короткое нажатие 2 кнопки
  else if (Btn2==0 && Btn2LastState==1 && Btn1==1){
    counter=0;
    switch (mode){
      case 0:
        mode=8;
        DrawLCD();
        break;
      case 1:
        tempHeatOn-=0.1;
        DrawLCD();  
        break;
      case 2:
        tempHeatOff-=0.1;
        DrawLCD();  
        break;
      case 3:
        humVentOn--;
        DrawLCD();  
        break;
      case 4:
        humVentOff--;
        DrawLCD();  
        break;
      case 5:
        VentActive=VentActive+1;
        if (VentActive>1){VentActive=0;}
        DrawLCD();
        break;              
      case 7:        
        mode=8;
        DrawLCD();
        break;
      case 8:
        mode=0;
        DrawLCD();
        break;

    }
  }
  //4.длинное нажатие 2 кнопки - отключаем подсветку дисплея
  else if (Btn2==0 && Btn1==1 && Btn2LastState==0){
    counter=0;
    Hold2Time++;
    if (Hold2Time>=300){
      Hold2Time=0;
      lcd.noBacklight();
    }
  }
  //5.длинное нажатие 1 кнопки - включаем подсветку дисплея
  else if (Btn2==1 && Btn1==0 && Btn1LastState==0){
    counter=0;
    Hold1Time++;
    if (Hold1Time>=300){
      Hold2Time=0;
      lcd.backlight();
    }
  }
  //когда ничего не нажато
  else{
    Hold1Time=0;
    Hold2Time=0;
    //если на экране настройки/информации ничего долго (в течение 10 сек) не нажимается то возвращаемся в режим отображения температуры и влажности
    if (mode!=0 && counter>1000){
      //если возврат производится с какого-либо экрана настройки, то сообщим о том, что настройки не были сохранены
      if (mode<6){
        lcd.setCursor(0, 0);
        lcd.print(MakeString("Settings"));
        lcd.setCursor(0, 1);
        lcd.print(MakeString("NOT Saved"));
        delay(2000);
      }
      counter=0;
      mode=0;
      DrawLCD;
    }
  }

  //запоминаем предыдущее значение кнопок
  Btn1LastState=Btn1;
  Btn2LastState=Btn2;
  
  counter++; //увеличиваем значение счетчика циклов программы
  delay(10); //задержка 10 мсек между циклами программы
}



void ReadData(){
  //читаем с датчика
  rh = dht.readHumidity();
  rt = dht.readTemperature();
  
  //проверяем считанное значение, если пустое то выдаем ошибку, если не пустое то производим вычисления
  if (isnan(rh) || isnan(rt)) {
    //если не удалось считать данные
    lcd.setCursor(0, 0);
    strOut=MakeString("Read Error");
    lcd.print(strOut);  
    lcd.setCursor(0, 1);            
    strOut=MakeString("Check Sensor");
    lcd.print(strOut);  
  }
  else{
    measureCnt++; //увеличили количество проведенных измерений
    h=h+rh; //суммируем показания влажности
    t=t+rt; //суммируем показания температуры   
    //если набрали нужное количество измерений
    if (measureCnt==measureCntMax){
      //вычисляем средние значения
      h=h/measureCntMax;
      t=t/measureCntMax;
      //запомним последние значения температуры и влажности для вывода на экран
      out_h=String(h);
      out_t=String(t);
      //включаем/отключаем нагрузку
      if (t!=0 && h!=0){
        DoAll(); //процедура, котоаря управляем нагрузкой в соответствии с правилами 
      }
      //сбрасываем счетчик количества измерений
      measureCnt=0;
      h=0;
      t=0;
    }
    DrawLCD();
  }
}


//процедура отрисовки экрана LCD
void DrawLCD(){
  if (mode==0){
    lcd.setCursor(0, 0);
    strOut=MakeString("Temp: "+out_t+"*C");
    lcd.print(strOut);  
    
    lcd.setCursor(0, 1);            
    strOut=MakeString("Hum: "+out_h+"%");
    lcd.print(strOut); 
    
    //рисуем и убираем точку в правом нижнем углу для индикации процесса измерения
    if (tiktak==""){
    tiktak=".";
    }
    else{
      tiktak="";
    }
    lcd.setCursor(15, 1);            
    lcd.print(tiktak); 
  }
  if (mode==1){
    lcd.setCursor(0, 0);
    strOut=MakeString("Temp On: "+String(tempHeatOn)+"*C");
    lcd.print(strOut);  
    
    lcd.setCursor(0, 1);            
    strOut=MakeString("");
    lcd.print(strOut);  
  }
  if (mode==2){
    lcd.setCursor(0, 0);
    strOut=MakeString("Temp Off: "+String(tempHeatOff)+"*C");
    lcd.print(strOut);  
    
    lcd.setCursor(0, 1);            
    strOut=MakeString("");
    lcd.print(strOut);  
  }
  if (mode==3){
    lcd.setCursor(0, 0);
    strOut=MakeString("Hum On: "+String(humVentOn)+"%");
    lcd.print(strOut);  
    
    lcd.setCursor(0, 1);            
    strOut=MakeString("");
    lcd.print(strOut);  
  }
  if (mode==4){
    lcd.setCursor(0, 0);
    strOut=MakeString("Hum Off: "+String(humVentOff)+"%");
    lcd.print(strOut);  
    
    lcd.setCursor(0, 1);            
    strOut=MakeString("");
    lcd.print(strOut);  
  }
  if (mode==5){
    lcd.setCursor(0, 0);
    strOut=MakeString("Vent 1 Active: "+String(VentActive));
    lcd.print(strOut);  
    
    lcd.setCursor(0, 1);            
    strOut=MakeString("Vent 1 Active: "+String(VentActive));
    lcd.print(strOut);  
  }  
  if (mode==7){
    lcd.setCursor(0, 0);
    strOut=MakeString("Temp On: "+String(tempHeatOn)+"*C");
    lcd.print(strOut);  
    
    lcd.setCursor(0, 1);            
    strOut=MakeString("Temp Off: "+String(tempHeatOff)+"*C");
    lcd.print(strOut);  
  }  
  if (mode==8){
    lcd.setCursor(0, 0);
    strOut=MakeString("Hum On: "+String(humVentOn)+"%");
    lcd.print(strOut);  
    
    lcd.setCursor(0, 1);            
    strOut=MakeString("Hum Off: "+String(humVentOff)+"%");
    lcd.print(strOut);  
  }
}


//правила поведения устройства в зависимости от температуры и влажности
void DoAll(){
  //включаем обогрев
  //сделаем чтобы батарея также включалась при повышенной влажности, чтобы подсушивать воздух
  if (HeatState==0 && (t<=tempHeatOn || h>humVentOn)){
    digitalWrite(4,HIGH);
    digitalWrite(14,HIGH);
    HeatState=1;
    delay(200);
  }
  //отключаем обогрев, когда температура пришла в норму, а также влажность не превышает верхний предел (порог включения вытяжки)
  if (t>=tempHeatOff && HeatState==1 && h<=humVentOn){
    digitalWrite(4,LOW);
    digitalWrite(14,LOW);
    HeatState=0;
    delay(200);
  }

  //включаем вентиляцию, замыкаем разъемы 1 
  //также включаем вытяжку, если температура превышает верхнее значение 
  //(т>температуры отключения обогрева, например если перегрели во время просушки, 
  //то вытяжка продолжит работать когда влажность уже в норме, но температура воздуха еще высокая)
  if (VentState==0 && (h>=humVentOn || t>tempHeatOff)){
    if (VentActive==1){digitalWrite(9,1);}
    //if (Vent2Active==1){digitalWrite(2,1);}
    if (VentActive==1){
      digitalWrite(15,HIGH);
      VentState=1;
    }
    
    delay(200);
  }
  //отключаем вентиляцию, когда влажность и температура пришли в норму
  if (h<=humVentOff && VentState==1 && t<=tempHeatOff ){
    digitalWrite(9,0);
    //digitalWrite(2,0);
    digitalWrite(15,LOW);
    VentState=0;
    delay(200);
  }

}

//функция для формирования строки длинною 16 символов для вывода на экран, позволяет не вызывать функцию очистки экрана, ускоряет процесс работы скетча
//недостающие символы заполняются пробелами до 16 знака
String MakeString(String str){
  String res;
  res=str;
  while(res.length()<16){
    res=res+" ";
  }
  return res;
}
