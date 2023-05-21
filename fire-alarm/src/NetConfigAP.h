#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <WiFi.h>
#include <WebServer.h>

class SettingsManager;
class NetManager;

class NetConfigAP
{
public:
    NetConfigAP(NetManager& parent, WiFiClass& wifiClass, SettingsManager& settingsManager);

    bool Start();
    bool Stop();

    bool DoLoop();

private:
    void getAccessPoints();
    void getSettings();
    void postSettings();
    void postSwitchMode();

    void printRequest();

    NetManager& m_netManager;
    WiFiClass& m_wifiClass;
    SettingsManager& m_settingsManager;
    WebServer m_webServer;

    // static constexpr char s_configAPSSID[] = "FireAlarm_HOWL";
};

