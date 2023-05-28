#include "Led.h"

Led::Led(int pin_r, int pin_g, int pin_b)
    : m_pinR(pin_r)
    , m_pinG(pin_g)
    , m_pinB(pin_b)
    , m_colourR(0)
    , m_colourG(0)
    , m_colourB(0)
    , m_blinking(false)
    , m_blinkingState(false)
    , m_lastBlink(0)
{
    pinMode(pin_r, OUTPUT);
    pinMode(pin_g, OUTPUT);
    pinMode(pin_b, OUTPUT);
}

void Led::SetColour(int r, int g, int b)
{
    m_colourR = r;
    m_colourG = g;
    m_colourB = b;

    if (!m_blinking) {
        SetColourNoSave(m_colourR, m_colourG, m_colourB);
    }
}

void Led::Blink(bool enabled)
{
    m_blinking = enabled;

    if (!m_blinking) {
        SetColourNoSave(m_colourR, m_colourG, m_colourB);
    }
}

void Led::DoLoop()
{
    if (m_blinking) {
        unsigned long currentMs = millis();

        if (currentMs - m_lastBlink >= s_blinkInterval) {
            m_lastBlink = currentMs;

            m_blinkingState = !m_blinkingState;

            if (m_blinkingState) {
                SetColourNoSave(m_colourR, m_colourG, m_colourB);
            }
            else {
                SetColourNoSave(0, 0, 0);
            }
        }

    }
}

void Led::SetColourNoSave(int r, int g, int b)
{
    analogWrite(m_pinR, r);
    analogWrite(m_pinG, g);
    analogWrite(m_pinB, b);
}
