#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>  // обновление по воздуху
#include <PushButtonClicks.h> // обработка нажатия кнопки
#include <ShiftRegister74HC595.h> // сдвиговый регистр 
#include <ESP8266WebServer.h>

//#define DEBUG    //расскоментировать для отладочных сообщений
#ifdef DEBUG
  #define DEBUG_PRINT(x) Serial.print  (x)
  #define DEBUG_PRINTDEC(x) Serial.print  (x,DEC)
  #define DEBUG_PRINTLNDEC(x) Serial.println  (x,DEC)
  #define DEBUG_PRINTLN(x) Serial.println  (x)
#else  
  #define DEBUG_PRINT(x) 
  #define DEBUG_PRINTDEC(x) 
  #define DEBUG_PRINTLNDEC(x) 
  #define DEBUG_PRINTLN(x) 
#endif

#ifndef STASSID
#define STASSID "FALK"
#define STAPSK  "3199431994"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);
const char* www_username = "admin";
const char* www_password = "esp8266";

#define BUZZER_PIN  D5  //пищалка для оповещения
#define LED_PIN  D7  //светодиод для оповещения
#define BUTTON_PIN  D6 //кнопка для ручного управления релюхами
#define PHOTOSENSOR_PIN A0    // фоторезистор для отключения релюх днём если забыли

PushButton myButton;

//boolean relaysONstate = false ; 

unsigned int illuminationSensorValue = 0;

// create a global shift register object
// parameters: (number of shift registers, data pin, clock pin, latch pin)
ShiftRegister74HC595 sr (1, 5, 0, 4); 

//void handleRoot();              // function prototypes for HTTP handlers
//void handleNotFound();


void setup(void) 
{
  #ifdef DEBUG
    Serial.begin(115200);// initialize serial communication 
  #endif  
  DEBUG_PRINTLN("Start initialization.");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);//вкл светодиод

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); 

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); 

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(200);
    DEBUG_PRINT("*");
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINT("Connected to ");
  DEBUG_PRINTLN(ssid);
  DEBUG_PRINT("IP address: ");
  DEBUG_PRINTLN(WiFi.localIP());


  
  ArduinoOTA.setHostname("WEMOSMINI_RELAY4");    // имя в сети для обновления по OTA
  // No authentication by default
  // ArduinoOTA.setPassword("admin");
  ArduinoOTA.begin();
  
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.on("/rel1", handleRel1);
  server.on("/rel2", handleRel2);
  server.on("/rel3", handleRel3);
  server.on("/rel4", handleRel4);
  
  server.begin();
  
  digitalWrite(LED_BUILTIN, HIGH);//откл светодиод
  beep();
  DEBUG_PRINTLN("Initialization..OK");
}


void led_on()
{
    digitalWrite(LED_PIN, HIGH); 
}

void led_off()
{
    digitalWrite(LED_PIN, LOW); 
}


void beep()
{
  tone(BUZZER_PIN,3000,100);
}

uint8_t checkRelayStatusOn()
{
  for (int i=0; i <= 7; i++)
  {
      if (sr.get(i)==1) 
      { return 1;}//return True if any relay is on
   }

  return 0; //return false if all relay is off
}

uint8_t relayToggle(int rel) 
{
  // read pin (zero based, i.e. 6th pin)
  uint8_t stateOfPin = sr.get(rel-1);
  DEBUG_PRINT("relaystate = ");
  DEBUG_PRINTLNDEC(stateOfPin);
  if (stateOfPin==0) 
  {
    sr.set(rel-1, HIGH);
    led_on();
    return 1;
  }
  else
  {
    sr.set(rel-1, LOW);
    return 0;
  }
  
}

void relaysToggle() 
{
  if (relaysONstate==true) 
  {
    sr.setAllLow();
    //relaysONstate=false;
  }
  else
  {
    sr.setAllHigh();
    //relaysONstate=true;
  }
  relaysONstate=!relaysONstate;
}

void handleNotFound()
{
  //if(!server.authenticate(www_username, www_password)) return server.requestAuthentication();
  
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  //server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request

}

void handleRoot() 
{
  //if(!server.authenticate(www_username, www_password)) return server.requestAuthentication();
  
  String message = "hello from Wemos mini!\n\n";
  uint8_t valret =  sr.get(0);
    if (valret == HIGH)
    {message += "RELAY 1 ON\n";}
    else
    {message += "RELAY 1 OFF\n";};
    
    valret =  sr.get(1);
    if (valret == HIGH)
    {message += "RELAY 2 ON\n";}
    else
    {message += "RELAY 2 OFF\n";};
    
    valret =  sr.get(2);
    if (valret == HIGH)
    {message += "RELAY 3 ON\n";}
    else
    {message += "RELAY 3 OFF\n";};

    valret =  sr.get(3);
    if (valret == HIGH)
    {message += "RELAY 4 ON\n";}
    else
    {message += "RELAY 4 OFF\n";};

    message += "\nIllumination sensor value = "+String(illuminationSensorValue);
  
  
  server.send(200, "text/plain", message);
}

void handleRel1()
{
  if(!server.authenticate(www_username, www_password)) return server.requestAuthentication();
  uint8_t stateOfRelay =  relayToggle(1);
  beep();
  if (stateOfRelay==0) 
  {
    server.send(200, "text/plain", "OFF");
  }
  else
  {
    server.send(200, "text/plain", "ON");
  }
}

void handleRel2()
{
  if(!server.authenticate(www_username, www_password)) return server.requestAuthentication();
  uint8_t stateOfRelay =  relayToggle(2);
  beep();
  if (stateOfRelay==0) 
  {
    server.send(200, "text/plain", "OFF");
  }
  else
  {
    server.send(200, "text/plain", "ON");
  }
}

void handleRel3()
{
  if(!server.authenticate(www_username, www_password)) return server.requestAuthentication();
  uint8_t stateOfRelay =  relayToggle(3);
  beep();
  if (stateOfRelay==0) 
  {
    server.send(200, "text/plain", "OFF");
  }
  else
  {
    server.send(200, "text/plain", "ON");
  }
}

void handleRel4()
{
  if(!server.authenticate(www_username, www_password)) return server.requestAuthentication();
  uint8_t stateOfRelay =  relayToggle(4);
  beep();
  if (stateOfRelay==0) 
  {
    server.send(200, "text/plain", "OFF");
  }
  else
  {
    server.send(200, "text/plain", "ON");
  }
}



void loop(void) 
{
  ArduinoOTA.handle();
  server.handleClient();
  switch(myButton.buttonCheck(millis(), digitalRead(BUTTON_PIN))) 
  {
    case 1 : DEBUG_PRINTLN("Pressed and not released for a long time"); break;
    case 2 : DEBUG_PRINTLN("Pressed and released after a long time (take all relays to Off state )"); beep(); sr.setAllLow();led_off(); break;
    case 3 : DEBUG_PRINTLN("A click"); beep();relayToggle(1);break;
    case 4 : DEBUG_PRINTLN("Double click");beep();relayToggle(2); break;
    case 5 : DEBUG_PRINTLN("Triple click");beep();relayToggle(3); break;
    case 6 : DEBUG_PRINTLN("Four clicks");beep();relayToggle(4); break;
    case 7 : DEBUG_PRINTLN("Five clicks"); break;
  }
  //delay(10);
  
  if (checkRelayStatusOn())
    {led_on();}
    else
    {led_off();};
  
  illuminationSensorValue = analogRead(PHOTOSENSOR_PIN);
  //DEBUG_PRINTLNDEC(sensorValue);
}
