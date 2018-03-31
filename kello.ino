#include <ESP8266HTTPClient.h>

#include <ESP8266WiFi.h>
#include <TM1637Display.h>
#include <EasyNTPClient.h>
#include <WiFiUdp.h>
 
const int CLK = D1; //Set the CLK pin connection to the display
const int DIO = D3; //Set the DIO pin connection to the display
 
int numCounter = 0;

TM1637Display display(CLK, DIO); //set up the 4-Digit Display.

WiFiUDP udp;

EasyNTPClient ntpClient(udp, "pool.ntp.org"); // IST = GMT + 5:30

const char *ssid = "";
const char *password = "";
// place where to sync summer time. 
// page should print int
const char *remote_clock_sync = "";

int timevalue = 0;
int paivat_pois;

int sekun = 0;
int minuu = 0;
int tunti = 2;

const int sync_hour = 18;
const int sync_min = 50;
const int sync_sec = 00;

const int turnOffHour = 20;
const int turnOffMin = 30;

const int turnOnHour = 6;
const int turnOnMin = 0;

void wifiConnect(){
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    sekun++;
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  udp.begin(123);
  return;
}

void wifiDisconnect(){
  WiFi.disconnect();
}

void syncTime(bool disp){
  int x = 1;
  wifiConnect();
  timevalue = ntpClient.getUnixTime();
  int tryTimes = 10;
  while(timevalue == 0 && tryTimes > 0){
    delay(2000);
    sekun = sekun + 2;
    tryTimes--;
    timevalue = ntpClient.getUnixTime();
  }
  
  if(timevalue > 0){
    // nollaa tunnit
    sekun = 0;
    minuu = 0;
    tunti = 2;
    paivat_pois = timevalue % 86400;
    while(paivat_pois >= 3600){
      tunti++;
      paivat_pois = paivat_pois - 3600;
    }
    while(paivat_pois >= 60){
      minuu++;
      paivat_pois = paivat_pois - 60;
    }
    sekun = paivat_pois;
  }

  // summer aika
  HTTPClient http;  //Declare an object of class HTTPClient
  http.begin(remote_clock_sync);  //Specify request destination
  int httpCode = http.GET();//Send the request
  if (httpCode > 0) { //Check the returning code
    String payload = http.getString(); //Get the request response payload
    tunti += payload.toInt();
  }
  http.end();   //Close connection
  
  wifiDisconnect();
}

int value = 1244;
uint8_t segto = 0x80; //  | display.encodeDigit((value / 100)%10);
  
void setup() {
  Serial.begin(115200);
  display.setBrightness(2);
  display.showNumberDec(8888);
  syncTime(true);
  display.showNumberDecEx(tunti*100+minuu, (0x80 >> 1)); //Display the numCounter value;
  
}

bool update = false;

void loop(){
  delay(1000);
  sekun++;
  if(sekun >= 60){
    sekun = 0;
    minuu++;
    update = true;
  }
  if(minuu >= 60){
    minuu = 0;
    tunti++;
    tunti = tunti%24;
    update = true;
  }
  
  if(update){
    display.showNumberDecEx(tunti*100+minuu, (0x80 >> 1), true);
    update = false;
  }

  if(tunti == sync_hour && minuu == sync_min && sekun == sync_sec){
    syncTime(false);
  }

  if(tunti == turnOffHour && minuu == turnOffMin && sekun == 0){
    display.setBrightness(2, false);
  } else if(tunti == turnOnHour && minuu == turnOnMin && sekun == 0){
    display.setBrightness(2, true);
  }
  
}


