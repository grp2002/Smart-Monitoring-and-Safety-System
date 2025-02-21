#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <poll.h>

#include "gpioevent.h"

class TMP117TemperatureSensor : public GPIOPin::GPIOEventCallbackInterface {

	
	#define GPIO_CHIP "/dev/gpiochip0"  // Raspberry Pi 5 GPIO controller
	#define ALERT_PIN 17                // GPIO17 (TMP117 ALERT pin)
	#define I2C_BUS "/dev/i2c-1"         // I2C bus for TMP117
	#define TMP117_ADDR 0x48             // Default I2C address for TMP117
	#define TMP117_TEMP_REG 0x00         // Temperature register address

	virtual void hasEvent(gpiod_line_event& e) override {
	    switch (e.event_type) {
	    case GPIOD_LINE_EVENT_RISING_EDGE:
		{
		double temperature = readTemperature();
		printf("🌡️ Temperature: %d °C\n",temperature);

		printf("Rising!\n");
		break;
		}
	    case GPIOD_LINE_EVENT_FALLING_EDGE:
		printf("Falling\n");
		break;
	    default:
		printf("Unkown event\n");
	    }
	}

	// Function to read temperature from TMP117 over I2C
double readTemperature() {
    int fd = open(I2C_BUS, O_RDWR);
    if (fd < 0) {
        printf("Error: Failed to open I2C bus!\n");
        return -1;
    }

    // Set the I2C slave address
    if (ioctl(fd, I2C_SLAVE, TMP117_ADDR) < 0) {
        printf("Error: Failed to set I2C address!\n");
        close(fd);
        return -1;
    }

    // Read 2 bytes from the TMP117 temperature register
    unsigned char reg = TMP117_TEMP_REG;
    if (write(fd, &reg, 1) != 1) {
        printf("Error: Failed to write to TMP117!\n");
        close(fd);
        return -1;
    }

    unsigned char buffer[2];
    if (read(fd, buffer, 2) != 2) {
        printf("Error: Failed to read temperature data!\n");
        close(fd);
        return -1;
    }

    close(fd);

    // Convert raw data to temperature (TMP117 outputs a signed 16-bit value)
    int16_t rawTemp = (buffer[0] << 8) | buffer[1];
    return rawTemp * 0.0078125;  // Conversion formula from TMP117 datasheet
}
};

int main(int argc, char *argv[]) {
	fprintf(stderr,"Press any key to stop.\n");
	TMP117TemperatureSensor t1; //, t2;
	GPIOPin gpiopin17; //, gpiopin27;

	gpiopin17.registerCallback(&t1);
	const int gpioPinNo17 = 17;
	gpiopin17.start(gpioPinNo17);

	//gpiopin27.registerCallback(&t2);
	//const int gpioPinNo27 = 27;
	//gpiopin27.start(gpioPinNo27);

	getchar();
    gpiopin17.stop();
	//gpiopin27.stop();
	return 0;
}
