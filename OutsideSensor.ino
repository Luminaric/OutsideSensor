/*
This sketch uses the DHT22 to get Temprature and Humidity
readings. It then uses the ESP8266 to transmit those readings
to the local MySQL database and to ThingSpeak. A reading is
performed every 5 minuts using Timer.

  PGM:      OutsideSensor
  Platform: ESP8266
  Ver:      1.1
  Date:     2015-11-03
  Requires:
  Hardware: Uses the DHT22 Temp and Humid sensor.
*/
#include "Timer.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "ethernetSettings.h"
#include <DHT.h>

extern "C" {
#include "user_interface.h"
uint16 readvdd33(void);
}

#define MYSQL_OPEN_STRING     "Get /add2.php?f0="
#define CLOSE_STRING          "Host: 10.1.1.25 Connection: close"
#define DHTPIN                2
#define DHTTYPE               DHT22
#define ONE_MINUTE            60000
#define FIVE_MINUTE           300000
#define TEN_MINUTE            600000
#define SIXTY_MINUTE          3600000
#define SENSORID              "ESP03"

DHT dht(DHTPIN, DHTTYPE, 16);

//Setup Sensor Interface
float temp, humidity;
//long previousMillis = 0, previousMillisHour = 0, currentMillis = 0;
String getString, getIOTString, sensorID;

WiFiClient client;

Timer t;

void doIT() {
  //Get the sensor readings and send values to serial port
  float humidity = dht.readHumidity();
  float temp = dht.readTemperature();
  float battery = readvdd33() / 1000.0;
  Serial.print("Sensors readings: ");
  Serial.print(temp);
  Serial.print(" | ");
  Serial.print(humidity);
  Serial.print(" | ");
  Serial.println(battery);
  //make the MySQL string
  getString = MYSQL_OPEN_STRING;
  getString += SENSORID;
  getString += "&f1=";
  getString += temp;
  getString += "&f2=";
  getString += humidity;
  getString += "&f3=";
  getString += battery;
  getString += "&f4=";
  getString += battery;
  Serial.print("sqlString: ");
  Serial.println(getString);
  Serial.println();
  //Send data to the MySQL server
  if (client.connect(serverLH, 80))  {
    client.println (getString);
    client.println(CLOSE_STRING);
    client.println();
    client.println();
    client.stop();
    client.stop();
  }
  else  {
    Serial.println("Transmission error with MySQL server");
    client.stop();
  }
  //make the IOT string
  getIOTString = "field1=";
  getIOTString += SENSORID;
  getIOTString += "&field2=";
  getIOTString += temp*1.8+32;
  getIOTString += "&field3=";
  getIOTString += humidity;
  Serial.print("iotString: ");
  Serial.println(getIOTString);
  //Send the data to ThingSpeak
  if (client.connect(serverTS, 80))  { 
      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection:close\n");
      client.print("X-THINGSPEAKAPIKEY: B34V6DS7CPZOWDP0\n");
      client.print("Content-type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(getIOTString.length());
      client.print("\n\n");
      client.print(getIOTString);
      client.stop();
  }
  else  {
      Serial.println("Transmission error with ThingSpeak server");
      client.stop();
  }
}
void timeIsUp()
{
  Serial.println("10 minutes have elapsed, getting and sending data");
  doIT();
}

void setup()
{
  dht.begin();
  Serial.begin(9600);
//  int oneMinutePause = t.every(ONE_MINUTE, oneMinute);
//  Serial.print("1 minute tick started id= ");
//  Serial.println(oneMinutePause);
  Serial.print(SENSORID);
  Serial.print(" is using Pin ");
  Serial.print(DHTPIN);
  Serial.println(" For DHT sensor.");
  //int fiveMinutePause = t.every(FIVE_MINUTE, fiveMinutes);
  int tenMinutePause = t.every(TEN_MINUTE, timeIsUp);
  Serial.print("10 minute tick started id= ");
  Serial.println(tenMinutePause);
  
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
//collect the first reading and send it.
  if (!client.connect(serverLH, 80)) {
    Serial.println("connection failed");
    return;
  } else  {
      doIT();
  }
  delay(1000);
    Serial.println("Setup complete, starting timer loop");
}
  
void loop()
{
  t.update();
}

//String sqlString ()  {
//  getString = MYSQL_OPEN_STRING;
//  getString += SENSORID;
//  getString += "&t=";
//  getString += temp;
//  getString += "&h=";
//  getString += humidity;
//  getString += "&p=";
//  getString += battery;
//  /*getString += "&hi=";
//  getString += DHTHeatIndex;*/
//  Serial.print("sqlString: ");
//  Serial.println(getString);
//  Serial.println();
//  return (getString);
//}
//
//String iotString ()  {
//  getIOTString = "field1=";
//  getIOTString += SENSORID;
//  getIOTString += "&field2=";
//  getIOTString += temp*1.8+32;
//  getIOTString += "&field3=";
//  getIOTString += humidity;
//  Serial.print("iotString: ");
//  Serial.println(getIOTString);
//  return (getIOTString);
//}
//float getLocalSensor()  {
//  float humidity = dht.readHumidity();
//  float temp = dht.readTemperature();
//  float battery = analogRead(A0);
//  Serial.print("Sensors readings: ");
//  Serial.print(temp);
//  Serial.print(" | ");
//  Serial.print(humidity);
//  Serial.print(" | ");
//  Serial.println(battery);
//  return (humidity, temp,battery);
//}
//void  sendMySQL()  {
//  if (client.connect(serverLH, 80))  {
//    client.println (getString);
//    //Serial.println(getString);
//    client.println(CLOSE_STRING);
//    //Serial.println(CLOSE_STRING);
//    client.println();
//    client.println();
//    client.stop();
//    client.stop();
//  }
//  else  {
//    Serial.println("Transmission error with MySQL server");
//    client.stop();
//  }
//}
//
//void  sendIoT()  {
//  if (client.connect(serverTS, 80))  { 
//      client.print("POST /update HTTP/1.1\n");
//      client.print("Host: api.thingspeak.com\n");
//      client.print("Connection:close\n");
//      client.print("X-THINGSPEAKAPIKEY: UMBGKCJ3ZULDV9Y2\n");
//      client.print("Content-type: application/x-www-form-urlencoded\n");
//      client.print("Content-Length: ");
//      client.print(getIOTString.length());
//      client.print("\n\n");
//      client.print(getIOTString);
//      client.stop();
//  }
//  else  {
//      Serial.println("Transmission error with ThingSpeak server");
//      client.stop();
//  }
//}
//
//void writeData()  {
//  iotString ();
//  sendIoT();
//  sqlString ();
//  sendMySQL();
//  Serial.println();
//  Serial.println("Sent data to servers");
//  Serial.println();
//}

