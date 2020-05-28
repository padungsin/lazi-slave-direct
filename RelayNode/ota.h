/* Note that this code is specific to ESP32-based boards like
 * the Feather HUZZAH32. Some modifications may be required
 * for different boards and support libraries.
 */

#include "Update.h"
#include "WiFi.h"
#include "HTTPClient.h"

HTTPClient http;
void updateFirmware(String version) {


  String url = "https://firebasestorage.googleapis.com/v0/b/smartmanipulator.appspot.com/o/RelayNode-" + version + ".bin?alt=media";
  
  
  // Start pulling down the firmware binary.
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode <= 0) {
    Serial.printf("HTTP failed, error: %s\n", 
       http.errorToString(httpCode).c_str());
    return;
  }
  // Check that we have enough space for the new binary.
  int contentLen = http.getSize();
  Serial.printf("Content-Length: %d\n", contentLen);
  bool canBegin = Update.begin(contentLen);
  if (!canBegin) {
    Serial.println("Not enough space to begin OTA");
    return;
  }
  // Write the HTTP stream to the Update library.
  WiFiClient* client = http.getStreamPtr();
  size_t written = Update.writeStream(*client);
  Serial.printf("OTA: %d/%d bytes written.\n", written, contentLen);
  if (written != contentLen) {
    Serial.println("Wrote partial binary. Giving up.");
    return;
  }
  if (!Update.end()) {
    Serial.println("Error from Update.end(): " + 
      String(Update.getError()));
    return;
  }
  if (Update.isFinished()) {

    preferences.begin("config", false);
       preferences.putString("version", version);     
    preferences.end();
    
    Serial.println("Update to version " + version + " successfully completed. Rebooting."); 
    // This line is specific to the ESP32 platform:
    ESP.restart();
  } else {
    Serial.println("Error from Update.isFinished(): " + 
      String(Update.getError()));
    return;
  }
}
