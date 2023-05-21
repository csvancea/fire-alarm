#include "Buzzer.h"

Buzzer::Buzzer(int pin)
    : m_pin(pin)
    , m_state(false)
{
    pinMode(m_pin, OUTPUT);
    digitalWrite(m_pin, HIGH);
}

void Buzzer::SetState(bool state)
{
    if (m_state == state)
    {
        return;
    }

    m_state = state;
    digitalWrite(m_pin, !m_state);
}

bool Buzzer::GetState() const
{
    return m_state;
}
