#include "ESP8266.h"

ESP8266::ESP8266(int pinRx, int pinTx, int baudrate) : _serial(pinRx, pinTx)
{
	_serial.begin(baudrate);
}

boolean ESP8266::IsInitialized()
{
	return ExecuteCommand("AT");
}

boolean ESP8266::Reset()
{
	if (!ExecuteCommand("AT+RST")) {
		return false;
	}

	/* make sure we're getting "ready" message back */
	return ReadResponse(NULL, 5000);
}

boolean ESP8266::SetMode(ESP8266_CWMODE mode)
{
	return ExecuteCommand(String("AT+CWMODE_CUR=") + mode);
}

boolean ESP8266::ConnectToAP(const String& ssid, const String& password)
{
	String command;

	command = "AT+CWJAP_CUR=";
	command += "\"" + ssid + "\",";
	command += "\"" + password + "\"";

	/* Establishing a connection might take a while */
	return ExecuteCommand(command, NULL, 10000);
}

boolean ESP8266::DisconnectFromAP()
{
	return ExecuteCommand("AT+CWQAP", NULL, 5000);
}

boolean ESP8266::ExecuteCommand(const String& command, String *response, unsigned long timeout, boolean echo)
{
	/* discard any leftover */
	while (_serial.available()) {
		_serial.read();
	}

	_serial.println(command);
	return ReadResponse(response, timeout, ESP8266_FORCE_COMMAND_ECHO || echo);
}

boolean ESP8266::ReadResponse(String *response, unsigned long timeout, boolean echo)
{
	unsigned long oldTimeout = _serial.getTimeout();
	unsigned long waitUntil = millis() + timeout;
	boolean status;
	String output;

	_serial.setTimeout(timeout);
	while (1) {
		output = _serial.readStringUntil('\n');
		output.trim();

		if (echo) {
			Serial.println(output);
		}

		if (response) {
			*response += output + "\r\n";
		}

		if (output == "OK" || output == "ready") {
			/* successful command */
			status = true;
			break;
		}
		if (output == "FAIL" || output == "ERROR") {
			/* failed command */
			status = false;
			break;
		}
		if (waitUntil < millis()) {
			/* no response */
			status = false;
			
			if (echo) {
				Serial.println("TIMEOUT");
			}

			if (response) {
				*response += "TIMEOUT\r\n";
			}
			break;
		}
	}
	_serial.setTimeout(oldTimeout);

	if (response) {
		response->trim();
	}

	return status;
}
