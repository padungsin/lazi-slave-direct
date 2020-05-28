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


const int timeZone = 1;

//Preferences preferences;  // declare class object
String deviceConfig;
String deviceCommand = "none";

int relay[] = {12, 13};
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
  
  const size_t capacity = 3*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4);
  DynamicJsonDocument doc(capacity);
  
  doc["version"] = selfVersion;
  doc["status"] = "connected";

  
  String json ="" ;
  serializeJson(doc, json);

  return json;
}

String getCommandStatus(String now, int port, const char* status, int duration){



    preferences.begin("config", false);
       String selfVersion  =  preferences.getString("version", "none");     
    preferences.end();
  
  const size_t capacity = 3*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4);
  DynamicJsonDocument doc(capacity);
  
  doc["version"] = selfVersion;
  doc["status"] = "commandExecuted";

  JsonObject lastActivity = doc.createNestedObject("lastActivity");
  lastActivity["date"] = now;
  
  JsonObject lastActivity_command = lastActivity.createNestedObject("command");
  lastActivity_command["port"] = port;
  lastActivity_command["status"] = status;
  lastActivity_command["duration"] = duration;


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
  
  const size_t capacity = 3*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4);
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


void connect() {
  //connectWifi();
  mqtt->mqttConnect();
}

void executeCommand(){
  String now = getLocalTime();

  char json[deviceCommand.length()];
  deviceCommand.toCharArray(json, deviceCommand.length());
  
  const size_t capacity = JSON_OBJECT_SIZE(3) + 30;
  DynamicJsonDocument doc(capacity);
  
  //const char* json = "{\"port\":30,\"status\":\"on\",\"duration\":30}";
  
  deserializeJson(doc, json);
  
  int port = doc["port"]; // 30
  const char* status = doc["status"]; // "on"
  int duration = doc["duration"]; // 30

  if(strcmp(status,"on") == 0){
    digitalWrite(relay[port], HIGH);   // Turn on relay 1
    if(duration > 0){
      delay(1000*duration);
      digitalWrite(relay[port], LOW);
    }
  }else{
     digitalWrite(relay[port], LOW);
     if(duration > 0){
      delay(1000*duration);
      digitalWrite(relay[port], HIGH);
    }
  }

  deviceCommand = "none";

  Serial.println("Update status:");
  Serial.println(publishState(getCommandStatus(now, port, status, duration)));

}

void checkConfig(){

  String now = getLocalTime();

  char json[deviceConfig.length()];
  deviceConfig.toCharArray(json, deviceConfig.length());

  const size_t capacity = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 6*JSON_OBJECT_SIZE(4) + 260;
  DynamicJsonDocument doc(capacity);
  
  //const char* json = "{\"schedule\":{\"enable\":true,\"timer\":[{\"port\":0,\"time\":\"07:00\",\"duration\":10},{\"port\":0,\"time\":\"12:00\",\"duration\":10},{\"port\":0,\"time\":\"17:00\",\"duration\":10},{\"port\":1,\"time\":\"07:00\",\"duration\":10},{\"port\":1,\"time\":\"12:00\",\"duration\":10},{\"port\":1,\"time\":\"17:00\",\"duration\":10}]}}";
  
  deserializeJson(doc, json);
  const char* configVersion = doc["version"]; // "0.1";
  bool schedule_enable = doc["schedule"]["enable"]; // true
  JsonArray schedule_timer = doc["schedule"]["timer"];

//check firmware version
    preferences.begin("config", false);
       String selfVersion  =  preferences.getString("version", "none");     
    preferences.end();
  if(selfVersion != String(configVersion)){
    updateFirmware(configVersion);
  
  }

  

  char *token;

  for(int i = 0; i < sizeof(schedule_timer); i++){
    schedule_timer[i];
     int port = schedule_timer[i]["port"]; // 0
     const char* status = schedule_timer[i]["status"]; // "on"
     const char* time = schedule_timer[i]["time"]; // "07:00"
     int duration = schedule_timer[i]["duration"]; // 10

     String timeStr = String(time);

     bool trigger = isTrigger(timeStr.substring(0, timeStr.indexOf(":")), timeStr.substring(timeStr.indexOf(":")+1));
     
    //Serial.print("Is triger for " + timeStr + ": ");
    //Serial.println(trigger);
    if(trigger){


      if(strcmp(status,"on") == 0){
        digitalWrite(relay[port], HIGH);   // Turn on relay 1
        if(duration > 0){
          delay(1000*duration);
          digitalWrite(relay[port], LOW);
        }
      }else{
         digitalWrite(relay[port], LOW);
         if(duration > 0){
          delay(1000*duration);
          digitalWrite(relay[port], HIGH);
        }
      }

      
      Serial.println(publishState(getScheduleStatus(now, port, status, timeStr, duration)));
      delay(60000-1000*duration+1);
    
    }
     
  }
/*
  Serial.print("Current Time: ");
  Serial.print(hour());
  Serial.print(":");
  Serial.print(minute());
  Serial.print(":");
  Serial.println(second());
*/
  /*
  JsonObject schedule_timer_0 = schedule_timer[0];
  int schedule_timer_0_port = schedule_timer_0["port"]; // 0
  const char* schedule_timer_0_time = schedule_timer_0["time"]; // "07:00"
  int schedule_timer_0_duration = schedule_timer_0["duration"]; // 10
  
  JsonObject schedule_timer_1 = schedule_timer[1];
  int schedule_timer_1_port = schedule_timer_1["port"]; // 0
  const char* schedule_timer_1_time = schedule_timer_1["time"]; // "12:00"
  int schedule_timer_1_duration = schedule_timer_1["duration"]; // 10
  
  JsonObject schedule_timer_2 = schedule_timer[2];
  int schedule_timer_2_port = schedule_timer_2["port"]; // 0
  const char* schedule_timer_2_time = schedule_timer_2["time"]; // "17:00"
  int schedule_timer_2_duration = schedule_timer_2["duration"]; // 10
  
  JsonObject schedule_timer_3 = schedule_timer[3];
  int schedule_timer_3_port = schedule_timer_3["port"]; // 1
  const char* schedule_timer_3_time = schedule_timer_3["time"]; // "07:00"
  int schedule_timer_3_duration = schedule_timer_3["duration"]; // 10
  
  JsonObject schedule_timer_4 = schedule_timer[4];
  int schedule_timer_4_port = schedule_timer_4["port"]; // 1
  const char* schedule_timer_4_time = schedule_timer_4["time"]; // "12:00"
  int schedule_timer_4_duration = schedule_timer_4["duration"]; // 10
  
  JsonObject schedule_timer_5 = schedule_timer[5];
  int schedule_timer_5_port = schedule_timer_5["port"]; // 1
  const char* schedule_timer_5_time = schedule_timer_5["time"]; // "17:00"
  int schedule_timer_5_duration = schedule_timer_5["duration"]; // 10

  */

  



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
