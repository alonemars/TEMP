#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
//#include <WiFiUdp.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>


#define TEMPERATURE_PRECISION 12
OneWire ds(5);
DallasTemperature sensors(&ds);
DeviceAddress tempDeviceAddress,temp_adress[4];
int numberOfDevices;
float temp[]={};
const char* ssid     = "xxxxxx";
const char* password = "xxxxxx"; 
const char* host = "api.heclouds.com";
const char* APIKEY = "xxxxxx";    // API KEY
int32_t deviceId = xxxxxx;                             // Device ID
const char* DS_Temp1 = "temperature_1";                 // 数据流 - temperature_1
const char* DS_Temp2 = "temperature_2";                 // 数据流 - temperature_2
const size_t MAX_CONTENT_SIZE = 1024;                  //
const unsigned long HTTP_TIMEOUT = 2100;                // max respone time from server
const int tcpPort = 80;
WiFiClient client;

void ds18b20_init(){
  sensors.begin();
  numberOfDevices =int (sensors.getDeviceCount());
    for(int i=0;i<numberOfDevices; i++)
//    for(int i=0;i<2; i++)
      {
      // Search the wire for address
      if(sensors.getAddress(tempDeviceAddress, i))
      {
          Serial.print("Get device ");
          Serial.print(i, DEC);
          Serial.print(" with address: ");
          for (uint8_t j = 0; j < 8; j++)
              {
                // zero pad the address if necessary
                if (tempDeviceAddress[j] < 16) Serial.print("0");
                Serial.print(tempDeviceAddress[j], HEX);
                temp_adress[i][j]=tempDeviceAddress[j];
              }
          Serial.println();
          // set the resolution to 12 bit (Each Dallas/Maxim device is capable of several different resolutions)
          sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
          Serial.print("Resolution set to: ");
          Serial.print(sensors.getResolution(tempDeviceAddress), DEC);
          Serial.println();
      }
      else
          {
          Serial.print("Found ghost device at ");
          Serial.print(i, DEC);
          }
      }
}

void ds18b20_data()
{
  sensors.requestTemperatures();
  // Serial.println(numberOfDevices);
//  for(int i=0;i<numberOfDevices;i++)
  for(int i=0;i<numberOfDevices;i++)
    {// Search the wire for address
//        Serial.print("0+");
//        Serial.println(i);
        if(sensors.getAddress(tempDeviceAddress, i))
          {
            float tempC = sensors.getTempC(tempDeviceAddress);
             Serial.print("Temp C: ");
             Serial.println(tempC);
            // Serial.print(" Temp F: ");
            // Serial.println(DallasTemperature::toFahrenheit(tempC));
            temp[i]=tempC;
//            Serial.println(i);
//            Serial.println();
//           if (i>=1)
//              break;
          }
          else
            {
              break;
            }
          
   //else ghost device! Check your power requirements and cabling
  }
}

void post_OneNet(int dId){
  if (!client.connect(host, tcpPort))
      return;
//  Serial.println("post_OneNet Starting...");
  String url = "/devices/";
  url += String(dId);
  url += "/datapoints?type=3";           //http://open.iot.10086.cn/doc/art190.html#43
  String data_String = "{\"" + String(DS_Temp1) + "\":" + String(temp[0]) + ",\"" + String(DS_Temp2) + "\":" + String(temp[1]) + "}";
  String post_DATA = "POST " + url + " HTTP/1.1\r\n" +
                     "api-key:" + APIKEY + "\r\n" +
                     "Host:" + host + "\r\n" +
                     "Content-Length: " + String(data_String.length()) + "\r\n" +                     //发送数据长度
                     "Connection: close\r\n\r\n" +
                     data_String;
  client.print(post_DATA);
//  Serial.println("POS DATA...");
//  Serial.println(post_DATA);
  Serial.println("POS Ok.");
  delay(150000);
}


void setup() {
//post_CMCC();
  WiFi.mode(WIFI_STA);                 //set work mode:  WIFI_AP /WIFI_STA /WIFI_AP_STA
  Serial.begin(115200);
  delay(10);
   // We start by connecting to a WiFi network
  Serial.println("");
  Serial.print("Connecting WIFI ");
  Serial.print(ssid);
  WiFi.hostname("SOHO_Temp_1");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Hostname: ");
  Serial.println(WiFi.hostname());
 ds18b20_init();
}

void loop() {
ds18b20_data();
post_OneNet(deviceId);
ESP.wdtFeed();
delay(150000);
}
