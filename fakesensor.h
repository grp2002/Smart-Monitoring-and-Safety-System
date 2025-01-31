#ifndef __FAKESENSOR_H
#define __FAKESENSOR_H

#include "CppTimer.h"
#include <math.h>

class FakeSensor : public CppTimer {
    public:
	virtual void fakeSensorHasData(double inVal) = 0;
	void timerEvent() {
	    const double inVal = gain * sin( M_PI * (float)(count++) / 50.0 );
	    fakeSensorHasData(inVal);
	}
    private:
	int count = 0;
	static constexpr double gain = 7.5;
};


#endif
