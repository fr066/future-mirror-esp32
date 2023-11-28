#include <Arduino.h>
/*
   Данный код плавно управляет одной сервой   
   при помощи потенциометра (на пине А0).
   Используется драйвер PCA9685
   Откройте порт по последовательному соединению для наблюдения за положением серво
   Документация: https://alexgyver.ru/servosmooth/
*/
/*
  Данный скетч крутит 4 сервопривода с разными скоростями и ускорениями
  Документация: https://alexgyver.ru/servosmooth/
 */
#define AMOUNT 3  // кол-во серво
#include <ServoSmooth.h>
#include <ServoDriverSmooth.h>

ServoDriverSmooth servos[AMOUNT];
uint32_t servoTimer;
uint32_t turnTimer;
String cmd;
int positionm1 = 90;
int positionm2 = 90;
int positionm3 = 90;
int cAngle = 1;
int position1 = 60;   // первое положение серв
int position2 = 120;  // второе положение серв
int startPos = 10;
int endPos = 170;
int offsetPos = 5;
int turning = 1;
boolean manual = false;
boolean wavemode = false;
boolean reverse = false;
boolean autoturn = false;
boolean flag;
void setup() {
  Serial.begin(9600);
  // подключаем , ставим на 90 градусов
  servos[0].attach(0,90);
  servos[1].attach(1,90);
  servos[2].attach(2,90);
  // настраиваем макс. скорость
  servos[0].setSpeed(90);
  servos[1].setSpeed(90);
  servos[2].setSpeed(90);
  Serial.println("setup complete");

}

void upAngle(int angle) {
  if ( angle > 0) {
positionm1 = 90 + angle ;
  } else {
    positionm1 = 90 - angle ;
  }
    
    //positionm2 = 90 - angle;
    //positionm3 = 90 - angle;
    
}



void rAngle(int angle) {
 
    positionm2 = 90 - angle;
    positionm3 = 90 + angle;
}
void loop() {

  if (Serial.available() > 0) {  //если есть доступные данные
        // считываем байт
     cmd = Serial.readString();
  
  if ( cmd == "wave" ) {
    autoturn = false;
    wavemode = true;
    Serial.println("Wave mode started " );
  }
  if ( cmd == "stop" ) {
    autoturn = false;
    wavemode = false;
    Serial.println("Automoving disabled " );
  }
  if ( cmd == "mid" ) {
    autoturn = false;
    Serial.println(" set middle position " );
    positionm1 = positionm2 = positionm3 = 90;
    manual = true;
  }


   if ( cmd.startsWith("up") ){
    autoturn = false;
    Serial.println(" set up angle " );
      String np  = cmd.substring(3,6);
    Serial.println(cmd + " " + np);
     int newpos = np.toInt(); 
     if (newpos == 0) { 
      newpos = 90;
     }
    upAngle(newpos);
     
    manual = true;
  }

  if ( cmd.startsWith("tor") ){
    autoturn = false;
    Serial.println(" set right angle " );
      String np  = cmd.substring(4,7);
    Serial.println(cmd + " " + np);
     int newpos = np.toInt(); 
     if (newpos == 0) { 
      newpos = 90;
     }
    
    positionm1 = 90;
    positionm2 = 90 - newpos;
    positionm3 = 90 + newpos;
     
    manual = true;
  }
  
  if ( cmd == "hi" ) {
    autoturn = false;
    Serial.println(" set high position " );
    positionm1 = positionm2 = positionm3 = 20;
    manual = true;
  }

  if ( cmd == "low" ) {
    autoturn = false;
    Serial.println(" set low position " );
    positionm1 = positionm2 = positionm3 = 150;
    manual = true;
  }
  if ( cmd == "auto") {
    autoturn = true;
    Serial.println("Automoving started... " );
  } 
  if ( cmd.startsWith("turn") ) {

    manual = true;

  }
  if ( cmd.startsWith("go") ) {
    
    String np  = cmd.substring(3,6);
    //Serial.println(np);
     int newpos = np.toInt(); 
     if (newpos == 0) { 
      newpos = 90;
     }
     positionm1 = newpos;
     
     np  = cmd.substring(7,10);
    //Serial.println(np);
     newpos = np.toInt(); 
     if (newpos == 0) { 
      newpos = 90;
     }
     positionm2 = newpos;
     
     np  = cmd.substring(11,14);
    //Serial.println(np);
    newpos = np.toInt(); 
     if (newpos == 0) { 
      newpos = 90;
     }
     positionm3 = newpos;

    Serial.println("go... 1:" + String(positionm1) + " 2:" + String(positionm2) + " 3:" + String(positionm3)  ); 
  }

  
  }
  

  // каждые 20 мс
  if (millis() - servoTimer >= 20) {  // взводим таймер на 20 мс (как в библиотеке)
    servoTimer += 20;
    for (byte i = 0; i < AMOUNT; i++) {
      servos[i].tickManual();   // двигаем все сервы. Такой вариант эффективнее отдельных тиков
    }
    //Serial.println("timer tick ... ");
  }


  
  if (wavemode )  {
      
 if (millis() - turnTimer >= 1000) {
    
    
    if ( cAngle >= 26 ) {
      reverse = true;
    } 
    if ( cAngle <= -26 ) {
      reverse = false;
    }
   if (reverse ) {
    cAngle -= 3;
   } else {
    cAngle += 3;
   }
   rAngle(cAngle);
   upAngle(cAngle);
   manual = true;
   turnTimer = millis();
    
  }
  }

  if (autoturn) {  

  // каждые 2 секунды меняем положение
  if (turning == 7) {
    turning = 1;
    position2 -= offsetPos;
    position1 += offsetPos;
  }
  if (millis() - turnTimer >= 1000) {
    Serial.println("Turn - " + String(turning) + " Pos1:" + String(position1) + " Pos2:" + String(position2) );
    turnTimer = millis();
    flag = !flag;
    
    if (turning == 1) {
     servos[0].setTargetDeg(position1);
     servos[1].setTargetDeg(position2);
     servos[2].setTargetDeg(position1);
    }
    if (turning == 2) {
     servos[0].setTargetDeg(position2);
     servos[1].setTargetDeg(position1);
     servos[2].setTargetDeg(position2);
    }
    if (turning == 3) {
     servos[0].setTargetDeg(position1);
     servos[1].setTargetDeg(position1);
     servos[2].setTargetDeg(position2);
    }
    if (turning == 4) {
     servos[0].setTargetDeg(position2);
     servos[1].setTargetDeg(position2);
     servos[2].setTargetDeg(position1);
    }
    if (turning == 5) {
     servos[0].setTargetDeg(position1);
     servos[1].setTargetDeg(position2);
     servos[2].setTargetDeg(position2);
    }
    if (turning == 6) {
     servos[0].setTargetDeg(position2);
     servos[1].setTargetDeg(position1);
     servos[2].setTargetDeg(position1);
    }
    turning += 1;
  }
  } else {
    if ( manual ) {
    
    
    servos[0].setTargetDeg(positionm1);
    servos[1].setTargetDeg(positionm2);
    servos[2].setTargetDeg(positionm3);
    Serial.println("set position Pos1:" + String(positionm1) + " Pos2:" + String(positionm2) + " Pos3:" + String(positionm3) );
    manual = false;
    }
    //sleep(2);
  }
}