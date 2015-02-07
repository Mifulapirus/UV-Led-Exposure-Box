#include <SoftwareSerial.h>
#include <ESP8266.h>
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
ESP8266 wifi(wifiRx_pin, wifiTx_pin, wifiRST_pin);

serLCD lcd(display_pin);


void setup() {
  pinMode(padUp_pin, OUTPUT);
  pinMode(padDown_pin, OUTPUT);
  pinMode(padLeft_pin, OUTPUT);
  pinMode(padRight_pin, OUTPUT);
  pinMode(wifiRST_pin, OUTPUT);
  pinMode(UVSwitch_pin, OUTPUT);
  
  Serial.begin(9600);
    
  Serial.println(DeviceName);
  Serial.println(DeviceCategory);
  Serial.print("V. ");
  Serial.println(FirmwareVersion);
  Serial.print("ID: ");
  Serial.println(DeviceID);
  
  lcd.clear();
  lcd.print(DeviceCategory);
  lcd.print(" ");
  lcd.print(FirmwareVersion);
  
  UVSwithTest();
  
  int Err = -1;
  String Ans="";
  
  Serial.print("Reboot --> ");
  Err = wifi.WiFiReboot();
  if (Err == NO_ERROR) {Serial.println("Reboot OK");}
  else {Serial.println(Err);}

  Serial.print("WiFi Mode --> ");
  Err = wifi.WiFiMode(1);
  if (Err == NO_ERROR) {Serial.println("WiFi Mode OK");}
  else {Serial.println(Err);}
  
  Serial.print("Connect --> ");
  Err = wifi.ConnectWiFi("Interne", "Mec0mebien");
  if (Err == NO_ERROR) {Serial.println("Connection OK");}
  else {Serial.println(Err);}
  
  Serial.print("Get IP --> ");
  String IP = wifi.GetIP();
  Serial.println(IP);
  
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

