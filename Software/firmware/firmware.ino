#include <SoftwareSerial.h>
#include <serLCD.h>

#define display_pin A0

#define padUp_pin 9
#define padDown_pin 10
#define padLeft_pin 11
#define padRight_pin 12

#define UVSwitch_pin A5


/*******************************
* General variables
*******************************/
#define FirmwareVersion "0.1"
#define DeviceCategory "UV Exposure Box" 
int DeviceID = 1; 
#define DeviceName "UV"

/*******************************
* WiFi Stuff
*******************************/
#define wifiRx_pin 2
#define wifiTx_pin 3
#define wifiRST_pin 4

#define SSID "Interne"
#define PASS "Mec0mebien"
SoftwareSerial WiFiSerial(wifiRx_pin, wifiTx_pin); // RX, TX
String OwnIP = "000.000.000.000";
String WiFiLongMessage;

//----------------------
//Messages sent
#define ResponseOK     "OK"
#define Error          "ERROR "
  #define ErrorUnableToLink             "1"
  #define SendLongMessageError          "2"  
  #define ErrorModuleDoesntRespond      "3"
  #define ErrorModuleDoesntRespondToAT  "4"
  #define ErrorResponseNotFound         "5"
  #define ErrorUnableToConnect          "6"
  
#define CIPSTART       "AT+CIPSTART=\"TCP\",\""


serLCD lcd(display_pin);


void setup() {
  pinMode(padUp_pin, OUTPUT);
  pinMode(padDown_pin, OUTPUT);
  pinMode(padLeft_pin, OUTPUT);
  pinMode(padRight_pin, OUTPUT);
  pinMode(wifiRST_pin, OUTPUT);
  pinMode(UVSwitch_pin, OUTPUT);
  
  Serial.begin(9600);
  WiFiSerial.begin(9600);
  
  Serial.println(DeviceName);
  Serial.println(DeviceCategory);
  Serial.print("V. ");
  Serial.println(FirmwareVersion);
  Serial.print("ID: ");
  Serial.println(DeviceID);
  //Attach the interruption
//  attachInterrupt(0, FireDetected, RISING);
  WiFiLongMessage.reserve(400);
  lcd.clear();
  lcd.print(DeviceCategory);
  lcd.print(" ");
  lcd.print(FirmwareVersion);
  UVSwithTest();
  
  InitWiFi();
  lcd.print(" WiFi: Online");
}

/************************************
* General Functions
************************************/
void loop() {

}

void PrintError(char* ErrorMessage) {
  Serial.print(Error);
  Serial.println(ErrorMessage);
  }


/**************************************************************************************
* UV-LED Functions
***************************************************************************************/
void UVSwithTest() {
  for (int i=0; i<6; i++) {
   digitalWrite(UVSwitch_pin, !digitalRead(UVSwitch_pin));
   delay(500);
   }
}

/**************************************************************************************
* WiFi Functions
***************************************************************************************/
boolean OpenTCP(String IP, String Port) {
  String cmd = CIPSTART; 
  cmd += IP;
  cmd += "\",";
  cmd += Port;
  SendDebug(cmd);
  if (!ExpectResponse("Linked")) {
    PrintError(ErrorUnableToLink);
    return false;
  }
  return true;
}

boolean SendLongMessage(char* ExpectedReply) {
  Serial.println("Sending: ");
  Serial.println(WiFiLongMessage);
  WiFiSerial.print("AT+CIPSEND=");
  WiFiSerial.println(WiFiLongMessage.length());
  delay(200); 
  WiFiSerial.print(WiFiLongMessage);
  if (ExpectResponse(ExpectedReply)) {
    Serial.println(ResponseOK);
    return true;
    }
  else {
    PrintError(SendLongMessageError);
    return false;
    }
}

void WiFiRead() {
    while (WiFiSerial.available()) {
      Serial.write(WiFiSerial.read());
      delay(1);
    }
}  

void WiFiEcho() {
  while(1) {
    if (WiFiSerial.available())
      Serial.write(WiFiSerial.read());
    if (Serial.available())
      WiFiSerial.write(Serial.read());
  }
}

void SendDebug(String cmd) {
  WiFiSerial.flush();
  Serial.print("Sent: ");
  Serial.println(cmd);
  WiFiSerial.println(cmd);
}

boolean CloseTCP() {
  SendDebug("AT+CIPCLOSE");
  if (!ExpectResponse("Unlink")) {CheckWiFi();}
}


boolean InitWiFi() {
  while (!WiFiReboot())  {}
  //while (!CheckWiFi())   {}
  while (!ConnectWiFi()) {}
  SetCIPMODE(false);
  return true;
}

boolean WiFiReboot() {
  digitalWrite(wifiRST_pin, LOW);
  delay(500);
  digitalWrite(wifiRST_pin, HIGH);
  if (ExpectResponse("Ready")) {
    return true;  
  }
  else {
    PrintError(ErrorModuleDoesntRespond);
    return false;
  }
}

boolean WiFiReset() {
  WiFiSerial.flush();
  Serial.println("Reboot");
  WiFiSerial.println("AT+RST"); // restet and test if module is redy
  if (ExpectResponse("Ready")) {
    return true;  
  }
  else {
    PrintError(ErrorModuleDoesntRespond);
    return false;
  }
}

boolean CheckWiFi() {
  SendDebug("AT");
  if (ExpectResponse(ResponseOK)) {
    Serial.println(ResponseOK);
    return true;
  }
  else {
    PrintError(ErrorModuleDoesntRespondToAT);
    InitWiFi();
    return false;
  }
}

boolean ExpectResponse(char* _Expected) {
  Serial.print("Waiting for ");
  Serial.print(_Expected);
  Serial.print(": ");

  for (int i = 0; i < 10; i++) {
    if (WiFiSerial.find(_Expected)) {
      Serial.println(ResponseOK);
      return true;
    }

    else {
      //TODO: Do something if not found yet?
      Serial.print(".");
    }
    delay(1);
  }
  PrintError(ErrorResponseNotFound);
  return false;
}

String GetIP() {
  WiFiSerial.flush();
  WiFiSerial.println("AT+CIFSR"); // Get IP
  OwnIP = "";
  char incomingByte = ' ';
  delay(100);
  while (WiFiSerial.available() > 0) {
    // read the incoming byte:
    incomingByte = WiFiSerial.read();
    // say what you got:
    OwnIP += incomingByte;
    delay(1);
  }
  return OwnIP;
}

boolean ConnectWiFi() {
  WiFiSerial.flush();
  Serial.println("Connecting");
  WiFiSerial.println("AT+CWMODE=1");
  delay(2000);
  String cmd = "AT+CWJAP=\"";
  cmd += SSID;
  cmd += "\",\"";
  cmd += PASS;
  cmd += "\"";
  SendDebug(cmd);
  if (ExpectResponse(ResponseOK)) {
    Serial.print("IP: ");
    Serial.println(GetIP());
  }
  else {
    PrintError(ErrorUnableToConnect);
  }
}

boolean SetCIPMODE(boolean Value) {
  if (Value) {WiFiSerial.println("AT+CIPMODE=1");}
  else {WiFiSerial.println("AT+CIPMODE=0");}
}

int CheckBaudrate() {
  //300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 31250, 38400, 57600, and 115200. 
  int Pause=500;
  long Baudrate[13] = {300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 31250, 38400, 57600, 115200};
  
  for (int i=0; i<13; i++) {
    WiFiSerial.begin(Baudrate[i]);
    Serial.print("\n\r ");
    Serial.print(Baudrate[i]);
    Serial.print(": ");
    WiFiReboot();
    WiFiRead();
  }
}

