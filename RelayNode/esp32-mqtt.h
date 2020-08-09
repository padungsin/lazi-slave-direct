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

#include "device_config.h"

//config
DeviceConfig *deviceConfig;

const int timeZone = 1;

//Preferences preferences;  // declare class object
//String deviceConfig;
String deviceCommand = "none";
/*
int foggyValve = 12;
int wateringValve = 14;
int wateringPump = 25;
int mixFertilizerPump = 26;
int potassiumPump = 27;
int foliarPump = 32;
*/
///////////////////////////////

// Initialize WiFi and MQTT for this board
Client *netClient;
CloudIoTCoreDevice *device;
CloudIoTCoreMqtt *mqtt;
MQTTClient *mqttClient;
unsigned long iat = 0;
String jwt;


///////////////////////////////
// device io
///////////////////////////////
void setDeviceId(){
  Serial.print("MAC: ");
  Serial.println(MAC);
  
  String deviceId = "dev-" + MAC;

  Serial.print("Device: ");
  Serial.println(deviceId);
  
  deviceId.replace(":", "-");

  Serial.print("Device: ");
  Serial.println(deviceId);

  //char deviceChar[22];
  //Serial.print("Length: ");
  //Serial.println(deviceId.length());
  deviceId.toCharArray(device_id, sizeof(device_id));

  Serial.print("Device Char: ");
  Serial.print(device_id);
  Serial.println("*");
  
}


void setIO(){

   Serial.println("Setting Out Port");
   pinMode(deviceConfig->deviceTemplate->devicePort->foggyValve, OUTPUT);     // Initialize the relay1 pin as an output
   pinMode(deviceConfig->deviceTemplate->devicePort->wateringValve, OUTPUT);     // Initialize the relay2 pin as an output
   pinMode(deviceConfig->deviceTemplate->devicePort->wateringPump, OUTPUT);     // Initialize the relay1 pin as an output
   pinMode(deviceConfig->deviceTemplate->devicePort->mixFertilizerPump, OUTPUT);     // Initialize the relay2 pin as an output
   pinMode(deviceConfig->deviceTemplate->devicePort->potassiumPump, OUTPUT);     // Initialize the relay1 pin as an output
   pinMode(deviceConfig->deviceTemplate->devicePort->foliarPump, OUTPUT);     // Initialize the relay2 pin as an output
   pinMode(deviceConfig->deviceTemplate->devicePort->otherWateringValve, OUTPUT);     // Initialize the relay1 pin as an output
   pinMode(deviceConfig->deviceTemplate->devicePort->light, OUTPUT);     // Initialize the relay2 pin as an output

  if(deviceConfig->deviceTemplate->devicePort->relayActiveHigh){
     digitalWrite(deviceConfig->deviceTemplate->devicePort->foggyValve, LOW);
     digitalWrite(deviceConfig->deviceTemplate->devicePort->wateringValve, LOW);
     digitalWrite(deviceConfig->deviceTemplate->devicePort->wateringPump, LOW);
     digitalWrite(deviceConfig->deviceTemplate->devicePort->mixFertilizerPump, LOW);
     digitalWrite(deviceConfig->deviceTemplate->devicePort->potassiumPump, LOW);
     digitalWrite(deviceConfig->deviceTemplate->devicePort->foliarPump, LOW);
     digitalWrite(deviceConfig->deviceTemplate->devicePort->otherWateringValve, LOW);
     digitalWrite(deviceConfig->deviceTemplate->devicePort->light, LOW);
  }else{
     digitalWrite(deviceConfig->deviceTemplate->devicePort->foggyValve, HIGH);
     digitalWrite(deviceConfig->deviceTemplate->devicePort->wateringValve, HIGH);
     digitalWrite(deviceConfig->deviceTemplate->devicePort->wateringPump, HIGH);
     digitalWrite(deviceConfig->deviceTemplate->devicePort->mixFertilizerPump, HIGH);
     digitalWrite(deviceConfig->deviceTemplate->devicePort->potassiumPump, HIGH);
     digitalWrite(deviceConfig->deviceTemplate->devicePort->foliarPump, HIGH);
     digitalWrite(deviceConfig->deviceTemplate->devicePort->otherWateringValve, HIGH);
     digitalWrite(deviceConfig->deviceTemplate->devicePort->light, HIGH);
  }
}


void trigger(int port, int duration){
  Serial.print("Active High:");
  Serial.println(deviceConfig->deviceTemplate->devicePort->relayActiveHigh);

  Serial.print("Trigg Port/Duration: ");
  Serial.print(port);
  Serial.print("/");
  Serial.println(duration);

  if(!deviceConfig->deviceTemplate->devicePort->relayActiveHigh){
    digitalWrite(port, LOW);   // Turn on relay 1
    delay(1000*duration);
    digitalWrite(port, HIGH);  
  }else{
    digitalWrite(port, HIGH);   // Turn on relay 1
    delay(1000*duration);
    digitalWrite(port, LOW);
  }
}

void invertTrigger(int port, int duration){

  if(!deviceConfig->deviceTemplate->devicePort->relayActiveHigh){
    digitalWrite(port, HIGH);   // Turn on relay 1
    delay(1000*duration);
    digitalWrite(port, LOW);  
  }else{
    digitalWrite(port, LOW);   // Turn on relay 1
    delay(1000*duration);
    digitalWrite(port, HIGH);
  }
}
///////////////////////////////
// extract json
///////////////////////////////
void extractConfig(String jsonConfig){
  const size_t capacity = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(2) + 6*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(7) + JSON_OBJECT_SIZE(10) + 430;
  DynamicJsonDocument doc(capacity);
  
  char json[jsonConfig.length()];
  jsonConfig.toCharArray(json, jsonConfig.length());
  
  deserializeJson(doc, json);

  deviceConfig = new DeviceConfig();

  deviceConfig->version = String((const char*)doc["v"]); // "0.0.4"

  if(doc["t"] != NULL){
    JsonObject jsonTemplate = doc["t"];
    DeviceTemplate *deviceTemplate = new DeviceTemplate();
    deviceTemplate->mode = String((const char*)jsonTemplate["m"]); // "melon"
    deviceTemplate->range = String((const char*)jsonTemplate["r"]); // "medium"
    deviceTemplate->cutOff = String((const char*)jsonTemplate["c"]); // "melon"


    
    JsonObject jsonDevicePort = jsonTemplate["port"];
    DevicePort *devicePort = new DevicePort();
    devicePort->foggyValve = jsonDevicePort["fv"]; // 12
    devicePort->wateringValve = jsonDevicePort["wv"]; // 14
    devicePort->wateringPump = jsonDevicePort["wp"]; // 25
    devicePort->mixFertilizerPump = jsonDevicePort["mfp"]; // 26
    devicePort->potassiumPump = jsonDevicePort["kp"]; // 27
    devicePort->foliarPump = jsonDevicePort["fp"]; // 32
    devicePort->fertilizerKnead = jsonDevicePort["fkn"]; // 27
    devicePort->otherWateringValve = jsonDevicePort["owv"]; // 32
    devicePort->light = jsonDevicePort["lght"]; // false

    deviceTemplate->devicePort = devicePort;

    JsonObject jsonProperty = jsonTemplate["p"];
    DeviceTemplateProperty *property = new DeviceTemplateProperty();


      
    property->foggySproutDuration = jsonProperty["foggyspdrtn"]; // 15
    property->foliarSproutDuration = jsonProperty["foliarspdrtn"]; // 10
    property->pureWateringValveDuration = jsonProperty["wvdrtn"]; // 15
    property->mixFertilizerPumpDuration = jsonProperty["mixpdrtn"]; // 15
    property->mixFertilizerWateringValveDuration = jsonProperty["mixwvdrtn"]; // 20
    property->wateringPumpDuration = jsonProperty["wpdrtn"]; // 30
    property->potassiumPumpDuration = jsonProperty["kpdrtn"]; // 15
    property->potassiumWateringValveDuration = jsonProperty["kwvdrtn"]; // 10
    property->potassiumWateringPumpDuration = jsonProperty["kwpdrtn"]; // 30
    property->foliarDuration = jsonProperty["foliardrtn"]; // 20
    property->kneadDuration = jsonProperty["kndrtn"]; // 30
    deviceTemplate->property = property;

    deviceConfig->deviceTemplate = deviceTemplate;
    
  }
/*  
  
  if(doc["timer"] != NULL){
    JsonArray jsonTimer = doc["timer"];
    deviceConfig->timerCount = sizeof(jsonTimer);
    
    Timer timer[sizeof(jsonTimer)];
    for(int i = 0; i < sizeof(jsonTimer); i++){
      deviceConfig->timer[i] = new Timer();
      
      deviceConfig->timer[i]->port = String((const char*)jsonTimer[i]["port"]); // "wateringPump"
      deviceConfig->timer[i]->status = String((const char*)jsonTimer[i]["status"]); // "on"
      deviceConfig->timer[i]->time = String((const char*)jsonTimer[i]["time"]); // "07:00"
      deviceConfig->timer[i]->duration = jsonTimer[i]["duration"]; // 60
    }
  }

  */
}


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
  
  if(deviceConfig->deviceTemplate != NULL){
    doc["mode"] = deviceConfig->deviceTemplate->mode;
    doc["range"] = deviceConfig->deviceTemplate->range;

    preferences.begin("template", false);
       int day = preferences.getInt("day", 0);     
    preferences.end();
    
    doc["day"] = day;
  }

  if(deviceConfig->timerCount > 0){
    doc["timer"] = deviceConfig->timerCount;
  }
  
  String json ="" ;
  serializeJson(doc, json);
  return json;

}

String getCommandStatus(String now, const char* mode, const char* range, const char* status, int day, int port, int duration){
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
String getScheduleStatus(String now, int port, const char* status, String time, int duration){
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

String getProcessActivityStatus(String now, String activity, int port, int duration){
  preferences.begin("config", false);
     String selfVersion  =  preferences.getString("version", "none");     
  preferences.end();
  
  const size_t capacity = JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6);
  DynamicJsonDocument doc(capacity);
  
  doc["version"] = selfVersion;
  doc["status"] = "process-template";

  JsonObject lastActivity = doc.createNestedObject("lastActivity");
  lastActivity["date"] = now;
  
  doc["mode"] = deviceConfig->deviceTemplate->mode;
  doc["range"] = deviceConfig->deviceTemplate->range;

  preferences.begin("template", false);
    int day = preferences.getInt("day", 0);     
  preferences.end();
  
  doc["day"] = day;

  JsonObject lastActivity_template = lastActivity.createNestedObject("template");
  lastActivity_template["activity"] = activity;
  lastActivity_template["port"] = port;

  lastActivity_template["durationity"] = duration;


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

  Serial.println("publishing: ");
  Serial.println(data);
  return mqtt->publishState(data);
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
  
  int  port = doc["port"]; // 30
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
    trigger(port, duration);
  }else{
     invertTrigger(port, duration);
  }

  deviceCommand = "none";

  Serial.println("Update status:");
  Serial.println(publishState(getCommandStatus(now, mode, range, status, day, port, duration)));

}
/*
void processTimer(){

  String now = getLocalTime();

  char *token;

  for(int i = 0; i < sizeof(timerArray); i++){
    
     int  port = timerArray[i]["port"]; // 0
     const char* status = timerArray[i]["status"]; // "on"
     const char* time = timerArray[i]["time"]; // "07:00"
     int duration = timerArray[i]["duration"]; // 10

     String timeStr = String(time);

  
     
    //Serial.print("Is triger for " + timeStr + ": ");
    //Serial.println(trigger);
    if(isTrigger(timeStr.substring(0, timeStr.indexOf(":")), timeStr.substring(timeStr.indexOf(":")+1))){
      if(strcmp(status,"on") == 0){
        trigger(port, duration);   // Turn on relay 1
      }else{
         invertTrigger(port, duration);
      }
      publishState(getScheduleStatus(now, port, status, timeStr, duration));
      if(duration < 60){
        delay(60000-1000*duration+1);
      }
    
    }
  }
}*/

void foggySprout() {
  String now = getLocalTime();
  trigger(deviceConfig->deviceTemplate->devicePort->foggyValve, deviceConfig->deviceTemplate->property->foggySproutDuration);

  publishState(getProcessActivityStatus(now, "foggy-sprout", deviceConfig->deviceTemplate->devicePort->foggyValve, deviceConfig->deviceTemplate->property->foggySproutDuration));
  if(deviceConfig->deviceTemplate->property->foggySproutDuration < 60){
    delay(60000-1000*deviceConfig->deviceTemplate->property->foggySproutDuration);
  }
}

void pureWater(){
  String now = getLocalTime();
  //1000cc/ต้น
  trigger(deviceConfig->deviceTemplate->devicePort->wateringValve, deviceConfig->deviceTemplate->property->pureWateringValveDuration);

  publishState(getProcessActivityStatus(now, "mix-fertilizer", deviceConfig->deviceTemplate->devicePort->wateringValve, deviceConfig->deviceTemplate->property->pureWateringValveDuration));
  if(deviceConfig->deviceTemplate->property->pureWateringValveDuration < 60){
    delay(60000-1000*deviceConfig->deviceTemplate->property->pureWateringValveDuration);
  }

}

void mixFertilizer(){
  String now = getLocalTime();
  //คนปุ๋ย
  trigger(deviceConfig->deviceTemplate->devicePort->fertilizerKnead, deviceConfig->deviceTemplate->property->kneadDuration);
  publishState(getProcessActivityStatus(now, "knead-fertilizer",deviceConfig->deviceTemplate->devicePort->fertilizerKnead, deviceConfig->deviceTemplate->property->kneadDuration));
  
  //100cc/ต้น
  trigger(deviceConfig->deviceTemplate->devicePort->mixFertilizerPump, deviceConfig->deviceTemplate->property->mixFertilizerPumpDuration);
  publishState(getProcessActivityStatus(now, "pump-fertilizer",deviceConfig->deviceTemplate->devicePort->mixFertilizerPump, deviceConfig->deviceTemplate->property->mixFertilizerPumpDuration));
  
  //900cc/ต้น
  trigger(deviceConfig->deviceTemplate->devicePort->wateringValve, deviceConfig->deviceTemplate->property->mixFertilizerWateringValveDuration);
  publishState(getProcessActivityStatus(now, "pump-water-fertilizer", deviceConfig->deviceTemplate->devicePort->wateringValve, deviceConfig->deviceTemplate->property->mixFertilizerWateringValveDuration));
  
  if(deviceConfig->deviceTemplate->property->mixFertilizerPumpDuration+deviceConfig->deviceTemplate->property->mixFertilizerWateringValveDuration < 60){
    delay(60000-1000*(deviceConfig->deviceTemplate->property->mixFertilizerPumpDuration+deviceConfig->deviceTemplate->property->mixFertilizerWateringValveDuration + deviceConfig->deviceTemplate->property->kneadDuration));
  }

}

void potassium(){
  String now = getLocalTime();
  
//100cc/ต้น
  trigger(deviceConfig->deviceTemplate->devicePort->potassiumPump, deviceConfig->deviceTemplate->property->potassiumPumpDuration);
publishState(getProcessActivityStatus(now, "pump-potassium", deviceConfig->deviceTemplate->devicePort->potassiumPump, deviceConfig->deviceTemplate->property->potassiumPumpDuration));

  
//400cc/ต้น
  trigger(deviceConfig->deviceTemplate->devicePort->wateringValve, deviceConfig->deviceTemplate->property->potassiumWateringValveDuration);

  publishState(getProcessActivityStatus(now, "pump-water-potassium", deviceConfig->deviceTemplate->devicePort->wateringValve, deviceConfig->deviceTemplate->property->potassiumWateringValveDuration));

  if(deviceConfig->deviceTemplate->property->potassiumPumpDuration+deviceConfig->deviceTemplate->property->potassiumWateringValveDuration < 60){
    delay(60000-1000*(deviceConfig->deviceTemplate->property->potassiumPumpDuration+deviceConfig->deviceTemplate->property->potassiumWateringValveDuration));
  }
  
  
}
void watering() {
   String now = getLocalTime();

//คน
   trigger(deviceConfig->deviceTemplate->devicePort->mixWateringKnead, deviceConfig->deviceTemplate->property->kneadDuration);
   publishState(getProcessActivityStatus(now, "knead-watering", deviceConfig->deviceTemplate->devicePort->mixWateringKnead, deviceConfig->deviceTemplate->property->kneadDuration));

   trigger(deviceConfig->deviceTemplate->devicePort->wateringPump, deviceConfig->deviceTemplate->property->wateringPumpDuration);
   publishState(getProcessActivityStatus(now, "watering", deviceConfig->deviceTemplate->devicePort->wateringPump, deviceConfig->deviceTemplate->property->wateringPumpDuration));

  if(deviceConfig->deviceTemplate->property->wateringPumpDuration < 60){
    delay(60000-1000*(deviceConfig->deviceTemplate->property->wateringPumpDuration + deviceConfig->deviceTemplate->property->kneadDuration));
  }
}
void potassiumWatering() {
  String now = getLocalTime();

  trigger(deviceConfig->deviceTemplate->devicePort->wateringPump, deviceConfig->deviceTemplate->property->potassiumWateringPumpDuration);

  publishState(getProcessActivityStatus(now, "potassium-watering", deviceConfig->deviceTemplate->devicePort->wateringPump, deviceConfig->deviceTemplate->property->potassiumWateringPumpDuration));
  if(deviceConfig->deviceTemplate->property->potassiumWateringPumpDuration < 60){
    delay(60000-1000*deviceConfig->deviceTemplate->property->potassiumWateringPumpDuration);
  }
}

void foliarSprout() {
  String now = getLocalTime();
  
  trigger(deviceConfig->deviceTemplate->devicePort->foliarPump, deviceConfig->deviceTemplate->property->foggySproutDuration);

  publishState(getProcessActivityStatus(now, "foliar-sprout", deviceConfig->deviceTemplate->devicePort->foliarPump, deviceConfig->deviceTemplate->property->foggySproutDuration));
  if(deviceConfig->deviceTemplate->property->foggySproutDuration < 60){
    delay(60000-1000*deviceConfig->deviceTemplate->property->foggySproutDuration);
  }
 
}

void foliar() {
  String now = getLocalTime();

  trigger(deviceConfig->deviceTemplate->devicePort->foliarPump, deviceConfig->deviceTemplate->property->foliarDuration);
  
  publishState(getProcessActivityStatus(now, "foliar", deviceConfig->deviceTemplate->devicePort->foliarPump, deviceConfig->deviceTemplate->property->foliarDuration));
  if(deviceConfig->deviceTemplate->property->foliarDuration < 60){
    delay(60000-1000*deviceConfig->deviceTemplate->property->foliarDuration);
  }
  
}

void processTemplate(){

    preferences.begin("template", false);
      int day = preferences.getInt("day", 0);     
    preferences.end();
  /*
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
*/
  int potassiumOffset = 0;

  if(deviceConfig->deviceTemplate->range == "short"){
    potassiumOffset = 0;
  }else if(deviceConfig->deviceTemplate->range == "medium"){
    potassiumOffset = 2;
  }else {
    potassiumOffset = 15;
  }


  if(isTrigger(deviceConfig->deviceTemplate->cutOff.substring(0, deviceConfig->deviceTemplate->cutOff.indexOf(":")), deviceConfig->deviceTemplate->cutOff.substring(deviceConfig->deviceTemplate->cutOff.indexOf(":")+1))){
    day++;
    Serial.print("Change Day: ");
    Serial.println(day);
    preferences.begin("template", false);
       preferences.putInt("day", day);     
    preferences.end();

    delay(60000+1);
  }

  //temp for other plant
    if(isTrigger("07", "00") || isTrigger("17", "00")){
       foggySprout();
    }

/*
  if(day >= 1 && day <= 15){
    if(isTrigger("07", "00") || isTrigger("09", "00") || isTrigger("11", "00") ||isTrigger("13", "00") || isTrigger("15", "00")|| isTrigger("17", "00")){
       foggySprout();
    }
  }
  if(day == 14){
    if(isTrigger("07", "00") || isTrigger("12", "00")){
      foggySprout();
    }
  }
*/
  if(day == 5 || day == 8 || day == 11){
    if(isTrigger("06", "00")){
       foliarSprout();
    }
  }


  if(day >= 16 && day <= 17){

    if(isTrigger("06", "00")){
       pureWater();
    }
    if(isTrigger("07", "00") || isTrigger("12", "00") || isTrigger("17", "00")){
       watering();
    }
  }



  if(day >= 18 && day <= 70){

    if(isTrigger("06", "00")){
       mixFertilizer();
    }
    if(isTrigger("07", "00") || isTrigger("12", "00") || isTrigger("17", "00")){
       watering();
    }
  }
  
  if(day >= 17 && day <= 70){

    if(isTrigger("06", "00")){
       mixFertilizer();
    }
    if(isTrigger("07", "00") || isTrigger("10", "00") || isTrigger("13", "00") || isTrigger("16", "00") || isTrigger("18", "00")){
       watering();
    }
  }


  if(day >= 71 && day <= 80+potassiumOffset){

    if(isTrigger("06", "00")){
       potassium();
    }
    if(isTrigger("07", "00") || isTrigger("10", "00") || isTrigger("13", "00") || isTrigger("16", "00") || isTrigger("18", "00")){
       potassiumWatering();
    }
  }
  
//17-24-32-39-46-53-60-67
  if(day == 17 || day == 24 || day == 32 || day == 39 || day == 46 || day == 53 || day == 60 || day == 67 || day == 71+potassiumOffset || day == 74+potassiumOffset || day == 77+potassiumOffset || day == 80+potassiumOffset ){
    if(isTrigger("06", "00")){
       foliar();
    }
  }

}

void process(){

  if(deviceConfig == NULL){
    return;
  }
  

  //check firmware version
  preferences.begin("config", false);
     String selfVersion  =  preferences.getString("version", "none");     
  preferences.end();
  if(selfVersion != deviceConfig->version){
    updateFirmware(deviceConfig->version);
  }
  if(deviceConfig->deviceTemplate != NULL){
     processTemplate();
  }
/*
  if(deviceConfig->timerCount > 0){
    processTimer();
  }*/
}

// The MQTT callback function for commands and configuration updates
// Place your message handler code here.
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic );
  Serial.println(payload);
  if(topic.endsWith("config")){
    extractConfig(payload);
    setIO();
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
