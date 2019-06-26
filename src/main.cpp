#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#include <EEPROM.h>

#ifndef STASSID
#define STASSID "Huck!Lan"
#define STAPSK  "HuckSpeed2018+"
#endif

int fanSpeed = 0;
int freq = 100;

float t;
float h;


// EEPROM start index
int eeprom_tempOn_index = 10;
int eeprom_tempOff_index = 30;
int eeprom_tempEnabled_index = 100;

bool tempEnabled = true;
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
  String json = "{\"fan\":\"" + String(fanSpeed) + "\", \"temp\":\"" + t + "\",\"humi\":\"" + h + "\",\"tempEnabled\":\"" + tempEnabled + "\",\"tempOn\":\"" + tempOn + "\",\"tempOff\":\"" + tempOff + "\",\"freq\":\"" + freq + "\"}"; //"\",\"tempEnabled\":\"" + tempEnabled +
  server.send(200, "application/json", json);
}

void handleApi() {
  if (server.hasArg("fan")){
    setFanSpeed(server.arg("fan").toInt());
  }
  if (server.hasArg("freq")){
    freq = server.arg("freq").toInt();
    analogWriteFreq(freq);
  }
  if (server.hasArg("tempEnabled")){
    tempEnabled = server.arg("tempEnabled").toInt();
    EEPROM.begin(4096);
    EEPROM.put(eeprom_tempEnabled_index, tempEnabled);
    EEPROM.commit();                      // Only needed for ESP8266 to get data written
    EEPROM.end();
  }
  if (server.hasArg("tempOn")){
    tempOn = server.arg("tempOn").toFloat();
    EEPROM.begin(4096);
    EEPROM.put(eeprom_tempOn_index, tempOn);
    EEPROM.commit();                      // Only needed for ESP8266 to get data written
    EEPROM.end();
  }
  if (server.hasArg("tempOff")){
    tempOff = server.arg("tempOff").toFloat();
    EEPROM.begin(4096);
    EEPROM.put(eeprom_tempOff_index, tempOff);
    EEPROM.commit();                      // Only needed for ESP8266 to get data written
    EEPROM.end();
  }
  server.send(200,"application/json", "{\"success\":\"1\"}");
}

void loadEepromValues(){
  EEPROM.begin(4096);
  EEPROM.get( eeprom_tempOn_index, tempOn);
  EEPROM.get( eeprom_tempOff_index, tempOff);
  EEPROM.get( eeprom_tempEnabled_index, tempEnabled);
  EEPROM.end();
}

void setup() {
  analogWriteFreq(freq);
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
  loadEepromValues();
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
