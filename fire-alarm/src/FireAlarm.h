#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <EasyButton.h>

#include "SettingsManager.h"
#include "NetManager.h"
#include "Led.h"
#include "FlameSensor.h"
#include "GasSensor.h"
#include "Buzzer.h"

class FireAlarm
{
public:
    FireAlarm();

    void Setup();
    void Loop();

    void OnModeSwitch(NetManager::Mode oldMode, NetManager::Mode newMode);
    
private:
    SettingsManager m_settingsManager;
    NetManager m_netManager;
    Led m_led;
    EasyButton m_configButton;
    FlameSensor m_flameSensor;
    GasSensor m_gasSensor;
    Buzzer m_buzzer;

    union {
        struct {
            unsigned int flame : 1;
            unsigned int gas   : 1;
        };
        unsigned int flags;
    } volatile m_detection;

    unsigned long m_serverLastNotificationTime;
    unsigned long m_serverLastPositiveNotificationTime;
    static constexpr unsigned long s_serverHeartbeatInterval = 60 * 1000;
    static constexpr unsigned long s_serverMinIntervalBetweenPositives = 30 * 1000;
};
