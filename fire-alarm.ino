#include <SPI.h>
#include <SD.h>
#include <IniFile.h>

#include "src/ESP8266.h"

#define PIN_ESP8266_RX 3
#define PIN_ESP8266_TX 4

#define SLEEP_FOREVER() while(1) { delay(~0); }

ESP8266 WiFi(PIN_ESP8266_RX, PIN_ESP8266_TX);

String WiFi_SSID, WiFi_Pass;
String Server_Host, Server_GUID;
unsigned short Server_Port;

void InitSDCard()
{
	char buff[64];

	Serial.print("Initializing SDCard ...");
	pinMode(PIN_SPI_SS, OUTPUT);
	if (!SD.begin(PIN_SPI_SS)) {
		goto fail;
	}
	Serial.println(" OK!");

	
	Serial.print("Reading configuration file ...");

	IniFile config("/FIRE-A~1/NET-CO~1.INI", FILE_READ);
	if (!config.open()) {
		goto fail;
	}

	if (!config.getValue("wifi", "ssid", buff, sizeof(buff))) {
		goto fail;
	}
	WiFi_SSID = buff;
	
	if (!config.getValue("wifi", "pass", buff, sizeof(buff))) {
		goto fail;
	}
	WiFi_Pass = buff;

	if (!config.getValue("server", "host", buff, sizeof(buff))) {
		goto fail;
	}
	Server_Host = buff;

	if (!config.getValue("server", "port", buff, sizeof(buff))) {
		goto fail;
	}

	Server_Port = strtoul(buff, NULL, 10);
	if (!config.getValue("server", "guid", buff, sizeof(buff))) {
		goto fail;
	}
	Server_GUID = buff;

	Serial.println(" OK!");

	config.close();
	SD.end();
	return;

fail:
	Serial.println(" FAILED!");
	SLEEP_FOREVER();
}

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

// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(9600);
	while (!Serial) {
		delay(1000);
	}

	Serial.println("\n-------- BOOT --------\n");

	InitSDCard();
	InitWiFi();
	TestWiFiConnection();

	Serial.println("\n-------- DONE --------\n");
}

// the loop function runs over and over again until power down or reset
void loop() {
	WiFi.IsInitialized();
	delay(5000);
}
