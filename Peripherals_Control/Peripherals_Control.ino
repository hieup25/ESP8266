#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <Adafruit_NeoPixel.h>
#include "DHT.h"
#define PIN_LED 13
#define NUMBER_LED 12
#define PIN_DHT 12
const char *ssid = "TP-Link_0D5E";
const char *password = "93853767";

// Khởi tạo server với port là 80
ESP8266WebServer server(80);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_LED, PIN_LED, NEO_GRB + NEO_KHZ800);
DHT dht(PIN_DHT, DHT11);

void setup() {
  Serial.begin(115200);
  // Đặt chế độ Station cho ESP8266,
  // Router đang ở chế độ AP (điểm truy cập)
  WiFi.mode(WIFI_STA);
  // Kết nối đến Wifi, có ssid và password như cấu hình
  WiFi.begin(ssid, password);
  Serial.println("");

  // Đợi đến khi kết nối thành công đến wifi này
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // in địa chỉ IP của ESP8266
  // Khởi tạo 1 mDNS có tên myserver
  // có thể truy cập bằng http://myserver.local
  if (MDNS.begin("myserver")) {
    Serial.println("MDNS responder started");
  }
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  server.on("/", handleRoot);
  server.on("/style.css", handleCSS);
  server.on("/script.js", handleJS);
  server.on("/set-color", handleColor);
  server.on("/get-dht11", handleDHT11);
  server.onNotFound(handleNotFound);
  server.begin();
  strip.begin();
  strip.setBrightness(10);
  dht.begin();
}
void handleDHT11() {
  float h = dht.readHumidity();  // Đọc giá trị độ ẩm từ cảm biến
  float t = dht.readTemperature(); // Đọc giá trị nhiệt độ từ cảm biến
  // Đóng bản tin response
  String content = "{\"t\":" + String(t) + ", \"h\":" + String(h) + "}" ;
  // response dạng json, nên Content-Type là application/json
  server.send(200, "application/json", content);
}
void handleColor() {
  String value = server.arg("value"); // nội dung là ff0000
  //
  // Code thay đổi màu sắc LED với giá trị value
  // Tương tự với các mã màu khác...
  for (int i = 0; i < strip.numPixels(); i++) //numPixels = numLEDs
  {
    strip.setPixelColor(i, strtoul(value.c_str(), NULL, 16));
    strip.show();
    delay(20);
  }
  //
  server.send(200, "text/plain", "Change Color Success");
}
void handleNotFound() {
  server.send(404, "text/plain", "Not Found");
}
void handleCSS() {
  String filePath = "/style.css";
  if (!LittleFS.exists(filePath)) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  File file = LittleFS.open(filePath, "r");
  if (!file) {
    server.send(500, "text/plain", "Failed to read file");
    return;
  }
  String content = file.readString();
  file.close();
  server.send(200, "text/css", content);
}
void handleJS() {
  String filePath = "/script.js";
  if (!LittleFS.exists(filePath)) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  File file = LittleFS.open(filePath, "r");
  if (!file) {
    server.send(500, "text/plain", "Failed to read file");
    return;
  }
  String content = file.readString();
  file.close();
  server.send(200, "application/javascript", content);
}
void handleRoot() {
  String filePath = "/index.html";
  if (!LittleFS.exists(filePath)) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  File file = LittleFS.open(filePath, "r");
  if (!file) {
    server.send(500, "text/plain", "Failed to read file");
    return;
  }
  String content = file.readString();
  file.close();
  server.send(200, "text/html", content);
}
void loop() {
  server.handleClient();
  MDNS.update();
}