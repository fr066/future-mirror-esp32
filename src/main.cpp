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
#include <WiFi.h>



// Replace with your network credentials
const char* ssid     = "AlexP-LTD";
const char* password = "423011sn";


// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Decode HTTP GET value
String valueString = String(5);
int pos1 = 0;
int pos2 = 0;



// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

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
  servos[0].setSpeed(60);
  servos[1].setSpeed(60);
  servos[2].setSpeed(60);
  // ускорение
  servos[0].setAccel(0.5);
  servos[1].setAccel(0.5);
  servos[2].setAccel(0.5);
  servos[0].smoothStart();  
  servos[1].smoothStart();  
  servos[2].smoothStart();  
  Serial.println("setup complete");

Serial.print("Connecting to network ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void upAngle(int angle) {
  if ( angle > 0 ) {
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
 WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial; margin-left:auto; margin-right:auto;}");
            client.println(".slider { width: 300px; }</style>");
            client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>");
                     
            // Web Page
            client.println("</head><body><h1>ESP32 with Servo</h1>");
            client.println("<p>Position Up motor: <span id=\"servoPos\"></span></p>");          
            client.println("<input type=\"range\" min=\"0\" max=\"180\" class=\"slider\" id=\"servoSlider\" onchange=\"servo(this.value)\" value=\""+valueString+"\"/>");
            client.println("<p>Position Left motor: <span id=\"servoPos\"></span></p>");          
            client.println("<input type=\"range\" min=\"0\" max=\"180\" class=\"sliderl\" id=\"servoSliderl\" onchange=\"servo(this.value)\" value=\""+valueString+"\"/>");
            client.println("<p>Position Right motor: <span id=\"servoPos\"></span></p>");          
            client.println("<input type=\"range\" min=\"0\" max=\"180\" class=\"sliderr\" id=\"servoSliderr\" onchange=\"servo(this.value)\" value=\""+valueString+"\"/>");
            
            client.println("<script>var slider = document.getElementById(\"servoSlider\");");
            client.println("var servoP = document.getElementById(\"servoPos\"); servoP.innerHTML = slider.value;");
            client.println("slider.oninput = function() { slider.value = this.value; servoP.innerHTML = this.value; }");
            client.println("$.ajaxSetup({timeout:1000}); function servo(pos) { ");
            client.println("$.get(\"/?value=\" + pos + \"&\"); {Connection: close};}</script>");
           
            client.println("</body></html>");     
            
            //GET /?value=180& HTTP/1.1
            if(header.indexOf("GET /?value=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              
              //Rotate the servo
              //myservo.write(valueString.toInt());
              positionm1 = valueString.toInt();
              manual =true;
              Serial.println(valueString); 
            }         
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }


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