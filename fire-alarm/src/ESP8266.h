#ifndef _ESP8266_H
#define _ESP8266_H

#include "arduino.h"
#include <SoftwareSerial.h>

#define ESP8266_DEFAULT_BAUDRATE 9600
#define ESP8266_DEFAULT_COMMAND_TIMEOUT 1000
#define ESP8266_FORCE_COMMAND_ECHO 0

#define ESP8266_MAX_SSID_LEN 32
#define ESP8266_MAX_PASS_LEN 64
#define ESP8266_MAX_HOST_LEN 32

enum ESP8266_CWMODE
{
	CWMODE_STATION = 1,
	CWMODE_SOFTAP,
	CWMODE_SOFTAP_STATION
};

class ESP8266
{
public:
	ESP8266(int pinRx, int pinTx, int baudrate = ESP8266_DEFAULT_BAUDRATE);

	boolean IsInitialized();
	boolean Reset();
	boolean SetMode(ESP8266_CWMODE mode);
	boolean ConnectToAP(const char *ssid, const char *password);
	boolean ConnectToAP(const __FlashStringHelper *ssid, const __FlashStringHelper *password);
	boolean ConnectToAP(const String& ssid, const String& password);
	boolean DisconnectFromAP();
	boolean IsConnectedToAP();
	boolean StartConnection(const char *type, const char *host, int port);
	boolean StartConnection(const __FlashStringHelper *type, const __FlashStringHelper *host, int port);
	boolean StartConnection(const String& type, const String& host, int port);
	boolean CloseConnection();
	boolean Send(const char *buffer, size_t size);
	boolean Send(const char *string);
	boolean Send(const String& string);

	boolean ExecuteCommand(const char *command, const char *expectedResponse = "OK", String *response = NULL, unsigned long timeout = ESP8266_DEFAULT_COMMAND_TIMEOUT, boolean echo = false);
	boolean ExecuteCommand(const __FlashStringHelper *command, const __FlashStringHelper *expectedResponse, String *response = NULL, unsigned long timeout = ESP8266_DEFAULT_COMMAND_TIMEOUT, boolean echo = false);
	boolean ExecuteCommand(const String& command, const String& expectedResponse = "OK", String *response = NULL, unsigned long timeout = ESP8266_DEFAULT_COMMAND_TIMEOUT, boolean echo = false);

private:
	boolean ReadResponse(const char *expectedResponse = "OK", String *response = NULL, unsigned long timeout = ESP8266_DEFAULT_COMMAND_TIMEOUT, boolean echo = false, boolean flash = false);
	boolean ReadResponse(const __FlashStringHelper *expectedResponse, String *response = NULL, unsigned long timeout = ESP8266_DEFAULT_COMMAND_TIMEOUT, boolean echo = false);
	boolean ReadResponse(const String& expectedResponse = "OK", String *response = NULL, unsigned long timeout = ESP8266_DEFAULT_COMMAND_TIMEOUT, boolean echo = false);


	SoftwareSerial _serial;
};

#endif
