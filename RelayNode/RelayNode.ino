#include "FS.h"
#include "esp_system.h"
#include <string.h>

#include "wifi_connect.h"  // WiFi connection
#include "ota.h"
#include "esp32-mqtt.h"



  

void setup() {
  Serial.begin(115200);
  setupWifi();
  setupCloudIoT();



 pinMode(foggyValve, OUTPUT);     // Initialize the relay1 pin as an output
 pinMode(wateringValve, OUTPUT);     // Initialize the relay2 pin as an output
 pinMode(wateringPump, OUTPUT);     // Initialize the relay1 pin as an output
 pinMode(mixFertilizerPump, OUTPUT);     // Initialize the relay2 pin as an output
 pinMode(potassiumPump, OUTPUT);     // Initialize the relay1 pin as an output
 pinMode(foliarPump, OUTPUT);     // Initialize the relay2 pin as an output

  delay(2000);
} //  END setup()

unsigned long lastMillis = 0;
void loop()
{
  if ( WiFi.status() ==  WL_CONNECTED )      // Main connected loop

  { 
    // ANY MAIN LOOP CODE HERE
/*
    digitalWrite(relay1, HIGH);   // Turn on relay 1
    delay(5000);
    digitalWrite(relay1, LOW);   // Turn off relay 1
    delay(5000);

    */
    mqtt->loop();
    delay(10);  // <- fixes some issues with WiFi stability
  
    if (!mqttClient->connected()) {
      connect();
    }

    
    // TODO: replace with your code
    // publish a message roughly every second.
    if (millis() - lastMillis > 60000) {
      lastMillis = millis();
      publishState(getDefaultState());
    }

    process();

    delay(500);
/*    if(deviceCommand != "none"){
      executeCommand();
    }*/

  }   // END Main connected loop()
  else
  { // WiFi DOWN
    wifiReconnect();
    delay( 1000 );  
    //digitalWrite(relay1, LOW);
  }  // END WiFi down 
} // END loop()
