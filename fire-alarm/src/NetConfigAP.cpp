#include "NetConfigAP.h"

#include <ArduinoJson.h>

#include "SettingsManager.h"
#include "NetManager.h"

NetConfigAP::NetConfigAP(NetManager& parent, WiFiClass& wifiClass, SettingsManager& settingsManager)
    : m_netManager(parent)
    , m_wifiClass(wifiClass)
    , m_settingsManager(settingsManager)
{
    m_webServer.on("/v1/access-points", HTTPMethod::HTTP_GET, std::bind(&NetConfigAP::getAccessPoints, this));
    m_webServer.on("/v1/settings", HTTPMethod::HTTP_GET, std::bind(&NetConfigAP::getSettings, this));
    m_webServer.on("/v1/settings", HTTPMethod::HTTP_POST, std::bind(&NetConfigAP::postSettings, this));
    m_webServer.on("/v1/switch-mode", HTTPMethod::HTTP_POST, std::bind(&NetConfigAP::postSwitchMode, this));
}

bool NetConfigAP::Start()
{
    if (!m_wifiClass.mode(WIFI_MODE_APSTA)) {
        Serial.println("Could not change the WiFi mode to AP+STA");
        return false;
    }

    if (!m_wifiClass.softAP("FireAlarm_HOWL")) {
        Serial.println("Could not initialize the access point");
        Stop();
        return false;
    }

    Serial.println("SoftAP IP: " + m_wifiClass.softAPIP().toString());

    m_webServer.begin();
    return true;
}

bool NetConfigAP::Stop()
{
    m_webServer.close();

    if (!m_wifiClass.softAPdisconnect(true)) {
        Serial.println("Could not stop the access point");
        return false;
    }

    Serial.println("SoftAP stopped");
    return true;
}

bool NetConfigAP::DoLoop()
{
    m_webServer.handleClient();
    return true;
}

void NetConfigAP::getAccessPoints()
{
    printRequest();

    DynamicJsonDocument doc(4096);
    JsonArray accessPoints = doc.createNestedArray("access-points");
    String serializedJson;

    int16_t numNetworks = m_wifiClass.scanNetworks();
    for (int16_t i = 0; i < numNetworks; ++i) {
        JsonObject ap = accessPoints.createNestedObject();
        ap["ssid"] = m_wifiClass.SSID(i);
        ap["rssi"] = m_wifiClass.RSSI(i);
        ap["secure"] = (m_wifiClass.encryptionType(i) != WIFI_AUTH_OPEN);
    }
    m_wifiClass.scanDelete();

    doc.shrinkToFit();
    serializeJson(doc, serializedJson);
    if (doc.overflowed()) {
        Serial.println("JSON overflowed");
    }
    doc.clear();

    m_webServer.send(200, "application/json", serializedJson);
}

void NetConfigAP::getSettings()
{
    printRequest();

    String serializedJson;
    if (!m_settingsManager.SerializeJson(serializedJson)) {
        m_webServer.send(500);
        return;
    }

    m_webServer.send(200, "application/json", serializedJson);
}

void NetConfigAP::postSettings()
{
    printRequest();

    if (m_webServer.args() != 1) {
        m_webServer.send(400);
        return;
    }

    String json = m_webServer.arg(0);
    if (!m_settingsManager.DeserializeJson(json)) {
        m_webServer.send(500);
        return;
    }

    if (!m_settingsManager.Write()) {
        m_webServer.send(500);
        return;
    }

    m_webServer.send(200);
}

void NetConfigAP::postSwitchMode()
{
    printRequest();

    m_netManager.RequestToSwitchToSTA();
    m_webServer.send(200);
}

void NetConfigAP::printRequest()
{
    const char* method;

    switch (m_webServer.method()) {
    case HTTP_GET: method = "HTTP_GET"; break;
    case HTTP_POST: method = "HTTP_POST"; break;
    default: method = "HTTP_*";
    }

    Serial.print(m_webServer.client().remoteIP());
    Serial.print(":");
    Serial.print(m_webServer.client().remotePort());
    Serial.print(" -- ");
    Serial.print(method);
    Serial.print(" ");
    Serial.println(m_webServer.uri());
}
