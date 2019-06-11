#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#ifndef STASSID
#define STASSID "Huck!Lan"
#define STAPSK  "HuckSpeed2018+"
#endif

int fanSpeed = 0;

float t;
float h;

boolean tempEnabled = false;
float tempOn = 120;
float tempOff = 100;

const char* ssid = STASSID;
const char* password = STAPSK;

const int dht22Pin = 5;
const int fanPin = 4;

DHT dht(dht22Pin,DHT22);
ESP8266WebServer server(80);

//prototypes
void setFanSpeed(int speed);

void sendState() {
  delay(50);
  String json = "{\"fan\":\"" + String(fanSpeed) + "\", \"temp\":\"" + t + "\",\"humi\":\"" + h + "\",\"tempEnabled\":\"" + tempEnabled + "\",\"tempOn\":\"" + tempOn + "\",\"tempOff\":\"" + tempOff + "\"}"; //"\",\"tempEnabled\":\"" + tempEnabled + 
  server.send(200, "application/json", json);
}

void handleApi() {
  if (server.hasArg("fan")){
    setFanSpeed(server.arg("fan").toInt());
  }
  if (server.hasArg("freq")){
    analogWriteFreq(server.arg("freq").toInt());
  }
  if (server.hasArg("tempEnabled")){
    tempEnabled = server.arg("tempEnabled").toInt();
  }
  if (server.hasArg("tempOn")){
    tempOn = server.arg("tempOn").toFloat();
  }
  if (server.hasArg("tempOff")){
    tempOff = server.arg("tempOff").toFloat();
  }
  server.send(200,"application/json", "{\"success\":\"1\"}");
}

void setup() {
  analogWriteFreq(100);
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(4,LOW);
  Serial.println("connecting to: " + String(ssid));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    digitalWrite(2, HIGH);
    delay(1000);
    Serial.print(".");
    digitalWrite(2, LOW);
  }
  Serial.println("dht begin");
  dht.begin();
  Serial.println("server begin");
  Serial.println("OTA begin");
  ArduinoOTA.begin();
  SPIFFS.begin();
  server.on("/state.json", sendState);
  server.on("/api", handleApi);
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/logo.png", SPIFFS, "/logo.png");
  server.begin();
}
void setFanSpeed(int speed) {
  int pin = fanPin;
  if (speed < 99 && speed != 0){
    speed = 100;
  }
  fanSpeed = speed;
  if (speed < 500 && speed != 0) {
    analogWrite(pin, 1023);
    delay(200);
  }
  analogWrite(pin,speed);
}
 
void loop() {
  server.handleClient();
  ArduinoOTA.handle();
  t = dht.readTemperature();
  h = dht.readHumidity();
  if (tempEnabled == true) {
    if (t >= tempOn) {
      setFanSpeed(1023);
    }
    if (t<= tempOff) {
      setFanSpeed(0);
    }
  }
}
