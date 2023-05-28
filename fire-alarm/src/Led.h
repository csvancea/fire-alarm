#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class Led
{
public:
    Led(int pin_r, int pin_g, int pin_b);

    bool Turn(bool enabled);
    void SetColour(int r, int g, int b);
    void Blink(bool enabled);

    void DoLoop();
    
private:
    void SetColourNoSave(int r, int g, int b);

    int m_pinR;
    int m_pinG;
    int m_pinB;

    int m_colourR;
    int m_colourG;
    int m_colourB;

    bool m_blinking;
    bool m_blinkingState;
    unsigned long m_lastBlink;

    static constexpr unsigned long s_blinkInterval = 500;
};
