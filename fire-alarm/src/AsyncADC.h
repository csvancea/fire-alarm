#ifndef _ASYNCADC_H
#define _ASYNCADC_H

#include "arduino.h"

class AsyncADC {
public:
	AsyncADC(int pin);
	void SetPin(int pin);

	void Start();
	boolean HasFinished();
	int Get();

private:
	int _pin;
	int _value;
	bool _retrieved;
};

#endif