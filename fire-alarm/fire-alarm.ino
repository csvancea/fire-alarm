#include <SPI.h>
#include <SD.h>
#include <IniFile.h>

#include "src/AsyncADC.h"
#include "src/ESP8266.h"

#define GUID_LEN       36

#define PIN_ESP8266_RX 4
#define PIN_ESP8266_TX 5

#define PIN_MQ2_GAS_A  A5
#define PIN_MQ2_GAS_D  2
#define PIN_IR_FLAME   3
#define PIN_BUZZER     9

#define PIN_RGBLED_R   6
#define PIN_RGBLED_G   7
#define PIN_RGBLED_B   8

enum {
    SENSOR_FLAME,
    SENSOR_GAS
};

enum {
    ERROR_OK,
    ERROR_SDCARD,
    ERROR_WIFI,
    ERROR_SERVER,
    ERROR_COUNT
};

static const uint8_t Error_Colours[ERROR_COUNT][3] PROGMEM = {
    {0, 255, 0},    /* OK - green */
    {255, 80, 185}, /* SDCard - pink */
    {218, 0, 252},  /* Wi-Fi - purple */
    {255, 0, 0},    /* Server - red */
};

#define SLEEP_FOREVER(...) while(1) { SetLEDColour(__VA_ARGS__); delay(250); SetLEDColour(0, 0, 0); delay(100); }

ESP8266 WiFi(PIN_ESP8266_RX, PIN_ESP8266_TX);

char WiFi_SSID[ESP8266_MAX_SSID_LEN], WiFi_Pass[ESP8266_MAX_PASS_LEN];
char Server_Host[ESP8266_MAX_HOST_LEN], Server_Endpoint[32], Server_GUID[64];
unsigned short Server_Port;
unsigned long Server_LastTime;

volatile boolean Flame_Detected;
volatile boolean Gas_Detected;
volatile uint8_t Sensor_State;
int Gas_Value;

AsyncADC Gas_ADC(PIN_MQ2_GAS_A);

boolean InitSDCard()
{
    Serial.print(F("Initializing SDCard ... "));
    if (!SD.begin(PIN_SPI_SS)) {
        Serial.print(F("FAILED! "));
        Serial.println(F("(make sure you've inserted a card)"));
        return false;
    }
    Serial.println(F("OK!"));


    Serial.print(F("Reading configuration file ... "));

    IniFile config("/FIRE-A~1/NET-CO~1.INI", FILE_READ);
    if (!config.open()) {
        Serial.print(F("FAILED! "));
        Serial.println(F("(couldn't open config file: /fire-alarm/net-config.ini)"));
        return false;
    }

    if (!config.getValue("wifi", "ssid", WiFi_SSID, sizeof(WiFi_SSID))) {
        Serial.print(F("FAILED! "));
        Serial.println(F("(couldn't read Wi-Fi SSID)"));
        return false;
    }

    if (!config.getValue("wifi", "pass", WiFi_Pass, sizeof(WiFi_Pass))) {
        Serial.print(F("FAILED! "));
        Serial.println(F("(couldn't read Wi-Fi password)"));
        return false;
    }

    if (!config.getValue("server", "host", Server_Host, sizeof(Server_Host))) {
        Serial.print(F("FAILED! "));
        Serial.println(F("(couldn't read server host)"));
        return false;
    }

    if (!config.getValue("server", "port", Server_Endpoint, sizeof(Server_Endpoint))) {
        Serial.print(F("FAILED! "));
        Serial.println(F("(couldn't read server port)"));
        return false;
    }
    Server_Port = strtoul(Server_Endpoint, NULL, 10);
    if (Server_Port < 1 || Server_Port > 65535) {
        Serial.print(F("FAILED! "));
        Serial.println(F("(invalid server port)"));
        return false;
    }

    if (!config.getValue("server", "endpoint", Server_Endpoint, sizeof(Server_Endpoint))) {
        Serial.print(F("FAILED! "));
        Serial.println(F("(couldn't read server endpoint)"));
        return false;
    }

    if (!config.getValue("server", "guid", Server_GUID, sizeof(Server_GUID))) {
        Serial.print(F("FAILED! "));
        Serial.println(F("(couldn't read server GUID)"));
        return false;
    }
    if (strlen(Server_GUID) != GUID_LEN) {
        Serial.print(F("FAILED! "));
        Serial.println(F("(invalid GUID - expected " __STRINGIFY(GUID_LEN) " length string)"));
        return false;
    }

    Serial.println(F("OK!"));
    config.close();
    SD.end();
    return true;
}

boolean InitWiFi()
{
    Serial.print(F("Initializing Wi-Fi ... "));

    if (!WiFi.IsInitialized()) {
        /* Wait 5 seconds for the module to initialize */
        delay(5000);

        if (!WiFi.IsInitialized()) {
            Serial.print(F("FAILED! "));
            Serial.println(F("(ESP8266 not responding)"));
            return false;
        }
    }

    /* Reset just in case */
    if (!WiFi.Reset()) {
        Serial.print(F("FAILED! "));
        Serial.println(F("(couldn't reset)"));
        return false;
    }

    if (!WiFi.IsInitialized()) {
        /* Wait 5 seconds for the module to initialize */
        delay(5000);

        if (!WiFi.IsInitialized()) {
            Serial.print(F("FAILED! "));
            Serial.println(F("(not responding after reset)"));
            return false;
        }
    }

    if (!WiFi.ConnectToAP(WiFi_SSID, WiFi_Pass)) {
        Serial.print(F("FAILED! "));
        Serial.println(F("(couldn't connect to the access point)"));
        return false;
    }

    Serial.println(F("OK!"));
    return true;
}

boolean PostDataToServer(const String& data)
{
    Serial.print(F("Posting data to server ... "));

    if (!WiFi.IsConnectedToAP()) {
        Serial.println(F("\nLost connection to the Wi-Fi access point."));

        /* Try to reconnect to the AP */
        if (!InitWiFi()) {
            return false;
        }
    }

    if (!WiFi.Post(Server_Host, Server_Port, Server_Endpoint, data.c_str(), Server_GUID)) {
        Serial.print(F("FAILED! "));
        Serial.println(F("(POST request failed)"));
        return false;
    }

    Serial.println(F("OK!"));
    return true;
}

void SetLEDColour(int r, int g, int b)
{
    analogWrite(PIN_RGBLED_R, r);
    analogWrite(PIN_RGBLED_G, g);
    analogWrite(PIN_RGBLED_B, b);
}

void SetLEDColour(int error)
{
    if (error < 0 || error >= ERROR_COUNT) {
        SetLEDColour(255, 255, 0);
    }
    else {
        SetLEDColour(pgm_read_byte(&Error_Colours[error][0]), pgm_read_byte(&Error_Colours[error][1]), pgm_read_byte(&Error_Colours[error][2]));
    }
}

void GasInterrupt()
{
    int gasSensor = digitalRead(PIN_MQ2_GAS_D);

    /* gas sensor and buzzer are active on LOW */
    if (gasSensor == LOW) {
        if (!Gas_Detected) {
            Gas_Detected = true;

            /* start an async analog to digital conversion */
            /* must exit the ISR fast */
            Gas_ADC.Start();
        }
        Sensor_State |= (1 << SENSOR_GAS);
    }
    else {
        Sensor_State &= ~(1 << SENSOR_GAS);
    }

    digitalWrite(PIN_BUZZER, !Sensor_State);
}

void FlameInterrupt()
{
    int flameSensor = digitalRead(PIN_IR_FLAME);

    /* flame sensor and buzzer are active on LOW */
    if (flameSensor == LOW) {
        Flame_Detected = true;
        Sensor_State |= (1 << SENSOR_FLAME);
    }
    else {
        Sensor_State &= ~(1 << SENSOR_FLAME);
    }

    digitalWrite(PIN_BUZZER, !Sensor_State);
}

boolean SendNotification()
{
    char buf[50];

    sprintf_P(buf, PSTR("type=data&gas_v=%d"), Gas_Value);
    if (Gas_Detected) {
        strcat_P(buf, PSTR("&gas=1"));
    }
    if (Flame_Detected) {
        strcat_P(buf, PSTR("&flame=1"));
    }

    return PostDataToServer(buf);
}

// the setup function runs once when you press reset or power the board
void setup() {
    Serial.begin(9600);

    pinMode(PIN_RGBLED_R, OUTPUT);
    pinMode(PIN_RGBLED_G, OUTPUT);
    pinMode(PIN_RGBLED_B, OUTPUT);
    SetLEDColour(255, 255, 255);

    pinMode(PIN_BUZZER, OUTPUT);
    delay(50);
    digitalWrite(PIN_BUZZER, HIGH);

    while (!Serial) {
        delay(1000);
    }

    Serial.println(F("\n-------- BOOT --------\n"));

    /* Begin initialization code */
    attachInterrupt(digitalPinToInterrupt(PIN_MQ2_GAS_D), GasInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_IR_FLAME), FlameInterrupt, CHANGE);

    if (!InitSDCard()) {
        SLEEP_FOREVER(ERROR_SDCARD);
    }

    if (!InitWiFi()) {
        SLEEP_FOREVER(ERROR_WIFI);
    }

    if (!PostDataToServer(F("type=init&msg=ARDUINO%20%2B%20ESP8266%20%3D%20%3C3"))) {
        SLEEP_FOREVER(ERROR_SERVER);
    }

    SetLEDColour(ERROR_OK);
    Serial.println(F("\n-------- DONE --------\n"));
}

// the loop function runs over and over again until power down or reset
void loop() {
    unsigned long ms = millis();
    int sendRet = -1;

    if (Flame_Detected || Gas_Detected) {
        if (Gas_Detected) {
            /* retrieve old gas value (above the threshold at the time of detection) */
            Gas_Value = Gas_ADC.Get();
        }
        else {
            /* sensor didnt pick any smoke; retrieve current gas value */
            Gas_Value = analogRead(PIN_MQ2_GAS_A);
        }

        sendRet = SendNotification();

        /* reset detection status only if we've notified the user via Wi-Fi */
        if (sendRet) {
            Flame_Detected = false;
            Gas_Detected = false;
        }
    }
    else if (Server_LastTime == 0 || ms - Server_LastTime > 1000UL * 60) {
        /* Send heartbeat each minute */
        Gas_Value = analogRead(PIN_MQ2_GAS_A);
        sendRet = SendNotification();
        Server_LastTime = ms;
    }

    /* mark send errors */
    if (sendRet == 0) {
        SetLEDColour(ERROR_SERVER);
    }
    else if (sendRet == 1) {
        SetLEDColour(ERROR_OK);
    }

    delay(10000);
}
