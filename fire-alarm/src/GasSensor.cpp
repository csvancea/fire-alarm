#include "GasSensor.h"

GasSensor::GasSensor(int pinDigital, int pinAnalog)
    : m_pinDigital(pinDigital)
    , m_pinAnalog(pinAnalog)
{
    pinMode(m_pinDigital, INPUT);
    pinMode(m_pinAnalog, INPUT);
}

GasSensor::~GasSensor()
{
    detachInterrupt(digitalPinToInterrupt(m_pinDigital));
}

bool IRAM_ATTR GasSensor::IsGasDetected() const
{
    return digitalRead(m_pinDigital) == LOW;
}

int GasSensor::GetGasValue() const
{
    return analogRead(m_pinAnalog);
}

void GasSensor::OnGasStateChange(ChangeStateCallback_t callback)
{
    if (callback) {
        m_callback = callback;
        attachInterruptArg(digitalPinToInterrupt(m_pinDigital), OnInterrupt, this, CHANGE);
    }
}

void IRAM_ATTR GasSensor::OnInterrupt()
{
    if (m_callback) {
        m_callback(IsGasDetected());
    }
}

void IRAM_ATTR GasSensor::OnInterrupt(void* arg)
{
    reinterpret_cast<GasSensor*>(arg)->OnInterrupt();
}
