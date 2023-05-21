#include "NetClient.h"

#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "SettingsManager.h"

NetClient::NetClient(WiFiClass& wifiClass, SettingsManager& settingsManager)
    : m_wifiClass(wifiClass)
    , m_settingsManager(settingsManager)
{
}

bool NetClient::Start()
{
    if (!m_wifiClass.mode(WIFI_MODE_STA)) {
        Serial.println("Could not change the WiFi mode to STA");
        return false;
    }

    const auto& wifiSettings = m_settingsManager.Get().wifi;
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(wifiSettings.ssid);

    if (!m_wifiClass.begin(wifiSettings.ssid, wifiSettings.pass)) {
        Serial.println("Could not begin the connection phase");
        Stop();
        return false;
    }
    
    int connectResult = m_wifiClass.waitForConnectResult();
    if (connectResult != WL_CONNECTED) {
        Serial.print("Failed to connect to WiFi: ");
        Serial.println(connectResult);
        Stop();
        return false;
    }

    if (!m_wifiClass.setAutoReconnect(true)) {
        Serial.println("Could not enable auto reconnect");
        Stop();
        return false;
    }

    Serial.println("STA IP: " + m_wifiClass.localIP().toString());
    return true;
}

bool NetClient::Stop()
{
    if (!m_wifiClass.setAutoReconnect(false)) {
        Serial.println("Could not disable auto reconnect");
        // return false;
    }

    if (!m_wifiClass.disconnect(true, true)) {
        Serial.println("Could not disconnect from the access point");
        return false;
    }

    return true;
}

bool NetClient::Post(String& data)
{
    WiFiClientSecure wifiClient;
    HTTPClient httpClient;
    const auto& serverSettings = m_settingsManager.Get().server;

    if (!m_wifiClass.isConnected()) {
        Serial.print("Lost the WiFi connection. Trying to reconnect ... ");

        m_wifiClass.reconnect();
        if (m_wifiClass.waitForConnectResult() != WL_CONNECTED) {
            Serial.println("FAILED!");
            return false;
        }

        Serial.println("SUCCESSFUL!");
    }

    wifiClient.setCACert(serverSettings.root_certificate);
    if (!httpClient.begin(wifiClient, serverSettings.url)) {
        Serial.println("Can't connect to the HTTP(s) server");
        return false;
    }

    httpClient.addHeader("Content-Type", "application/json");

    int httpCode = httpClient.POST(data);
    if (httpCode != HTTP_CODE_OK) {
        Serial.print("Can't post data to the HTTP(s) server: ");
        Serial.println(httpCode);
        Serial.println(httpClient.getString());
        return false;
    }

    return true;
}
