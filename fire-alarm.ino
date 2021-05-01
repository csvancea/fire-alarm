#include <SPI.h>
#include <SD.h>
#include <IniFile.h>

#include "src/ESP8266.h"

#define PIN_ESP8266_RX 3
#define PIN_ESP8266_TX 4

#define PIN_MQ2_GAS    A5
#define PIN_IR_FLAME   2
#define PIN_BUZZER     9

#define PIN_RGBLED_R   5
#define PIN_RGBLED_G   6
#define PIN_RGBLED_B   7

#define SLEEP_FOREVER() while(1) { SetLEDColour(255, 0, 0); delay(250); SetLEDColour(0, 0, 0); delay(100); }

ESP8266 WiFi(PIN_ESP8266_RX, PIN_ESP8266_TX);

char WiFi_SSID[32], WiFi_Pass[64];
char Server_Host[32], Server_GUID[42];
unsigned short Server_Port;

void InitSDCard()
{
	Serial.print("Initializing SDCard ...");
	if (!SD.begin(PIN_SPI_SS)) {
		goto fail;
	}
	Serial.println(" OK!");


	Serial.print("Reading configuration file ...");

	IniFile config("/FIRE-A~1/NET-CO~1.INI", FILE_READ);
	if (!config.open()) {
		goto fail;
	}

	if (!config.getValue("wifi", "ssid", WiFi_SSID, sizeof(WiFi_SSID))) {
		goto fail;
	}

	if (!config.getValue("wifi", "pass", WiFi_Pass, sizeof(WiFi_Pass))) {
		goto fail;
	}

	if (!config.getValue("server", "host", Server_Host, sizeof(Server_Host))) {
		goto fail;
	}

	if (!config.getValue("server", "port", Server_GUID, sizeof(Server_GUID))) {
		goto fail;
	}
	Server_Port = strtoul(Server_GUID, NULL, 10);

	if (!config.getValue("server", "guid", Server_GUID, sizeof(Server_GUID))) {
		goto fail;
	}

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

	while (!WiFi.Reset()) {
		delay(1000);
	}

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

void ReadSensors()
{
	int analogValue = analogRead(PIN_MQ2_GAS);
	int flame = digitalRead(PIN_IR_FLAME);

	Serial.print("Gas: ");
	Serial.print(analogValue);
	if (flame == LOW)
		Serial.print(" | FLAME");
	if (analogValue > 350)
		Serial.print(" | THRES");
	Serial.println("");
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

	InitSDCard();
	InitWiFi();
	TestWiFiConnection();

	SetLEDColour(0, 255, 0);

	Serial.println("\n-------- DONE --------\n");
}

// the loop function runs over and over again until power down or reset
void loop() {
	WiFi.IsInitialized();
	ReadSensors();
	delay(5000);
}
