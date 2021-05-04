#include "ESP8266.h"

ESP8266::ESP8266(int pinRx, int pinTx, int baudrate) : _serial(pinRx, pinTx)
{
    _serial.begin(baudrate);
}

boolean ESP8266::IsInitialized()
{
    return ExecuteCommand(F("AT"), F("OK"));
}

boolean ESP8266::Reset()
{
    if (!ExecuteCommand(F("AT+RST"), F("OK"))) {
        return false;
    }

    /* make sure we're getting "ready" message back */
    return ReadResponse(F("ready"), NULL, 5000);
}

boolean ESP8266::SetMode(ESP8266_CWMODE mode)
{
    char command[16];

    sprintf_P(command, PSTR("AT+CWMODE_CUR=%d"), mode);
    return ExecuteCommand(command);
}

boolean ESP8266::ConnectToAP(const char *ssid, const char *password)
{
    char command[ESP8266_MAX_SSID_LEN + ESP8266_MAX_PASS_LEN + 20];

    sprintf_P(command, PSTR("AT+CWJAP=\"%s\",\"%s\""), ssid, password);

    /* Establishing a connection might take a while */
    return ExecuteCommand(command, "OK", NULL, 10000);
}

boolean ESP8266::ConnectToAP(const __FlashStringHelper *ssid, const __FlashStringHelper *password)
{
    char command[ESP8266_MAX_SSID_LEN + ESP8266_MAX_PASS_LEN + 20];

    sprintf_P(command, PSTR("AT+CWJAP=\"%S\",\"%S\""), ssid, password);

    /* Establishing a connection might take a while */
    return ExecuteCommand(command, "OK", NULL, 10000);
}

boolean ESP8266::DisconnectFromAP()
{
    return ExecuteCommand(F("AT+CWQAP"), F("WIFI DISCONNECT"), NULL, 5000);
}

boolean ESP8266::IsConnectedToAP()
{
    String response;

    if (!ExecuteCommand(F("AT+CWJAP?"), F("OK"), &response)) {
        return false;
    }

    return response.indexOf(F("\r\nNo AP\r\n")) == -1;
}

boolean ESP8266::StartConnection(const char *type, const char *host, int port)
{
    char command[ESP8266_MAX_HOST_LEN + 32];

    sprintf_P(command, PSTR("AT+CIPSTART=\"%s\",\"%s\",%d"), type, host, port);
    return ExecuteCommand(command, "OK", NULL, 5000);
}

boolean ESP8266::StartConnection(const __FlashStringHelper *type, const __FlashStringHelper *host, int port)
{
    char command[ESP8266_MAX_HOST_LEN + 32];

    sprintf_P(command, PSTR("AT+CIPSTART=\"%S\",\"%S\",%d"), type, host, port);
    return ExecuteCommand(command, "OK", NULL, 5000);
}

boolean ESP8266::StartConnection(const String& type, const String& host, int port)
{
    return StartConnection(type.c_str(), host.c_str(), port);
}

boolean ESP8266::CloseConnection()
{
    return ExecuteCommand(F("AT+CIPCLOSE"), F("OK"));
}

boolean ESP8266::Send(const char* buffer, size_t size, boolean flash)
{
    char command[24];

    sprintf_P(command, PSTR("AT+CIPSEND=%u"), size);
    if (!ExecuteCommand(command, "OK", NULL, 10000)) {
        return false;
    }

    if (flash) {
        for (; size != 0; --size, ++buffer) {
            _serial.write(pgm_read_byte(buffer));
        }
    }
    else {
        _serial.write(buffer, size);
    }

    return ReadResponse(F("SEND OK"), NULL, 10000, ESP8266_FORCE_COMMAND_ECHO);
}

boolean ESP8266::Send(const char *string)
{
    return Send(string, strlen(string));
}

boolean ESP8266::Send(const __FlashStringHelper *string)
{
    return Send(reinterpret_cast<const char*>(string), strlen_P(reinterpret_cast<const char *>(string)), true);
}

boolean ESP8266::Send(const String& string)
{
    return Send(string.c_str(), string.length());
}

boolean ESP8266::Post(const char *host, int port, const char *endpoint, const char *data, const char *cookies)
{
    boolean ret = true;
    unsigned long timeout;
    char buf[ESP8266_MAX_HOST_LEN + 32];
    
    buf[sizeof(buf) - 1] = 0;

    if (!StartConnection(F("TCP"), host, port)) {
        return false;
    }

    snprintf_P(buf, sizeof(buf) - 1, PSTR("POST %s HTTP/1.0\r\n"), endpoint);
    if (!Send(buf)) {
        ret = false;
        goto close;
    }

    snprintf_P(buf, sizeof(buf) - 1, PSTR("Host: %s\r\n"), host);
    if (!Send(buf)) {
        ret = false;
        goto close;
    }

    if (cookies) {
        snprintf_P(buf, sizeof(buf) - 1, PSTR("Cookie: %s\r\n"), cookies);
        if (!Send(buf)) {
            ret = false;
            goto close;
        }
    }

    if (!Send(F("Content-Type: application/x-www-form-urlencoded\r\n"))) {
        ret = false;
        goto close;
    }

    snprintf_P(buf, sizeof(buf) - 1, PSTR("Content-Length: %d\r\n\r\n"), strlen(data));
    if (!Send(buf)) {
        ret = false;
        goto close;
    }
    
    if (!Send(data)) {
        ret = false;
        goto close;
    }

    /* assume everything went fine if we get to this point */
    timeout = _serial.getTimeout();
    _serial.readBytesUntil('\n', buf, sizeof(buf));
    while (_serial.available()) {
        _serial.read();
    }
    _serial.setTimeout(timeout);

close:
    CloseConnection();
    return ret;
}

boolean ESP8266::ExecuteCommand(const char *command, const char *expectedResponse, String *response, unsigned long timeout, boolean echo)
{
    /* discard any leftover */
    while (_serial.available()) {
        _serial.read();
    }

    _serial.println(command);
    return ReadResponse(expectedResponse, response, timeout, ESP8266_FORCE_COMMAND_ECHO || echo);
}

boolean ESP8266::ExecuteCommand(const __FlashStringHelper *command, const __FlashStringHelper *expectedResponse, String* response, unsigned long timeout, boolean echo)
{
    /* discard any leftover */
    while (_serial.available()) {
        _serial.read();
    }

    _serial.println(command);
    return ReadResponse(expectedResponse, response, timeout, ESP8266_FORCE_COMMAND_ECHO || echo);
}

boolean ESP8266::ExecuteCommand(const String& command, const String& expectedResponse, String* response, unsigned long timeout, boolean echo)
{
    return ExecuteCommand(command.c_str(), expectedResponse.c_str(), response, timeout, echo);
}

boolean ESP8266::ReadResponse(const char *expectedResponse, String *response, unsigned long timeout, boolean echo, boolean flash)
{
    unsigned long oldTimeout = _serial.getTimeout();
    unsigned long waitUntil = millis() + timeout;
    boolean status;
    String output;
    int (*cmp)(const char*, const char*);

    if (flash) {
        cmp = strcmp_P;
    }
    else {
        cmp = strcmp;
    }

    _serial.setTimeout(timeout);
    while (1) {
        output = _serial.readStringUntil('\n');
        output.trim();

        if (echo) {
            Serial.println(output);
        }

        if (response) {
            *response += output + F("\r\n");
        }

        if (cmp(output.c_str(), expectedResponse) == 0) {
            /* successful command */
            status = true;
            break;
        }
        if (strcmp_P(output.c_str(), PSTR("FAIL")) == 0 || strcmp_P(output.c_str(), PSTR("ERROR")) == 0) {
            /* failed command */
            status = false;
            break;
        }
        if (waitUntil < millis()) {
            /* no response */
            status = false;

            if (echo) {
                Serial.println(F("TIMEOUT"));
            }

            if (response) {
                *response += F("TIMEOUT");
                *response += F("\r\n");
            }
            break;
        }
    }
    _serial.setTimeout(oldTimeout);

    if (response) {
        response->trim();
    }

    /* discard any leftover */
    while (_serial.available()) {
        _serial.read();
    }

    return status;
}

boolean ESP8266::ReadResponse(const __FlashStringHelper *expectedResponse, String* response, unsigned long timeout, boolean echo)
{
    return ReadResponse(reinterpret_cast<const char *>(expectedResponse), response, timeout, echo, true);
}

boolean ESP8266::ReadResponse(const String& expectedResponse, String* response, unsigned long timeout, boolean echo)
{
    return ReadResponse(expectedResponse.c_str(), response, timeout, echo);
}
