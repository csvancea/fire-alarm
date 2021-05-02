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

char WiFi_SSID[ESP8266_MAX_SSID_LEN], WiFi_Pass[ESP8266_MAX_PASS_LEN];
char Server_Host[ESP8266_MAX_HOST_LEN], Server_GUID[42];
unsigned short Server_Port;
unsigned long Server_LastTime;

volatile boolean Flame_Detected;
boolean Gas_Detected;
int Gas_Value;

boolean InitSDCard()
{
	Serial.print(F("Initializing SDCard ..."));
	if (!SD.begin(PIN_SPI_SS)) {
		Serial.print(F(" FAILED!"));
		Serial.println(F(" -- make sure you've inserted a card"));
		return false;
	}
	Serial.println(F(" OK!"));


	Serial.print(F("Reading configuration file ..."));

	IniFile config("/FIRE-A~1/NET-CO~1.INI", FILE_READ);
	if (!config.open()) {
		Serial.print(F(" FAILED!"));
		Serial.println(F(" -- couldn't open config file: /fire-alarm/net-config.ini"));
		return false;
	}

	if (!config.getValue("wifi", "ssid", WiFi_SSID, sizeof(WiFi_SSID))) {
		Serial.print(F(" FAILED!"));
		Serial.println(F(" -- couldn't read Wi-Fi SSID"));
		return false;
	}

	if (!config.getValue("wifi", "pass", WiFi_Pass, sizeof(WiFi_Pass))) {
		Serial.print(F(" FAILED!"));
		Serial.println(F(" -- couldn't read Wi-Fi password"));
		return false;
	}

	if (!config.getValue("server", "host", Server_Host, sizeof(Server_Host))) {
		Serial.print(F(" FAILED!"));
		Serial.println(F(" -- couldn't read server host"));
		return false;
	}

	if (!config.getValue("server", "port", Server_GUID, sizeof(Server_GUID))) {
		Serial.print(F(" FAILED!"));
		Serial.println(F(" -- couldn't read server port"));
		return false;
	}
	Server_Port = strtoul(Server_GUID, NULL, 10);
	if (Server_Port < 1 || Server_Port > 65535) {
		Serial.print(F(" FAILED!"));
		Serial.println(F(" -- invalid server port"));
		return false;
	}

	if (!config.getValue("server", "guid", Server_GUID, sizeof(Server_GUID))) {
		Serial.print(F(" FAILED!"));
		Serial.println(F(" -- couldn't read server GUID"));
		return false;
	}

	Serial.println(F(" OK!"));
	config.close();
	SD.end();
	return true;
}

boolean InitWiFi()
{
	Serial.print(F("Initializing Wi-Fi ..."));

	if (!WiFi.IsInitialized()) {
		/* Wait 5 seconds for the module to initialize */
		delay(5000);

		if (!WiFi.IsInitialized()) {
			Serial.print(F(" FAILED!"));
			Serial.println(F(" -- not responding"));
			return false;
		}
	}

	/* Reset just in case */
	if (!WiFi.Reset()) {
		Serial.print(F(" FAILED!"));
		Serial.println(F(" -- couldn't reset"));
		return false;
	}

	if (!WiFi.IsInitialized()) {
		/* Wait 5 seconds for the module to initialize */
		delay(5000);

		if (!WiFi.IsInitialized()) {
			Serial.print(F(" FAILED!"));
			Serial.println(F(" -- not responding after reset"));
			return false;
		}
	}

	if (!WiFi.ConnectToAP(WiFi_SSID, WiFi_Pass)) {
		Serial.print(F(" FAILED!"));
		Serial.println(F(" -- couldn't connect to the access point"));
		return false;
	}

	Serial.println(F(" OK!"));
	return true;
}

boolean TestServerConnection()
{
	Serial.print(F("Testing Server connection ..."));

	if (!WiFi.StartConnection(F("TCP"), Server_Host, Server_Port)) {
		Serial.print(F(" FAILED!"));
		Serial.println(F(" -- couldn't establish a TCP connection"));
		return false;
	}

	if (!WiFi.Send(F("ARDUINO + ESP8266 = <3"))) {
		Serial.print(F(" FAILED!"));
		Serial.println(F(" -- couldn't send the test message"));
		return false;
	}

	if (!WiFi.CloseConnection()) {
		Serial.print(F(" FAILED!"));
		Serial.println(F(" -- couldn't close the connection"));
		return false;
	}

	Serial.println(F(" OK!"));
	return true;
}

void SetLEDColour(int r, int g, int b)
{
	analogWrite(PIN_RGBLED_R, r);
	analogWrite(PIN_RGBLED_G, g);
	analogWrite(PIN_RGBLED_B, b);
}

void ReadGasSensor()
{
	Gas_Value = analogRead(PIN_MQ2_GAS);

	if (Gas_Value > 500) {
		Gas_Detected = true;
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

	if (!WiFi.StartConnection(F("TCP"), Server_Host, Server_Port)) {
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

	Serial.println(F("\n-------- BOOT --------\n"));

	pinMode(PIN_RGBLED_R, OUTPUT);
	pinMode(PIN_RGBLED_G, OUTPUT);
	pinMode(PIN_RGBLED_B, OUTPUT);

	SetLEDColour(255, 255, 255);

	if (!InitSDCard()) {
		SLEEP_FOREVER();
	}

	if (!InitWiFi()) {
		SLEEP_FOREVER();
	}

	if (!TestServerConnection()) {
		SLEEP_FOREVER();
	}

	attachInterrupt(digitalPinToInterrupt(PIN_IR_FLAME), FlameInterrupt, FALLING);
	SetLEDColour(0, 255, 0);

	Serial.println(F("\n-------- DONE --------\n"));
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
	}
	else if (Server_LastTime == 0 || ms - Server_LastTime > 1000UL * 60 * 10) {
		/* Send heartbeat each 10 mins */
		SendNotification();
		Server_LastTime = ms;
	}
	delay(10000);
}
