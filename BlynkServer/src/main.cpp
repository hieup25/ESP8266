#include <Arduino.h>
#include <BlynkSimpleEsp8266.h>
#include "DHT.h"

#define BLYNK_TEMPLATE_ID "TMPL6PfaCPrYe"
#define BLYNK_TEMPLATE_NAME "Test Blynk"
#define BLYNK_AUTH_TOKEN "t7ssPa-KbVBzIKlZsBAz_uww0j8nAnet"

const char *ssid = "TP-Link_0D5E";
const char *password = "93853767";
float humi;
float temp;

DHT dht(D6, DHT11);
BlynkTimer timer;

void updateDHT11() {
	Blynk.virtualWrite(V1, temp);
	Blynk.virtualWrite(V2, humi);
	if (temp > 20) {
		Blynk.logEvent("evt_dht11", String("High Temperature Detected!: ") + String(temp));
	}
}
void setup() {
	pinMode(D7, OUTPUT);
	dht.begin();
	Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
	timer.setInterval(30000L, updateDHT11);
}
BLYNK_CONNECTED() {
	Blynk.syncVirtual(V0);
}
BLYNK_WRITE(V0) {
	int value = param.asInt();
	digitalWrite(D7, value);
}
void loop() {
	humi = dht.readHumidity();
	temp = dht.readTemperature();
  	Blynk.run();
	timer.run();
}