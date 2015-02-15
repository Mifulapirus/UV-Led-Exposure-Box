#include <SoftwareSerial.h>
#include <ESP8266.h>
#include <serLCD.h>

/*******************************
* Pin definitions
*******************************/
#define DISPLAY_PIN   A1

#define PAD_UP_PIN    9
#define PAD_DOWN_PIN  10
#define PAD_LEFT_PIN  11
#define PAD_RIGHT_PIN 12

#define UV_SWITCH_PIN A5

#define WIFI_RX_PIN 2
#define WIFI_TX_PIN 3
#define WIFI_RST_PIN 4

/*******************************
* General variables
*******************************/
#define FIRMWARE_VERSION   "0.2"
#define DEVICE_CATEGORY    "UV Exposure Box" 
#define DEVICE_NAME        "UV Exposure Box"
long timerStartedTime = 0;
long timerLastUpdate   = 0;
long timerValue       = 10000;         //10 seconds
boolean timerStarted = false;

/*****************************
* Other devices
*****************************/
serLCD lcd(DISPLAY_PIN);

/*******************************
* WiFi Stuff
*******************************/
#define BAUD 9600
#define SSID "Interne"
#define PASS "Mec0mebien"
#define SERVER_PORT  "5555"
ESP8266 wifi(WIFI_RX_PIN, WIFI_TX_PIN, WIFI_RST_PIN, BAUD);

/****************************
* Control Commands
****************************/
#define CMD_UV_ON    "UV:On"
#define CMD_UV_OFF   "UV:Off"
#define CMD_UV_TEST  "UV:Test"
#define CMD_TIMER_ON  "Timer:On"
#define CMD_TIMER_ON  "Timer:Off"
#define CMD_SET_TIMER  "SetTimer"
#define CMD_START_TIMER  "StartTimer"
#define CMD_STOP_TIMER  "StopTimer"

/****************************
* Configuration Commands
****************************/
#define CMD_WIFI_SSID  "SetSSID"
#define CMD_WIFI_PASS  "SetPass"



/************************************
* Setup and initializations
************************************/  
void setup() {
  //Set input pins
  pinMode(PAD_UP_PIN, INPUT);
  pinMode(PAD_DOWN_PIN, INPUT);
  pinMode(PAD_LEFT_PIN, INPUT);
  pinMode(PAD_RIGHT_PIN, INPUT);
  //Set output pins
  pinMode(WIFI_RST_PIN, OUTPUT);
  pinMode(UV_SWITCH_PIN, OUTPUT);
  
  //Initialize navite serial port
  Serial.begin(9600);
  lcd.clear();
  delay(1000);
  bootUp(); 
}


/************************************
* printing and reporting
************************************/
void lcdAndSerialPrint(String _text) {
  lcd.print(_text);
  Serial.println(_text);
}


/************************************
* General Functions
************************************/
void loop() {
  receiveCommand();
  checkUVTimer();
}

//Bootup sequence
void bootUp() {
  int _err = -1;
  int Err = -1;
  lcd.clear();
  lcdAndSerialPrint(DEVICE_NAME);
  lcd.selectLine(2);
  lcdAndSerialPrint("Version: ");
  lcdAndSerialPrint(FIRMWARE_VERSION);
  
  delay(1000);
  lcd.clear();
  lcdAndSerialPrint("UV Test -> ");
  UVSwithTest();
  lcdAndSerialPrint("Done");
  delay(1000);
  
  lcd.clear();
  lcdAndSerialPrint("Initializing");
  lcd.selectLine(2);
  lcdAndSerialPrint("WiFi ESP8266");
  _err = wifi.init(SSID, PASS);
  if(_err != NO_ERROR) {
    lcdAndSerialPrint("Error: ");
    lcdAndSerialPrint(String(_err));
    delay(3000);
  }
  
  else{
    lcd.clear();
    lcdAndSerialPrint("Connected: ");
    lcd.selectLine(2);
    lcdAndSerialPrint(wifi.IP);
    delay(2000);  
  }
     
  _err = wifi.setServer(SERVER_PORT);
  if(_err != NO_ERROR) {
    lcd.clear();
    lcdAndSerialPrint("Error: ");
    lcdAndSerialPrint(String(_err));
  }
  
  else{
    lcd.clear();
    lcdAndSerialPrint("Server OK: ");
    lcdAndSerialPrint(wifi.serverPort);
    
    delay(1000);
    lcd.clear();
    lcdAndSerialPrint(DEVICE_NAME);
    lcd.selectLine(2);
    lcdAndSerialPrint("     ONLINE");  
  }    
}


/**************************************************************************************
* Command functions
***************************************************************************************/
int receiveCommand() {
  String _cmd = wifi.readCmd();
  if (_cmd == "") return false;
  if (_cmd == CMD_UV_ON)                  return setUV(true);
  else if (_cmd == CMD_UV_OFF)            return setUV(false);
  else if (_cmd == CMD_UV_TEST)           return UVSwithTest();
  else if (contains(_cmd, CMD_SET_TIMER)) return setTimer(extractParam(_cmd).toInt());
  else if (_cmd == CMD_START_TIMER)       return startTimer();
  else if (_cmd == CMD_STOP_TIMER)        return stopTimer();
  else return false;
}

boolean contains(String _original, String _search) {
    int _searchLength = _search.length();
	int _max = _original.length() - _searchLength;
    for (int i = 0; i <= _max; i++) {
        if (_original.substring(i, i+_searchLength) == _search) {return true;}
    }
    return false;
} 

//Extracts the value of a parameter from the '=' separator till the end
String extractParam(String _cmd) {
  int _separatorIndex = _cmd.indexOf('=') + 1;
  return _cmd.substring(_separatorIndex);
}

  
/**************************************************************************************
* UV-LED Functions
***************************************************************************************/
boolean UVSwithTest() {
  for (int i=0; i<4; i++) {
   digitalWrite(UV_SWITCH_PIN, !digitalRead(UV_SWITCH_PIN));
   delay(200);
  }
  return true;
}

boolean setUV(boolean _state) {
   digitalWrite(UV_SWITCH_PIN, _state);
   getUV();
   if (digitalRead(UV_SWITCH_PIN)== _state) return true;
   else return false;
}

boolean getUV() {
  boolean _state = digitalRead(UV_SWITCH_PIN);
  lcd.setCursor(2,16);
  lcd.print(_state);
  return _state;
}

void toggleUV() {
  digitalWrite(UV_SWITCH_PIN, !digitalRead(UV_SWITCH_PIN));
}

void checkUVTimer() {
  if (timerStarted == true) {
    long _now = millis();
        
    if (_now > (timerStartedTime + timerValue - 1000)) {
      setUV(false);
      timerStarted = false;
      lcd.clear();
      lcdAndSerialPrint("Exposure DONE");
    }
    
    else if (_now >= timerLastUpdate + 1000) {  //Every second print remaining time
      timerLastUpdate = _now;
      lcd.setCursor(2,1);
      lcd.print("               ");
      lcd.setCursor(2,1);
      lcdAndSerialPrint(String((timerStartedTime + timerValue - _now)/1000));
    }  
  }
}

boolean setTimer(int _time) {
  timerValue = _time;
  timerValue = timerValue * 1000;  //time converted to ms
  lcd.setCursor(2,1);
  lcd.print("               ");
  lcd.setCursor(2,1);
  lcdAndSerialPrint(String(timerValue/1000));
  return true;
}
  
boolean startTimer() {
    timerStarted = true;
    timerStartedTime=millis();
    setUV(true);
    return true;
}

boolean stopTimer() {
    timerStarted = false;
    setUV(false);
    return true;
}


