#include "NetManager.h"

NetManager::NetManager(WiFiClass& wifiClass, SettingsManager& settingsManager)
    : m_requestToSwitchToSTA(false)
    , m_mode(Mode::NONE)
    , m_netConfigAP(*this, wifiClass, settingsManager)
    , m_netClient(wifiClass, settingsManager)
{
}

bool NetManager::SetMode(NetManager::Mode mode)
{
    if (mode == GetMode())
    {
        Serial.println("Same mode. No switching needed!");
        return true;
    }

    Mode oldMode = GetMode();
    switch (oldMode) {
    case Mode::AP:
        m_netConfigAP.Stop();
        break;

    case Mode::STA:
        m_netClient.Stop();
        break;
    }

    bool turned = true;
    switch (mode) {
    case Mode::AP:
        turned = m_netConfigAP.Start();
        break;

    case Mode::STA:
        turned = m_netClient.Start();
        break;
    }

    if (!turned) {
        mode = Mode::NONE;
    }

    m_mode = mode;

    if (m_switchModeCallback) {
        m_switchModeCallback(oldMode, m_mode);
    }

    return turned;
}

NetManager::Mode NetManager::GetMode() const
{
    return m_mode;
}

void NetManager::OnModeSwitch(SwitchModeCallback_t callback)
{
    m_switchModeCallback = callback;
}

bool NetManager::Post(String& data)
{
    if (GetMode() != Mode::STA) {
        Serial.println("Wrong NetManager mode");
        return false;
    }

    return m_netClient.Post(data);
}

void NetManager::RequestToSwitchToSTA()
{
    m_requestToSwitchToSTA = true;
}

bool NetManager::DoLoop()
{
    switch (GetMode()) {
    case Mode::AP:
        if (m_requestToSwitchToSTA) {
            m_requestToSwitchToSTA = false;
            return SetMode(Mode::STA);
        }

        return m_netConfigAP.DoLoop();
    }

    return true;
}
