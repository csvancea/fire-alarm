#include <SPI.h>
#include <SD.h>
#include <IniFile.h>

#include "src/ESP8266.h"

#define PIN_ESP8266_RX 3
#define PIN_ESP8266_TX 4

#define PIN_RGBLED_R 5
#define PIN_RGBLED_G 6
#define PIN_RGBLED_B 7

#define SLEEP_FOREVER() while(1) { SetLEDColour(255, 0, 0); delay(250); SetLEDColour(0, 0, 0); delay(100); }

ESP8266 WiFi(PIN_ESP8266_RX, PIN_ESP8266_TX);

const char WiFi_SSID[] = "SSID";
const char WiFi_Pass[] = "PASS";
const char Server_Host[] = "HOST";
unsigned short Server_Port = 7331;
const char Server_GUID[] = "GUID";

void InitWiFi()
{
	Serial.print("Waiting for Wi-Fi ...");

	/* Wait until module is communicating */
	while (!WiFi.IsInitialized()) {
		delay(1000);
	}

	Serial.println(" OK!");
}

void TestWiFiConnection()
{
	Serial.print("Testing Wi-Fi connection ...");

	if (!WiFi.ConnectToAP(WiFi_SSID, WiFi_Pass)) {
		goto fail;
	}

	if (!WiFi.StartConnection("TCP", Server_Host, Server_Port)) {
		goto fail;
	}

	if (!WiFi.Send("ARDUINO + ESP8266 = <3")) {
		goto fail;
	}

	if (!WiFi.CloseConnection()) {
		goto fail;
	}

	if (!WiFi.DisconnectFromAP()) {
		goto fail;
	}

	Serial.println(" OK!");
	return;

fail:
	Serial.println(" FAILED!");
	SLEEP_FOREVER();
}

void SetLEDColour(int r, int g, int b)
{
	analogWrite(PIN_RGBLED_R, r);
	analogWrite(PIN_RGBLED_G, g);
	analogWrite(PIN_RGBLED_B, b);
}

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
	while (!Serial) {
		delay(1000);
	}

	Serial.println("\n-------- BOOT --------\n");

	pinMode(PIN_RGBLED_R, OUTPUT);
	pinMode(PIN_RGBLED_G, OUTPUT);
	pinMode(PIN_RGBLED_B, OUTPUT);

	SetLEDColour(255, 255, 255);

	InitWiFi();
	TestWiFiConnection();

	SetLEDColour(0, 255, 0);

	Serial.println("\n-------- DONE --------\n");
}

// the loop function runs over and over again until power down or reset
void loop() {
	WiFi.IsInitialized();
	delay(5000);
}
