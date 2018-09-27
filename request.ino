#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "index.h"
#include "clock.h"

#define LED 2

ESP8266WebServer server(80);

void handleRoot() {
 String s = MAIN_page;
 server.send(200, "text/html", s);
}

void handleGetDate() { 
 server.send(200, "text/plane", "");
}

void handleSetDate() {  
  String date = server.arg("date");
  int year = server.arg("year").toInt();
  int month = server.arg("month").toInt();
  int day = server.arg("day").toInt();
  int hours = server.arg("hours").toInt();
  int minutes = server.arg("minutes").toInt();
  int seconds = server.arg("seconds").toInt();
  Serial.println(date);
  Serial.println(year);
  Serial.println(month);
  Serial.println(day);
  Serial.println(hours);
  Serial.println(minutes);
  Serial.println(seconds);  
}

void handleLED() {
 String ledState = "OFF";
 String t_state = server.arg("LEDstate");
 Serial.println(t_state);
 if(t_state == "1")
 {
  digitalWrite(LED,LOW);
  ledState = "ON";
 }
 else
 {
  digitalWrite(LED,HIGH);
  ledState = "OFF";
 }
 
 server.send(200, "text/plane", ledState);
}
//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  Serial.begin(115200);
  
  
  Serial.println("");
  
  pinMode(LED, OUTPUT); 

  WiFi.softAP("ESP-LED");
  
  Serial.println("Connecting ...");
  
  if (MDNS.begin("esp8266")) {
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  } 
    
  Serial.println("");
  Serial.print("Connected to ");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 
  server.on("/", handleRoot);
  server.on("/setLED", handleLED);
  server.on("/setDate", handleSetDate);
  server.on("/getDate", handleGetDate);

  server.begin();
  Serial.println("HTTP server started");
}
//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  server.handleClient();          //Handle client requests
  
  if(Clock.hasSecondsChanged()) {
    Clock.toString();
  }
}
