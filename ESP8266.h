#ifndef _ESP8266_H
#define _ESP8266_H

#include "arduino.h"
#include <SoftwareSerial.h>

#define ESP8266_DEFAULT_BAUDRATE 9600
#define ESP8266_DEFAULT_COMMAND_TIMEOUT 1000
#define ESP8266_FORCE_COMMAND_ECHO 1

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
	boolean ConnectToAP(const String& ssid, const String& password);
	boolean DisconnectFromAP();
	boolean StartConnection(const String& type, const String& host, int port);
	boolean CloseConnection();
	boolean Send(const char *buffer, size_t size);
	boolean Send(const String& string);

	boolean ExecuteCommand(const String& command, const String& expectedResponse = "OK", String *response = NULL, unsigned long timeout = ESP8266_DEFAULT_COMMAND_TIMEOUT, boolean echo = false);

private:
	boolean ReadResponse(const String& expectedResponse = "OK", String *response = NULL, unsigned long timeout = ESP8266_DEFAULT_COMMAND_TIMEOUT, boolean echo = false);


	SoftwareSerial _serial;
};

#endif
