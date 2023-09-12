#include <Arduino.h>
#include <BlynkSimpleEsp8266.h>

#include "DHT.h"
DHT dht(D6, DHT11);
	dht.begin();
	float h = dht.readHumidity();    //Đọc độ ẩm
  	float t = dht.readTemperature(); //Đọc nhiệt độ
	Blynk.virtualWrite(V1, 20.6f); 
	Blynk.virtualWrite(V2, 3.3f); 


BlynkTimer timer; 
void updateDHT11() 
{
  Blynk.virtualWrite(V5, 12);  
}
	timer.setInterval(1000L, updateDHT11); 
	timer.run(); 

#define BLYNK_TEMPLATE_ID "TMPL6Uv4MeKX_"
#define BLYNK_TEMPLATE_NAME "Test Blynk"
#define BLYNK_AUTH_TOKEN "YOUR_TOKEN"

const char *ssid = "TP-Link_0D5E";
const char *password = "93853767";

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
}

BLYNK_CONNECTED() {
  Blynk.sendInternal("meta", "get", "Seri");
}
BLYNK_WRITE(InternalPinMETA) {
    String field = param[0].asStr();
    if (field == "Seri") {
        String value = param[1].asStr();
        Serial.println(value);
        // Do something with Metadata value
    }
}
Blynk.sendInternal("meta", "set", "Seri", "follow toi nha");
BLYNK_WRITE(V0)
{
  int value = param.asInt(); // Lấy giá trị được gửi đến
  Serial.println("Gia tri thay doi: " + String(value));
}
void loop() {
  Blynk.run();
}
BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V0);
}

Blynk.logEvent("evt_dht11", String("High Temperature Detected!: ") + String(temp));

	float _humi[2] = {90.5, 92.1};
	float _temp[2] = {23.7, 25.3};
	static int c = 0;
	Blynk.virtualWrite(V1, _temp[c]);
	Blynk.virtualWrite(V2, _humi[c]);
	c++;
	if (c>=2) c = 0;