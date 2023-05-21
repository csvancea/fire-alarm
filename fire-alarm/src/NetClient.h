#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <WiFi.h>
#include <WebServer.h>

class SettingsManager;

class NetClient
{
public:
    NetClient(WiFiClass& wifiClass, SettingsManager& settingsManager);

    bool Start();
    bool Stop();

    bool Post(String& data);

private:
    WiFiClass& m_wifiClass;
    SettingsManager& m_settingsManager;

    // static constexpr char s_configAPSSID[] = "FireAlarm_HOWL";
};

