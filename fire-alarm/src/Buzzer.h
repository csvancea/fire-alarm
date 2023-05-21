#pragma once

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class Buzzer
{
public:
    Buzzer(int pin);

    void SetState(bool state);
    bool GetState() const;
    
private:
    int m_pin;
    bool m_state;
};
