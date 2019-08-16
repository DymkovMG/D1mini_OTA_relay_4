#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <PushButtonClicks.h>
#include <ShiftRegister74HC595.h>

//#define DEBUG 
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

#define BUZZER_PIN  D5
#define BUTTON_PIN  D6
#define PHOTOSENSOR_PIN A0    // фоторезистор

PushButton myButton;

boolean relaysONstate = false ; 

// create a global shift register object
// parameters: (number of shift registers, data pin, clock pin, latch pin)
ShiftRegister74HC595 sr (1, 5, 0, 4); 
 


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

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

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


  
  ArduinoOTA.setHostname("WEMOSMINI_RELAY4");
  // No authentication by default
  // ArduinoOTA.setPassword("admin");
  ArduinoOTA.begin();
  
  digitalWrite(LED_BUILTIN, HIGH);//откл светодиод
  beep();
  DEBUG_PRINTLN("Initialization..OK");
}




void beep()
{
  tone(BUZZER_PIN,4000,100);
}

void relayToggle(int rel) 
{
  // read pin (zero based, i.e. 6th pin)
  uint8_t stateOfPin = sr.get(rel-1);
  DEBUG_PRINT("relaystate = ");
  DEBUG_PRINTLNDEC(stateOfPin);
  if (stateOfPin==0) 
  {
    sr.set(rel-1, HIGH);
  }
  else
  {
    sr.set(rel-1, LOW);
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

void loop(void) 
{
  ArduinoOTA.handle();
  switch(myButton.buttonCheck(millis(), digitalRead(BUTTON_PIN))) 
  {
    case 1 : DEBUG_PRINTLN("Pressed and not released for a long time"); break;
    case 2 : DEBUG_PRINTLN("Pressed and released after a long time"); beep();relaysToggle(); break;
    case 3 : DEBUG_PRINTLN("A click"); beep();relayToggle(1);break;
    case 4 : DEBUG_PRINTLN("Double click");beep();relayToggle(2); break;
    case 5 : DEBUG_PRINTLN("Triple click");beep();relayToggle(3); break;
    case 6 : DEBUG_PRINTLN("Four clicks");beep();relayToggle(4); break;
    case 7 : DEBUG_PRINTLN("Five clicks"); break;
  }
  delay(10);
  unsigned int sensorValue = analogRead(PHOTOSENSOR_PIN);
  //DEBUG_PRINTLNDEC(sensorValue);
}
