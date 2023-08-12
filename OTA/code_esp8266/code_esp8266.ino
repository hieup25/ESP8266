#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>

#define ATOMIC_FS_UPDATE

const char *ssid = "TP-Link_0D5E";
const char *password = "93853767";
const char *currentVersion = "3";
const char *serverUrl = "http://192.168.0.120:3000/getOTA";
ESP8266HTTPUpdate esp_http_update;
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // Connect to wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi connected");
  Serial.print(" at address: http://");
  Serial.println(WiFi.localIP());

  // Check newest version
  WiFiClient client;
  esp_http_update.onProgress([](size_t cur, size_t total) {
    Serial.print("Progress Upload: ");
    Serial.print((float)cur * 100 / total);
    Serial.println("%");
  });
  t_httpUpdate_return ret = esp_http_update.update(client, serverUrl, currentVersion);
  switch (ret) {
    case HTTP_UPDATE_NO_UPDATES:
      {
        Serial.println("No need update verion");
        break;
      }
    case HTTP_UPDATE_OK:
      {
        Serial.println("HTTP_UPDATE_OK");
        break;
      }
    default:
      {
        Serial.printf("Update faild error (%d): %s\n",
                      esp_http_update.getLastError(),
                      esp_http_update.getLastErrorString().c_str());
        break;
      }
  }
}
void loop() {
  // TODO SOMETHING
  Serial.println("Version: " + String(currentVersion));
  delay(2000);
}
