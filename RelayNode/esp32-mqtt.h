/******************************************************************************
 * Copyright 2018 Google
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
// This file contains static methods for API requests using Wifi / MQTT
#ifndef __ESP32_MQTT_H__
#define __ESP32_MQTT_H__

#include <Client.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>  // WiFi storage
#include <TimeLib.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include <CloudIoTCore.h>
#include <CloudIoTCoreMqtt.h>
#include "ciotc_config.h" // Update this file with your configuration

JsonObject templateObject;
JsonArray timerArray;

const int timeZone = 1;

//Preferences preferences;  // declare class object
String deviceConfig;
String deviceCommand = "none";

int foggyValve = 12;
int wateringValve = 13;
int wateringPump = 14;
int mixFertilizerPump = 27;
int potassiumPump = 32;
int foliarPump = 33;

///////////////////////////////

// Initialize WiFi and MQTT for this board
Client *netClient;
CloudIoTCoreDevice *device;
CloudIoTCoreMqtt *mqtt;
MQTTClient *mqttClient;
unsigned long iat = 0;
String jwt;

///////////////////////////////
// Helpers specific to this board
///////////////////////////////
String getDefaultState() {

    preferences.begin("config", false);
       String selfVersion  =  preferences.getString("version", "none");     
    preferences.end();


  
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6);
  DynamicJsonDocument doc(capacity);
  
  doc["version"] = selfVersion;
  doc["status"] = "connected";
  
  if(templateObject != NULL){


    
    doc["mode"] = templateObject["mode"];
    doc["range"] = templateObject["range"];

    preferences.begin("template", false);
       int day = preferences.getInt("day", 0);     
    preferences.end();
    
    doc["day"] = day;
  }

  if(timerArray != NULL && sizeof(timerArray) > 0){
    doc["timer"] = sizeof(timerArray);
  }
  
  String json ="" ;
  serializeJson(doc, json);
  return json;
}

String getCommandStatus(String now, const char* mode, const char* range, const char* status, int day, const char* port, int duration){
    preferences.begin("config", false);
       String selfVersion  =  preferences.getString("version", "none");     
    preferences.end();
  
  const size_t capacity = 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5);
  DynamicJsonDocument doc(capacity);
  
  doc["version"] = selfVersion;
  doc["status"] = "commandExecuted";

  JsonObject lastActivity = doc.createNestedObject("lastActivity");
  lastActivity["date"] = now;
  
  JsonObject lastActivity_command = lastActivity.createNestedObject("command");


  if(strcmp(status, "set-day") == 0){
    lastActivity_command["status"] = status;
    lastActivity_command["day"] = day;
  }else{
    lastActivity_command["status"] = status;
    lastActivity_command["port"] = port;
    lastActivity_command["duration"] = duration;
  }

 String json ="" ;
  serializeJson(doc, json);
  Serial.println("updating status");
  Serial.println(json);

//char* json ="command" ;
  return json;
}
String getScheduleStatus(String now, String port, const char* status, String time, int duration){
    preferences.begin("config", false);
       String selfVersion  =  preferences.getString("version", "none");     
    preferences.end();
  
  const size_t capacity = 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5);
  DynamicJsonDocument doc(capacity);
  
  doc["version"] = selfVersion;
  doc["status"] = "scheduleTrigger";

  JsonObject lastActivity = doc.createNestedObject("lastActivity");
  lastActivity["date"] = now;
  
  JsonObject lastActivity_timer = lastActivity.createNestedObject("timer");
  lastActivity_timer["port"] = port;
  lastActivity_timer["status"] = status;
  lastActivity_timer["time"] = time;
  lastActivity_timer["duration"] = duration;
  
 String json ="" ;
  serializeJson(doc, json);
  Serial.println("updating status");
  Serial.println(json);

  return json;

}

String getProcessActivityStatus(String now, String activity){
  preferences.begin("config", false);
     String selfVersion  =  preferences.getString("version", "none");     
  preferences.end();
  
  const size_t capacity = JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6);
  DynamicJsonDocument doc(capacity);
  
  doc["version"] = selfVersion;
  doc["status"] = "process-template";

  JsonObject lastActivity = doc.createNestedObject("lastActivity");
  lastActivity["date"] = now;
  
  doc["mode"] = templateObject["mode"];
  doc["range"] = templateObject["range"];

  preferences.begin("template", false);
    int day = preferences.getInt("day", 0);     
  preferences.end();
  
  doc["day"] = day;

  JsonObject lastActivity_template = lastActivity.createNestedObject("template");
  lastActivity_template["activity"] = activity;

  String json ="" ;
  serializeJson(doc, json);
  Serial.println("updating status");
  Serial.println(json);

  return json;
  
}

String getJwt() {
  iat = time(nullptr);
  Serial.println("Refreshing JWT");
  jwt = device->createJWT(iat, jwt_exp_secs);
  return jwt;
}


/*
void connectWifi() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
}*/

///////////////////////////////
// Orchestrates various methods from preceeding code.
///////////////////////////////
bool publishTelemetry(String data) {
  return mqtt->publishTelemetry(data);
}

bool publishTelemetry(const char* data, int length) {
  return mqtt->publishTelemetry(data, length);
}

bool publishTelemetry(String subfolder, String data) {
  return mqtt->publishTelemetry(subfolder, data);
}

bool publishTelemetry(String subfolder, const char* data, int length) {
  return mqtt->publishTelemetry(subfolder, data, length);
}
bool publishState(String data) {
  return mqtt->publishState(data);
}


//utility
int getPort(const char* portName){
  if(strcmp(portName, "foggyValve") == 0){
    return 12;
  }

  if(strcmp(portName, "wateringValve") == 0){
    return 13;
  }

  if(strcmp(portName, "wateringPump") == 0){
    return 14;
  }

  if(strcmp(portName, "mixFertilizerPump") == 0){
    return 127;
  }

  if(strcmp(portName, "potassiumPump") == 0){
    return 32;
  }

  if(strcmp(portName, "foliarPump") == 0){
    return 33;
  }

  return 0;
}


void connect() {
  //connectWifi();
  mqtt->mqttConnect();
}

void executeCommand(){
  String now = getLocalTime();

  char json[deviceCommand.length()];
  deviceCommand.toCharArray(json, deviceCommand.length());
  
  const size_t capacity = JSON_OBJECT_SIZE(5) + 70;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, json);
  
  const char*  port = doc["port"]; // 30
  const char* status = doc["status"]; // "on"
  const char* mode = doc["mode"]; 
  const char* range = doc["range"];
  int duration = doc["duration"]; // 30
  int day = doc["day"]; // 30



  if(strcmp(status,"set-day") == 0){
    preferences.begin("template", false);
       preferences.putInt("day", day);     
    preferences.end();
  }else if(strcmp(status,"on") == 0){
    digitalWrite(getPort(port), HIGH);   // Turn on relay 1
    if(duration > 0){
      delay(1000*duration);
      digitalWrite(getPort(port), LOW);
    }
  }else{
     digitalWrite(getPort(port), LOW);
     if(duration > 0){
      delay(1000*duration);
      digitalWrite(getPort(port), HIGH);
    }
  }

  deviceCommand = "none";

  Serial.println("Update status:");
  Serial.println(publishState(getCommandStatus(now, mode, range, status, day, port, duration)));

}

void processTimer(){

  String now = getLocalTime();

  char *token;

  for(int i = 0; i < sizeof(timerArray); i++){
    
     const char*  port = timerArray[i]["port"]; // 0
     const char* status = timerArray[i]["status"]; // "on"
     const char* time = timerArray[i]["time"]; // "07:00"
     int duration = timerArray[i]["duration"]; // 10

     String timeStr = String(time);

     bool trigger = isTrigger(timeStr.substring(0, timeStr.indexOf(":")), timeStr.substring(timeStr.indexOf(":")+1));
     
    //Serial.print("Is triger for " + timeStr + ": ");
    //Serial.println(trigger);
    if(trigger){
      if(strcmp(status,"on") == 0){
        digitalWrite(getPort(port), HIGH);   // Turn on relay 1
        if(duration > 0){
          delay(1000*duration);
          digitalWrite(getPort(port), LOW);
        }
      }else{
         digitalWrite(getPort(port), LOW);
         if(duration > 0){
          delay(1000*duration);
          digitalWrite(getPort(port), HIGH);
        }
      }
      publishState(getScheduleStatus(now, port, status, timeStr, duration));
      delay(60000-1000*duration+1);
    
    }
  }
}

void foggySprout(int foggySproutDuration) {
  String now = getLocalTime();
  
  digitalWrite(foggyValve, HIGH);   // Turn on relay 1
  delay(1000*foggySproutDuration);
  digitalWrite(foggyValve, LOW);
 
  publishState(getProcessActivityStatus(now, "foggy-sprout"));
}



void mixFertilizer(int mixFertilizerPumpDuration, int mixFertilizerWateringValveDuration){
  String now = getLocalTime();
  
//100cc/ต้น
  digitalWrite(mixFertilizerPump, HIGH);   // Turn on relay 1
  delay(1000*mixFertilizerPumpDuration);
  digitalWrite(mixFertilizerPump, LOW);

//900cc/ต้น
  digitalWrite(wateringValve, HIGH);   // Turn on relay 1
  delay(1000*mixFertilizerWateringValveDuration);
  digitalWrite(wateringValve, LOW);


  publishState(getProcessActivityStatus(now, "mix-fertilizer"));
  
}

void potassium(int potassiumPumpDuration, int potassiumWateringValveDuration){
  String now = getLocalTime();
  
//100cc/ต้น
  digitalWrite(potassiumPump, HIGH);   // Turn on relay 1
  delay(1000*potassiumPumpDuration);
  digitalWrite(potassiumPump, LOW);

//400cc/ต้น
  digitalWrite(wateringValve, HIGH);   // Turn on relay 1
  delay(1000*potassiumWateringValveDuration);
  digitalWrite(wateringValve, LOW);

  publishState(getProcessActivityStatus(now, "mix-potassium"));
  
}
void watering(int wateringPumpDuration) {
   String now = getLocalTime();
  digitalWrite(wateringPump, HIGH);   // Turn on relay 1
  delay(1000*wateringPumpDuration);
  digitalWrite(wateringPump, LOW);
  publishState(getProcessActivityStatus(now, "watering"));
}
void potassiumWatering(int potassiumWateringPumpDuration) {
  String now = getLocalTime();
  digitalWrite(wateringPump, HIGH);   // Turn on relay 1
  delay(1000*potassiumWateringPumpDuration);
  digitalWrite(wateringPump, LOW);
  publishState(getProcessActivityStatus(now, "potassium-watering"));
}

void foliarSprout(int foggySproutDuration) {
  String now = getLocalTime();
  digitalWrite(foliarPump, HIGH);   // Turn on relay 1
  delay(1000*foggySproutDuration);
  digitalWrite(foliarPump, LOW);

  publishState(getProcessActivityStatus(now, "foliar-sprout"));
}

void foliar(int foliarDuration) {
  String now = getLocalTime();
  digitalWrite(foliarPump, HIGH);   // Turn on relay 1
  delay(1000*foliarDuration);
  digitalWrite(foliarPump, LOW);
  
  publishState(getProcessActivityStatus(now, "foliar"));
}

void processTemplate(){

    preferences.begin("template", false);
      int day = preferences.getInt("day", 0);     
    preferences.end();
  
  const char* mode = templateObject["mode"]; // "melon"
  const char* range = templateObject["range"]; // "medium"
  
  JsonObject properties = templateObject["properties"];
  int foggySproutDuration = properties["foggySproutDuration"]; // 15
  int foliarSproutDuration = properties["foliarSproutDuration"]; // 10
  int mixFertilizerPumpDuration = properties["mixFertilizerPumpDuration"]; // 15
  int mixFertilizerWateringValveDuration = properties["mixFertilizerWateringValveDuration"]; // 20
  int wateringPumpDuration = properties["wateringPumpDuration"]; // 30
  int potassiumPumpDuration = properties["potassiumPumpDuration"]; // 15
  int potassiumWateringValveDuration = properties["potassiumWateringValveDuration"]; // 10
  int potassiumWateringPumpDuration = properties["potassiumWateringPumpDuration"]; // 30
  int foliarDuration = properties["foliarDuration"]; // 20

  int potassiumOffset = 0;

  if(strcmp(range, "short") == 0){
    potassiumOffset = 0;
  }else if(strcmp(range, "medium") == 0){
    potassiumOffset = 2;
  }else {
    potassiumOffset = 15;
  }

  if(isTrigger("00", "00")){
    day++;
    preferences.begin("template", false);
       preferences.putInt("day", day);     
    preferences.end();
  }

  if(day >= 1 && day <= 13){
    if(isTrigger("07", "00") || isTrigger("12", "00") || isTrigger("17", "00")){
       foggySprout(foggySproutDuration);
    }
  }
  if(day == 14){
    if(isTrigger("07", "00") || isTrigger("12", "00")){
      foggySprout(foggySproutDuration);
    }
  }

  if(day == 5 || day == 8 || day == 11){
    if(isTrigger("06", "00")){
       foliarSprout(foliarSproutDuration);
    }
  }

  if(day >= 15 && day <= 70){

    if(isTrigger("06", "00")){
       mixFertilizer(mixFertilizerPumpDuration, mixFertilizerWateringValveDuration);
    }
    if(isTrigger("07", "00") || isTrigger("12", "00") || isTrigger("17", "00")){
       watering(wateringPumpDuration);
    }
  }


  if(day >= 71 && day <= 80+potassiumOffset){

    if(isTrigger("06", "00")){
       potassium(potassiumPumpDuration, potassiumWateringValveDuration);
    }
    if(isTrigger("07", "00") || isTrigger("12", "00") || isTrigger("17", "00")){
       potassiumWatering(potassiumWateringPumpDuration);
    }
  }
  
//17-24-32-39-46-53-60-67
  if(day == 17 || day == 24 || day == 32 || day == 39 || day == 46 || day == 53 || day == 60 || day == 67 || day == 71+potassiumOffset || day == 74+potassiumOffset || day == 77+potassiumOffset || day == 80+potassiumOffset ){
    if(isTrigger("06", "00")){
       foliar(foliarDuration);
    }
  }

}

void process(){

  

  const size_t capacity = JSON_ARRAY_SIZE(6) + 2*JSON_OBJECT_SIZE(3) + 6*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(9) + 640;
  DynamicJsonDocument doc(capacity);
  
  char json[deviceConfig.length()];
  deviceConfig.toCharArray(json, deviceConfig.length());
   
  deserializeJson(doc, json);
  
  const char* version = doc["version"]; // "0.2"




  //check firmware version
  preferences.begin("config", false);
     String selfVersion  =  preferences.getString("version", "none");     
  preferences.end();
  if(selfVersion != String(version)){
    updateFirmware(version);
  }


  templateObject = doc["template"];
  timerArray = doc["timer"];
  
  
  if(templateObject != NULL){
     processTemplate();
  }

  if(timerArray != NULL){
    processTimer();
  }
}

// The MQTT callback function for commands and configuration updates
// Place your message handler code here.
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  if(topic.endsWith("config")){
    deviceConfig = payload;
  }else if(topic.endsWith("commands")){
    deviceCommand = payload;
    executeCommand();
  }
}


void setupCloudIoT() {
  device = new CloudIoTCoreDevice(
      project_id, location, registry_id, device_id,
      private_key_str);

  //setupWifi();
  netClient = new WiFiClientSecure();
  mqttClient = new MQTTClient(512);
  mqttClient->setOptions(180, true, 1000); // keepAlive, cleanSession, timeout
  mqtt = new CloudIoTCoreMqtt(mqttClient, netClient, device);
  mqtt->setUseLts(true);
  mqtt->startMQTT();
}

#endif //__ESP32_MQTT_H__
