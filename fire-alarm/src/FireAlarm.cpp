#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPIFFS.h>

#include "FireAlarm.h"
#include "Pins.h"

FireAlarm::FireAlarm()
    : m_settingsManager(SPIFFS)
    , m_netManager(WiFi, m_settingsManager)
    , m_led(PIN_RGBLED_R, PIN_RGBLED_G, PIN_RGBLED_B)
    , m_configButton(PIN_BUTTON)
    , m_flameSensor(PIN_IR_FLAME)
    , m_gasSensor(PIN_MQ2_GAS_D, PIN_MQ2_GAS_A)
    , m_buzzer(PIN_BUZZER)
    , m_detection()
{
    m_detection.flags = 0;
}

void FireAlarm::Setup()
{
    using namespace std::placeholders;
    Serial.begin(115200);

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed");
        return;
    }

    // m_led.Blink(true);
    m_configButton.begin();
    m_netManager.OnModeSwitch(std::bind(&FireAlarm::OnModeSwitch, this, _1, _2));

    m_flameSensor.OnFlameStateChange(
        [this](bool detected) {
            m_detection.flame = detected;
            m_buzzer.SetState(m_detection.flags);
        }
    );

    m_gasSensor.OnGasStateChange(
        [this](bool detected) {
            m_detection.gas = detected;
            m_buzzer.SetState(m_detection.flags);
        }
    );

    NetManager::Mode startMode = NetManager::Mode::STA;
    if (!m_settingsManager.Load()) {
        Serial.println("Failed to load the settings");
        startMode = NetManager::Mode::AP;
    }

    if (m_configButton.read()) {
        startMode = NetManager::Mode::AP;
    }

    m_netManager.SetMode(startMode);
}

void FireAlarm::Loop()
{
    unsigned long ms = millis();

    m_detection.flame = m_flameSensor.IsFlameDetected();
    m_detection.gas = m_gasSensor.IsGasDetected();
    
    m_buzzer.SetState(m_detection.flags);

    if (m_detection.flags || m_serverLastNotificationTime == 0 || ms - m_serverLastNotificationTime > s_serverHeartbeatInterval)
    {
        int gasValue = m_gasSensor.GetGasValue();

        if (m_netManager.GetMode() == NetManager::Mode::NONE) {
            m_netManager.SetMode(NetManager::Mode::STA);
        }

        if (m_netManager.GetMode() == NetManager::Mode::STA) {
            StaticJsonDocument<128> doc;
            String serializedJson;

            doc["guid"] = m_settingsManager.Get().server.guid;
            doc["gas_value"] = gasValue;
            doc["gas_detected"] = static_cast<bool>(m_detection.gas);
            doc["flame_detected"] = static_cast<bool>(m_detection.flame);

            serializeJson(doc, serializedJson);

            if (m_netManager.Post(serializedJson)) {
                m_serverLastNotificationTime = ms;
                Serial.println("POST'ed measurements");
            }
            else {
                m_netManager.SetMode(NetManager::Mode::NONE);
            }
        }
    }

    m_configButton.read();
    m_led.DoLoop();
    m_netManager.DoLoop();
}

void FireAlarm::OnModeSwitch(NetManager::Mode /*oldMode*/, NetManager::Mode newMode)
{
    switch (newMode) {
    case NetManager::Mode::NONE:
        Serial.println("Entered disabled mode");
        m_led.SetColour(255, 0, 0);
        break;
    case NetManager::Mode::AP:
        Serial.println("Entered configuration mode");
        m_led.SetColour(0, 255, 255);
        break;
    case NetManager::Mode::STA:
        Serial.println("Entered normal mode");
        m_led.SetColour(0, 255, 0);
        break;
    }
}
