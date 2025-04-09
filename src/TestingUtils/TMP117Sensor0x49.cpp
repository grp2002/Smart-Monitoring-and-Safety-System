#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <error.h>
#include <string.h>
#include <cmath>
#include <iostream>

#include "gpioevent.h"
#include <mutex>

// Mutex to protect the use of single I2C bus shared by multiple sensors
static std::mutex i2c_mutex; 

class TMP117TemperatureSensor : public GPIOPin::GPIOEventCallbackInterface {

	
	#define GPIO_CHIP "/dev/gpiochip0"  // Raspberry Pi 5 GPIO controller
	//#define ALERT_PIN 17                // GPIO17 (TMP117 ALERT pin)
	#define I2C_BUS "/dev/i2c-1"         // I2C bus for TMP117
	#define TMP117_ADDR1 0x48             // Default I2C address for TMP117
	#define TMP117_ADDR2 0x49             // Default I2C address for TMP117
	#define TMP117_TEMP_REG 0x00         // Temperature register address

	virtual void hasEvent(gpiod_line_event& e) override {

		system("sudo ntpdate time.google.com > /dev/null 2>&1");
		auto now = std::chrono::system_clock::now();
    	std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
		std::cout << std::ctime(&currentTime);
		printf("Sensor %d : interrupt received!\n", sensor_id);

	    switch (e.event_type) {
	    case GPIOD_LINE_EVENT_RISING_EDGE:
		{
		double temperature = readTemperature();

		if (std::isnan(temperature)) {
			printf("Sensor %d : Failed to read temperature.\n", sensor_id);
		} else {
			printf("Sensor %d : 🌡️ Temperature: %.2f °C\n", sensor_id, temperature);
		}

		printf("Sensor %d : Rising!\n", sensor_id);
		break;
		}
	    case GPIOD_LINE_EVENT_FALLING_EDGE:
		printf("Sensor %d : Falling\n", sensor_id);
		break;
	    default:
		printf("Unkown event\n");
	    }
	}
	/**
	 * Function to read temperature from TMP117 over I2C
	 */
	double readTemperature() {
		printf("Sensor %d waiting for I2C lock.\n", sensor_id);
		std::lock_guard<std::mutex> lock(i2c_mutex);
	/*
	* Step#1: Opening the I2C Bus Device File:
	* --------------------------------
	* The I2C bus on Raspberry Pi is exposed as a device file (typically /dev/i2c-1).
	* To communicate with I2C devices like TMP117, we need to open this file.
	* 
	* Function:
	*     int fd = open(const char *pathname, int flags);
	* 
	* Parameters:
	*     - pathname: The path to the I2C bus device file (e.g., "/dev/i2c-1").
	*     - flags: Access mode for the file. We use O_RDWR (read/write access) because
	*              we need to both write to and read from the I2C device. 
	* 				Other possible flag values are:
	* 				O_RDONLY	Open for reading only
	*				O_WRONLY	Open for writing only
	*				O_NONBLOCK	Non-blocking mode
	*				O_SYNC		Writes complete synchronously
	* 
	* 
	* Return Value:
	*     - On success: Returns a non-negative integer file descriptor (fd).
	*     - On failure: Returns -1, and sets errno to indicate the specific error.
	* 
	* Usage in I2C:
	*     - The returned file descriptor is used with ioctl() to specify the target I2C device address.
	*     - Then, read() and write() system calls are used to communicate with the device.
	*     - Always close(fd) when done to free the resource.
	*/
    int fd = open(I2C_BUS, O_RDWR);
    if (fd < 0) {
        printf("Error: Failed to open I2C bus!\n");
        return NAN;
    }

	/**
	* Step#2: Setting the I2C Slave Address:
	* ---------------------------------
	* Function:
	*     int ioctl(int fd, unsigned long request, int arg);
	* 
	* Purpose:
	*     Configures which I2C slave device we are targeting (TMP117 address 0x48).
	* 
	* Parameters:
	*     - fd: File descriptor returned by open().
	*     - request: I2C_SLAVE → Tells the kernel to set the target slave address.
	*     - arg: The 7-bit slave address (0x48 for TMP117 default).
	* 
	* Returns:
	*     - 0 on success.
	*     - -1 on failure; errno set accordingly.
	* 
	* Example:
	*     if (ioctl(fd, I2C_SLAVE, 0x48) < 0) {
	*         // Handle error
	*     }
	*/
	/**
	if (ioctl(fd, I2C_SLAVE, TMP117_ADDR2) < 0) {
		printf("Error: Failed to set I2C address! %s\n", strerror(errno));
		close(fd);
		return NAN;
	}
	*/
	if (sensor_id == 1){
		if (ioctl(fd, I2C_SLAVE, TMP117_ADDR1) < 0) {
			printf("Error: Failed to set I2C address! %s\n", strerror(errno));
			close(fd);
			return NAN;
		}
	}
	else if (sensor_id == 2){
		if (ioctl(fd, I2C_SLAVE, TMP117_ADDR2) < 0) {
			printf("Error: Failed to set I2C address! %s\n", strerror(errno));
			close(fd);
			return NAN;
		}
	}
	/**
	 * recommended to add a short delay after interrupt to ensure the TMP117 data is stable
	 */
	usleep(2000); //2000 was giving Error: Failed to write to TMP117! Remote I/O error
	
	/**
	*
	* step#3. Writing register address to the I2C Device:
	* -----------------------------
	* Function:
	*     ssize_t write(int fd, const void *buf, size_t count);
	* 
	* Purpose:
	*     Sends data to the I2C slave device.
	*     Typically used to send the register address you want to read from or write to.
	* 
	* Parameters:
	*     - fd: File descriptor.
	*     - buf: Pointer to data buffer (e.g., register address).
	*     - count: Number of bytes to write (usually 1 byte for register).
	* 
	* Returns:
	*     - Number of bytes written on success.
	*     - -1 on failure; errno set accordingly.
	* 
	* Example:
	*     unsigned char reg = 0x00; // Temperature register
	*     if (write(fd, &reg, 1) != 1) {
	*         // Handle error
	*     }
	* 
	*/
    unsigned char reg = TMP117_TEMP_REG;
    if (write(fd, &reg, 1) != 1) {
        printf("Error: Failed to write to TMP117! %s\n", strerror(errno));
        close(fd);
        return NAN;
    }
	/**
	*
	* step#4. Reading value from the I2C Device:
	* -------------------------------
	* Function:
	*     ssize_t read(int fd, void *buf, size_t count);
	* 
	* Purpose:
	*     Reads data from the I2C slave device.
	* 
	* Parameters:
	*     - fd: File descriptor.
	*     - buf: Buffer to store received data.
	*     - count: Number of bytes to read (2 bytes for TMP117 temperature).
	* 
	* Returns:
	*     - Number of bytes read on success.
	*     - -1 on failure; errno set accordingly.
	* 
	* Example:
	*     unsigned char buffer[2];
	*     if (read(fd, buffer, 2) != 2) {
	*         // Handle error
	*     }
	* 
	*/
    unsigned char buffer[2];
    if (read(fd, buffer, 2) != 2) {
        printf("Error: Failed to read temperature data!\n");
        close(fd);
        return NAN;
    }
	/**
	* 
	* step#5. Closing the I2C Bus:
	* -----------------------
	* Function:
	*     int close(int fd);
	* 
	* Purpose:
	*     Closes the I2C bus file descriptor to release system resources.
	* 
	* Example:
	*     close(fd);
	* 
	*/
    close(fd);

	/**
	 * Convert raw data to temperature in degree celcius 
	 * TMP117 generates a signed 16-bit value as a raw output
	 */
    int16_t rawTemp = (buffer[0] << 8) | buffer[1];
    return rawTemp * 0.0078125;  // Conversion formula from TMP117 datasheet
	}
	
	public:

	TMP117TemperatureSensor(int sensor_id){
		this->sensor_id = sensor_id;
	}
	private:
	int sensor_id;
	
};

int main(int argc, char *argv[]) {
	fprintf(stderr,"Press any key to stop.\n");
	//TMP117TemperatureSensor t1, t2;
	
	TMP117TemperatureSensor *t1, *t2;
	//t1 = new TMP117TemperatureSensor(1);
	t2 = new TMP117TemperatureSensor(2);
	
	GPIOPin gpiopin27;//, gpiopin17;

	//gpiopin17.registerCallback(t1);
	//const int gpioPinNo17 = 17;
	
	gpiopin27.registerCallback(t2);
	const int gpioPinNo27 = 27;

	gpiopin27.start(gpioPinNo27);
	//gpiopin17.start(gpioPinNo17);


	getchar();
    //gpiopin17.stop();
	gpiopin27.stop();
	return 0;
}
