#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

class FlameSensor
{
public:
    using ChangeStateCallback_t = std::function<void(bool)>;

    FlameSensor(int pinDigital);
    ~FlameSensor();

    FlameSensor(const FlameSensor&) = delete;
    FlameSensor(FlameSensor&&) = delete;
    void operator=(const FlameSensor&) = delete;
    void operator=(FlameSensor&&) = delete;

    bool IsFlameDetected() const;
    void OnFlameStateChange(ChangeStateCallback_t callback);

private:
    void OnInterrupt();
    static void OnInterrupt(void* arg);

    int m_pinDigital;
    ChangeStateCallback_t m_callback;
};
