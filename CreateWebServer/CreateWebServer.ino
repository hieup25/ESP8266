#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char *ssid = "TP-Link_0D5E";
const char *password = "93853767";

// Khởi tạo server với port là 80
ESP8266WebServer server(80);

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
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
}

void handleNotFound() {
  server.send(404, "text/plain", "Not Found");
}

// Khi browser request đến ESP8266 với nội dung
// http://myserver.local hoặc
// http://{localIP_Esp8266}:80, hàm này sẽ xử lý request này
void handleRoot() {
  // Làm bất cứ thứ gì trong hàm này
  // Bật tắt LED, bật tắt thiết bị trong nhà cho ứng dụng Smart Home...
  String res = R"(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Page Title</title>
    </head>
    <body>
      <h1>Hello HieuP25</h1>
    </body>
    </html>
  )";
  server.send(200, "text/html", res);
}
void loop() {
  server.handleClient();
  MDNS.update();
}
