extern "C" {
	#include <i2c/smbus.h>
  }

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
//#include <i2c/smbus.h>
#include <poll.h>
#include <error.h>
#include <string.h>
#include <cmath>

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
		printf("\n %s", std::ctime(&currentTime));
		printf(" Sensor %d : interrupt received!\n", sensor_id);

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
		{
			printf("Sensor %d : Falling\n", sensor_id);
			double temperature = readTemperature();

			if (std::isnan(temperature)) {
				printf("Sensor %d : Failed to read temperature.\n", sensor_id);
			} else {
				printf("Sensor %d : 🌡️ Temperature: %.2f °C\n", sensor_id, temperature);
			}
			
			break;
		}
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
	//usleep(5000); //2000 was giving Error: Failed to write to TMP117! Remote I/O error
	
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

   //****READ OPERATION IS NOT RESETTING THE ALERT PIN THEREFORE USING smbus.h */

 
    unsigned char reg = TMP117_TEMP_REG;
    if (write(fd, &reg, 1) != 1) {
        printf("Error: Failed to write to TMP117! %s\n", strerror(errno));
        close(fd);
        return NAN;
    }

	usleep(1000);
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

		//DEBUG START
		printf("Sensor %d ALERT should be cleared after this read.\n", sensor_id);
	
		//DEBUG END 
    return rawTemp * 0.0078125;  // Conversion formula from TMP117 datasheet

/*
	// Read temperature using smbus protocol
	int raw = i2c_smbus_read_word_data(fd, TMP117_TEMP_REG);
    close(fd);

    if (raw < 0) {
        fprintf(stderr, "Failed to read temperature via smbus.\n");
        return NAN;
    }

    // TMP117 gives big-endian, smbus returns little-endian — swap
    raw = ((raw & 0xFF) << 8) | ((raw >> 8) & 0xFF);

    double tempC = raw * 0.0078125;

    printf("Sensor %d ALERT should now be cleared.\n", sensor_id);
    return tempC;
*/
	}
	
	
	int i2c_smbus_read_word_data(int file, uint8_t command) {
		union i2c_smbus_data data;
		struct i2c_smbus_ioctl_data args;
	
		args.read_write = I2C_SMBUS_READ;
		args.command = command;
		args.size = I2C_SMBUS_WORD_DATA;
		args.data = &data;
	
		if (ioctl(file, I2C_SMBUS, &args) < 0)
			return -1;
	
		return 0x0FFFF & data.word;
	}

	public:

	TMP117TemperatureSensor(int sensor_id){
		this->sensor_id = sensor_id;
	}

	void readAndPrintStartupTemperature() {
		double temp = readTemperature();
		if (!std::isnan(temp)) {
			printf("Sensor %d : 🌡️ Startup Temperature: %.2f °C\n", sensor_id, temp);
		} else {
			printf("Sensor %d : Failed to read temperature at startup.\n", sensor_id);
		}
	}
	
	//To set the TMP117 into CR mode is a feature of the TMP117 temperature sensor where the ALERT pin (INT) is used to signal when a new temperature conversion is ready.
	  
	void initialize() {
		std::lock_guard<std::mutex> lock(i2c_mutex);
	
		int fd = open(I2C_BUS, O_RDWR);
		if (fd < 0) {
			perror("I2C open failed\n");
			return;
		}
	
		int addr = (sensor_id == 1) ? TMP117_ADDR1 : TMP117_ADDR2;
		if (ioctl(fd, I2C_SLAVE, addr) < 0) {
			perror("I2C address set failed\n");
			close(fd);
			return;
		}
	
		usleep(200000); // Allow TMP117 to boot
	
		// 1. Read Device ID
		unsigned char reg = 0x0F;
		unsigned char id_buf[2];
		if (write(fd, &reg, 1) != 1 || read(fd, id_buf, 2) != 2) {
			perror("Device ID read failed\n");
			close(fd);
			return;
		}
	
		uint16_t device_id = (id_buf[0] << 8) | id_buf[1];
		printf("Sensor %d: Detected TMP117 (ID: 0x%04X)\n", sensor_id, device_id);
	
		// 2. Dummy temperature read
		reg = 0x00;
		unsigned char dummy[2];
		if (write(fd, &reg, 1) == 1 && read(fd, dummy, 2) == 2) {
			printf("Sensor %d: Dummy temp read: 0x%02X 0x%02X\n", sensor_id, dummy[0], dummy[1]);
		} else {
			perror("Dummy temperature read failed\n");
		}
	
		usleep(5000);
	
		// 3. Set T_HIGH = 28°C, T_LOW = 20°C (safe bracket around ~25°C)
		uint16_t high_raw = static_cast<uint16_t>(28.0 / 0.0078125);
		uint16_t low_raw  = static_cast<uint16_t>(20.0 / 0.0078125);
		i2c_smbus_write_word_data(fd, 0x02, high_raw);
		printf("Sensor %d: T_HIGH set to ~28.0°C\n", sensor_id);
		i2c_smbus_write_word_data(fd, 0x03, low_raw);
		printf("Sensor %d: T_LOW set to ~20.0°C\n", sensor_id);
	
		// 4. Shutdown mode before CR mode write
		uint16_t shutdown_config = 0x0820;
		unsigned char buf_shutdown[3] = {
			0x01,
			static_cast<unsigned char>((shutdown_config >> 8) & 0xFF),
			static_cast<unsigned char>(shutdown_config & 0xFF)
		};
		write(fd, buf_shutdown, 3);
		usleep(50000);
	
		// 5. Read temperature to clear ALERT
		reg = 0x00;
		unsigned char temp_buf[2];
		write(fd, &reg, 1);
		read(fd, temp_buf, 2);  // Discarded, just to clear ALERT
		usleep(5000);
	
		// 6. Attempt CR config write with retries
		uint16_t cr_config = 0x6062;
		unsigned char buf_cr[3] = {
			0x01,
			static_cast<unsigned char>((cr_config >> 8) & 0xFF),
			static_cast<unsigned char>(cr_config & 0xFF)
		};
	
		bool success = false;
		int attempts = 0;
		const int max_attempts = 10;
	
		while (!success && attempts < max_attempts) {
			if (write(fd, buf_cr, 3) != 3) {
				perror("Failed to write CR config\n");
				break;
			}
	
			usleep(100000); // wait 100ms
	
			reg = 0x01;
			unsigned char cfg[2];
			if (write(fd, &reg, 1) != 1 || read(fd, cfg, 2) != 2) {
				perror("Failed to read config after write\n");
				break;
			}
	
			uint16_t config = (cfg[0] << 8) | cfg[1];
			printf("Sensor %d attempt %d: config reg = 0x%04X\n", sensor_id, attempts + 1, config);
	
			if (config == cr_config) {
				printf("✅ Sensor %d: CR mode successfully set after %d attempt(s).\n", sensor_id, attempts + 1);
				success = true;
			}
	
			attempts++;
		}
	
		// 7. Fallback if CR mode not accepted
		if (!success) {
			fprintf(stderr, "⚠️ Sensor %d: Failed to set CR mode after %d attempts. Rewriting thresholds to fallback (20°C).\n",
					sensor_id, max_attempts);
	
			uint16_t fallback_raw = static_cast<uint16_t>(20.0 / 0.0078125);
			i2c_smbus_write_word_data(fd, 0x02, fallback_raw);
			i2c_smbus_write_word_data(fd, 0x03, fallback_raw);
			printf("Sensor %d: T_HIGH and T_LOW reset to 20.0°C\n", sensor_id);
		}
	
		close(fd);
	}

	private:
	int sensor_id;
	
};

int main(int argc, char *argv[]) {
	fprintf(stderr,"Press any key to stop.\n");
	//TMP117TemperatureSensor t1, t2;
	
	TMP117TemperatureSensor *t1, *t2;
	t1 = new TMP117TemperatureSensor(1);
	t2 = new TMP117TemperatureSensor(2);

	//t1->initialize();
	//t2->initialize();

	
	GPIOPin gpiopin17, gpiopin27;

	gpiopin17.registerCallback(t1);
	const int gpioPinNo17 = 17;
	
	gpiopin27.registerCallback(t2);
	const int gpioPinNo27 = 27;

	gpiopin27.start(gpioPinNo27);
	gpiopin17.start(gpioPinNo17);

	// Force a startup temperature read for both sensors
	t1->readAndPrintStartupTemperature();
	t2->readAndPrintStartupTemperature();

// Just for debugging, run for 10 seconds in main():
/*for (int i = 0; i < 10; ++i) {
    int val17 = gpiod_line_get_value(gpiopin17.getLine());
    int val27 = gpiod_line_get_value(gpiopin27.getLine());
    printf("DEBUG: GPIO17=%d, GPIO27=%d\n", val17, val27);
    sleep(1);
}*/
	
	getchar();
    gpiopin17.stop();
	gpiopin27.stop();
	return 0;
}
