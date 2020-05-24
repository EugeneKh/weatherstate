#include "DHT.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

#define DHTPIN 2  // Вывод датчика DATA подключен ко второму пину
#define DHTTYPE DHT21   // DHT 21 (AM2301)

#ifndef STASSID
#define STASSID "MilkPoint"
#define STAPSK  "getmemilk"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;


DHT dht(DHTPIN, DHTTYPE, 15);
ESP8266WebServer server(80);
DynamicJsonDocument doc(1024);

class Meas {
  private:
    int numMeas = 3;
    float quene[3];


    int compfloat(const void * a, const void * b)
    {
      return (*(float*)a - * (float*)b);
    }

  public:
    void addMeas(float meas) {
      for (int i = 1; i < numMeas; i++) {
        quene[i - 1] = quene[i];
      }
      quene[numMeas - 1] = meas;
    }

    float getMedian() {
      //qsort(quene, 10 , sizeof(float) , compfloat);
      return  quene[1];
    }
};

Meas myTemp;
Meas myHum;

class Sensors {
  private:
    unsigned long DHTtime;
  public:
    void timeStart() {
      DHTtime = millis();
    }
    void interrogation() {
      if ((millis() - DHTtime) > 2000) {
        myTemp.addMeas(dht.readTemperature());
        myHum.addMeas(dht.readHumidity());
        DHTtime = millis();
      }
    }
};

Sensors mySensors;

String createJsonResponse() {
  doc["temp"] = myTemp.getMedian();
  doc["hum"] = myHum.getMedian();
  //serializeJson(doc, Serial);
  String json;
  serializeJson(doc, json);
  return json;
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  mySensors.timeStart();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  server.on("/json", []() {
    server.send(200, "text/json", createJsonResponse());
  });
  server.begin();
}
WiFiClient client;
HTTPClient http;
void loop() {
  mySensors.interrogation();
  server.handleClient();
  http.begin(client, "http://192.168.1.77:5000/login?tmp=" + String(myTemp.getMedian()));
  Serial.println(http.GET());
  delay(2000);
}
