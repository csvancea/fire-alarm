#include "FlameSensor.h"

FlameSensor::FlameSensor(int pinDigital)
    : m_pinDigital(pinDigital)
{
    pinMode(m_pinDigital, INPUT);
}

FlameSensor::~FlameSensor()
{
    detachInterrupt(digitalPinToInterrupt(m_pinDigital));
}

bool IRAM_ATTR FlameSensor::IsFlameDetected() const
{
    return digitalRead(m_pinDigital) == LOW;
}

void FlameSensor::OnFlameStateChange(ChangeStateCallback_t callback)
{
    if (callback) {
        m_callback = callback;
        attachInterruptArg(digitalPinToInterrupt(m_pinDigital), OnInterrupt, this, CHANGE);
    }
}

void IRAM_ATTR FlameSensor::OnInterrupt()
{
    if (m_callback) {
        m_callback(IsFlameDetected());
    }
}

void IRAM_ATTR FlameSensor::OnInterrupt(void* arg)
{
    reinterpret_cast<FlameSensor*>(arg)->OnInterrupt();
}
