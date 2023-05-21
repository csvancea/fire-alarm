#include "SettingsManager.h"

SettingsManager::SettingsManager(fs::FS& fileSystem)
    : m_fileSystem(fileSystem)
{
}

bool SettingsManager::Load()
{
    File file = m_fileSystem.open("/settings.json");
    if (!file || file.isDirectory()) {
        Serial.println("Cannot open settings file for reading");
        return false;
    }

    return DeserializeJson(file);
}

bool SettingsManager::Write() const
{
    File file = m_fileSystem.open("/settings.json", FILE_WRITE);
    if (!file) {
        Serial.println("Cannot open settings file for writing");
        return false;
    }

    return SerializeJson(file);
}

const SettingsManager::Settings& SettingsManager::Get() const
{
    return m_settings;
}
