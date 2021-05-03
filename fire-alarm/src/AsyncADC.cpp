#include "AsyncADC.h"

AsyncADC::AsyncADC(int pin)
{
	SetPin(pin);
}

void AsyncADC::SetPin(int pin)
{
	if (pin >= 14) {
		pin -= 14;
	}

	/* 3 valid bits for MUX */
	_pin = pin & 7;
}

void AsyncADC::Start()
{	
	_retrieved = false;

	/* Reference = 5V | MUX = pin */
	ADMUX = (1 << REFS0) | _pin;
	ADCSRA |= (1 << ADSC);

	/* Arduino enables ADC on startup (ADEN is set to 1) */
}

boolean AsyncADC::HasFinished()
{
	return _retrieved || (ADCSRA & (1 << ADSC)) == 0;
}

int AsyncADC::Get()
{
	if (!_retrieved) {
		int low = ADCL;
		int high = ADCH;

		while (!HasFinished()) {
			delay(10);
		}

		_value = (high << 8) | low;
		_retrieved = true;
	}

	return _value;
}
