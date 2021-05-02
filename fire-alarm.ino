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
unsigned long Server_LastTime;

volatile boolean Flame_Detected;
boolean Gas_Detected;
int Gas_Value;

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

	if (!WiFi.IsInitialized()) {
		/* Wait 5 seconds for the module to initialize */
		delay(5000);

		if (!WiFi.IsInitialized()) {
			goto fail;
		}
	}

	/* Reset just in case */
	if (!WiFi.Reset()) {
		goto fail;
	}

	if (!WiFi.IsInitialized()) {
		/* Wait 5 seconds for the module to initialize */
		delay(5000);

		if (!WiFi.IsInitialized()) {
			goto fail;
		}
	}

	if (!WiFi.ConnectToAP(WiFi_SSID, WiFi_Pass)) {
		goto fail;
	}

	Serial.println(" OK!");
	return;

fail:
	Serial.println(" FAIL!");
	SLEEP_FOREVER();
}

void TestServerConnection()
{
	Serial.print("Testing Server connection ...");

	if (!WiFi.StartConnection("TCP", Server_Host, Server_Port)) {
		goto fail;
	}

	if (!WiFi.Send("ARDUINO + ESP8266 = <3")) {
		goto fail;
	}

	if (!WiFi.CloseConnection()) {
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

void ReadGasSensor()
{
	int analogValue = analogRead(PIN_MQ2_GAS);
	
	if (analogValue > 350) {
		Gas_Detected = true;
		Gas_Value = analogValue;
	}
}

void FlameInterrupt()
{
	Flame_Detected = true;
}

boolean SendNotification()
{
	/* Flame detected interval: 0-1    = 1 bit of data */
	/* Gas detected interval:   0-1    = 1 bit of data */
	/* Gas sensor interval:     0-1023 = 10 bits of data */

	/* VVVVVVVVVVGF <- LSB */
	int encoded = (Gas_Value & 1023) << 2 | (Gas_Detected & 1) << 1 | (Flame_Detected & 1);

	if (!WiFi.StartConnection("TCP", Server_Host, Server_Port)) {
		return 0;
	}

	/* send as hexa string */
	if (!WiFi.Send(String(encoded, HEX))) {
		WiFi.CloseConnection();
		return 0;
	}

	WiFi.CloseConnection();
	return 1;
}

void Alarm()
{

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
	TestServerConnection();

	attachInterrupt(digitalPinToInterrupt(PIN_IR_FLAME), FlameInterrupt, FALLING);
	SetLEDColour(0, 255, 0);

	Serial.println("\n-------- DONE --------\n");
}

// the loop function runs over and over again until power down or reset
void loop() {
	unsigned long ms = millis();

	ReadGasSensor();

	if (Flame_Detected || Gas_Detected) {
		Alarm();
		SendNotification();
		Flame_Detected = false;
		Gas_Detected = false;

		Serial.println(__LINE__);
	}
	else if (ms - Server_LastTime > 1000 * 60 * 10) {
		/* Send heartbeat each 10 mins */
		SendNotification();
		Server_LastTime = ms;
	}
	delay(10000);
}
