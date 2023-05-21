#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class GasSensor
{
public:
    using ChangeStateCallback_t = std::function<void(bool)>;

    GasSensor(int pinDigital, int pinAnalog);
    ~GasSensor();

    GasSensor(const GasSensor&) = delete;
    GasSensor(GasSensor&&) = delete;
    void operator=(const GasSensor&) = delete;
    void operator=(GasSensor&&) = delete;

    bool IsGasDetected() const;
    int GetGasValue() const;
    void OnGasStateChange(ChangeStateCallback_t callback);
    
private:
    void OnInterrupt();
    static void OnInterrupt(void* arg);

    int m_pinDigital;
    int m_pinAnalog;
    ChangeStateCallback_t m_callback;
};
