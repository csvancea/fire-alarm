#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "NetConfigAP.h"
#include "NetClient.h"

class SettingsManager;
class WiFiClass;

class NetManager
{
public:
    enum class Mode {
        NONE,
        AP,
        STA
    };
    using SwitchModeCallback_t = std::function<void(Mode, Mode)>;

    NetManager(WiFiClass& wifiClass, SettingsManager& settingsManager);

    bool SetMode(NetManager::Mode mode);
    NetManager::Mode GetMode() const;
    void OnModeSwitch(SwitchModeCallback_t callback);

    bool Post(String& data);
    void RequestToSwitchToSTA();

    bool DoLoop();

private:
    bool m_requestToSwitchToSTA;
    Mode m_mode;
    SwitchModeCallback_t m_switchModeCallback;

    NetConfigAP m_netConfigAP;
    NetClient m_netClient;
};

