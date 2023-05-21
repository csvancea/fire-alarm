#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <ArduinoJson.h>
#include <FS.h>

class SettingsManager
{
public:
    struct WiFi
    {
    private:
        static constexpr size_t s_maxSSIDLen = 32 + 1;
        static constexpr size_t s_maxPassLen = 64;

    public:
        char ssid[s_maxSSIDLen];
        char pass[s_maxPassLen];
    };

    struct Server
    {
    private:
        static constexpr size_t s_maxURLLen = 128;
        static constexpr size_t s_maxGUIDLen = 36 + 1;
        static constexpr size_t s_maxCertLen = 2048;

    public:
        char url[s_maxURLLen];
        char guid[s_maxGUIDLen];
        char root_certificate[s_maxCertLen];
    };

    struct Settings
    {
        WiFi wifi;
        Server server;
    };

    explicit SettingsManager(fs::FS& fileSystem);
    
    bool Load();
    bool Write() const;
    
    template <typename TSource>
    bool DeserializeJson(TSource&& src) {
        DynamicJsonDocument doc(s_settingsJSONMaxAlloc);
        DeserializationError error = deserializeJson(doc, std::forward<TSource>(src));
        if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            return false;
        }

        if (doc.overflowed()) {
            Serial.println("Deserializing json settings failed due to overflowing");
            return false;
        }

        if (!doc.is<Settings>()) {
            Serial.println("Deserializing json settings failed due to bad schema");
            return false;
        }

        m_settings = doc.as<Settings>();
        return true;
    }

    template <typename TDest>
    bool SerializeJson(TDest&& dest) const
    {
        DynamicJsonDocument doc(s_settingsJSONMaxAlloc);
        doc.set(m_settings);

        if (doc.overflowed()) {
            Serial.println("Serializing json settings failed due to overflowing");
            return false;
        }

        serializeJson(doc, std::forward<TDest>(dest));
        return true;
    }

    const Settings& Get() const;

private:
    fs::FS& m_fileSystem;
    Settings m_settings;

    // static constexpr char s_settingsFileName[] = "/settings.json";
    static constexpr size_t s_settingsJSONMaxAlloc = 3072;
};

inline void convertToJson(const SettingsManager::WiFi& src, JsonVariant dst)
{
    dst["ssid"] = src.ssid;
    dst["pass"] = src.pass;
}

inline void convertFromJson(JsonVariantConst src, SettingsManager::WiFi& dst)
{
    strlcpy(dst.ssid, src["ssid"], sizeof(dst.ssid));
    strlcpy(dst.pass, src["pass"], sizeof(dst.pass));
}

inline bool canConvertFromJson(JsonVariantConst src, const SettingsManager::WiFi&)
{
    return src.containsKey("ssid") && src.containsKey("pass") && src["ssid"].is<const char*>() && src["pass"].is<const char*>();
}

inline void convertToJson(const SettingsManager::Server& src, JsonVariant dst)
{
    dst["url"] = src.url;
    dst["guid"] = src.guid;
    dst["root_certificate"] = src.root_certificate;
}

inline void convertFromJson(JsonVariantConst src, SettingsManager::Server& dst)
{
    strlcpy(dst.url, src["url"], sizeof(dst.url));
    strlcpy(dst.guid, src["guid"], sizeof(dst.guid));
    strlcpy(dst.root_certificate, src["root_certificate"], sizeof(dst.root_certificate));
}

inline bool canConvertFromJson(JsonVariantConst src, const SettingsManager::Server&)
{
    return src.containsKey("url") && src.containsKey("guid") && src.containsKey("root_certificate") && src["url"].is<const char*>() && src["guid"].is<const char*>() && src["root_certificate"].is<const char*>();
}

inline void convertToJson(const SettingsManager::Settings& src, JsonVariant dst)
{
    dst["wifi"] = src.wifi;
    dst["server"] = src.server;
}

inline void convertFromJson(JsonVariantConst src, SettingsManager::Settings& dst)
{
    dst.wifi = src["wifi"];
    dst.server = src["server"];
}

inline bool canConvertFromJson(JsonVariantConst src, const SettingsManager::Settings&)
{
    return src.containsKey("wifi") && src.containsKey("server") && src["wifi"].is<SettingsManager::WiFi>() && src["server"].is<SettingsManager::Server>();
}
