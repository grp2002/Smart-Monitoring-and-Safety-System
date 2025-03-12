#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "gpioevent.h"

class TMP117TemperatureSensor : public GPIOPin::GPIOEventCallbackInterface {
	virtual void hasEvent(gpiod_line_event& e) override {
	    switch (e.event_type) {
	    case GPIOD_LINE_EVENT_RISING_EDGE:
		printf("Rising!\n");
		break;
	    case GPIOD_LINE_EVENT_FALLING_EDGE:
		printf("Falling\n");
		break;
	    default:
		printf("Unkown event\n");
	    }
	}
};

int main(int argc, char *argv[]) {
	fprintf(stderr,"Press any key to stop.\n");
	TMP117TemperatureSensor temp_Sensor1, temp_Sensor2;
	GPIOPin gpiopin17, gpiopin27;

	gpiopin17.registerCallback(&temp_Sensor1);
	const int gpioPinNo17 = 17;
	gpiopin17.start(gpioPinNo17);

	gpiopin27.registerCallback(&temp_Sensor2);
	const int gpioPinNo27 = 27;
	gpiopin27.start(gpioPinNo27);

	getchar();
        gpiopin17.stop();
	gpiopin27.stop();
	return 0;
}
