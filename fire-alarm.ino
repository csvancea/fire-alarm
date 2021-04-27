#include "ESP8266.h"

ESP8266 WiFi(3, 2);

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
	Serial.println("BOOT");

	delay(2000);

	while (!WiFi.IsInitialized()) {
		delay(1000);
	}

	while (!WiFi.Reset()) {
		delay(1000);
	}

	while (!WiFi.IsInitialized()) {
		delay(1000);
	}

	while (!WiFi.SetMode(CWMODE_STATION)) {
		delay(1000);
	}

	while (!WiFi.ConnectToAP("SSID", "PASSWORD")) {
		delay(1000);
	}

	while (!WiFi.DisconnectFromAP()) {
		delay(1000);
	}

	Serial.println("WiFi Test OK!");
}

// the loop function runs over and over again until power down or reset
void loop() {
	WiFi.IsInitialized();
	delay(5000);
}
