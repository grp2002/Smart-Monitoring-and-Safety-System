extern "C" {
	#include <i2c/smbus.h>
  }
#include <cstdlib> // for system()
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
#include "buzzer.h"
#include <mutex>


// Mutex to protect the shared I2C bus in a multi-threaded environment
static std::mutex i2c_mutex; 


class TMP117TemperatureSensor : public GPIOPin::GPIOEventCallbackInterface {

	/**
	 * GPIO_CHIP : Raspberry Pi 5 GPIO controller
	 * useful commands: 
	 * "gpiodetect" : list all the GPIO chips on the board and the total number of GPIO lines they have at kernel level
	 * "gpioinfo gpiochipX" : {X: 0, 1..} list all the lines and their used / unused status along with the Process name using that Pin.
	 */
	#define GPIO_CHIP "/dev/gpiochip0"
	/**
	 * I2C_BUS : I2C bus on Raspberry Pi 5
	 * useful commands:
	 * "i2cdetect -l" : lists all I2C adapters
	 * "i2cdetect -y 1" : to see which devices are connected to a I2C bus 1 (i2c-1), which is a default I2C bus on Raspberry Pi 5.
	 */
	#define I2C_BUS "/dev/i2c-1"

	#define TMP117_ADDR1 0x48	 //I2C address of TMP117 Sensor where ADDR Pin is connected to GND.
	#define TMP117_ADDR2 0x49    //I2C address of TMP117 Sensor where ADDR Pin is connected to VIN (3.3V).
	/**
	 * TMP117_TEMP_REG : Register address to hold the current temperature value in signed 16-bit format.
	 * useful commands:
	 * "i2cget -y 1 0x48 0x00 w" : to read word (w) from register 0x00 of device at 0x48 on bus 1.
	 * "i2cset -y 1 0x48 0x01 0x03E4 w" : to write 0x03E4 to TMP117 config register 0x01 to update settings.
	 * Refer TMP117 Datasheet
	 */
	#define TMP117_TEMP_REG 0x00

	//GPIO Event Handler function
	virtual void hasEvent(gpiod_line_event& e) override {

		/*Update the Raspberry Pi system date...*/
		system("sudo ntpdate time.google.com > /dev/null 2>&1");
		auto now = std::chrono::system_clock::now();
    	std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
		printf("\n[ %s ] :: [TMP117TemperatureSensor::hasEvent() {%d}] : interrupt received!\n", std::ctime(&currentTime), sensor_id);
		
		/*Read the Temperature value when DATA READY INTERRUPT Pin is Low*/
	    switch (e.event_type) {
			case GPIOD_LINE_EVENT_RISING_EDGE:
			{	/**
				* TMP117 sends the rising event after the 0x00 or 0x01 register has been read. 
				*/

				printf("[TMP117TemperatureSensor::hasEvent() {%d}] : Rising!\n", sensor_id);
				break;
			}
			case GPIOD_LINE_EVENT_FALLING_EDGE:
			{	/**
				 * TMP117 sends the falling event after temperature conversion result is ready in the
				 * 0x00 register.
				 */
				printf("[TMP117TemperatureSensor::hasEvent() {%d}] : Falling\n", sensor_id);
				double temperature = readTemperature();

				if (std::isnan(temperature)) { //Error handling: Invalid value
					printf("[TMP117TemperatureSensor::hasEvent() {%d}] : Failed to read temperature.\n", sensor_id);
				} else {
					printf("[TMP117TemperatureSensor::hasEvent() {%d}] : 🌡️ Temperature: %.2f °C\n", sensor_id, temperature);
					
					/**
					 * [ALERT!] Send the Buzzer beep alert when Temperature has crossed the threshold range
					 */
					if (temperature > HIGH_THRESHOLD || temperature < LOW_THRESHOLD) {
						printf("⚠️ [ALERT!] [TMP117TemperatureSensor::hasEvent() {%d}] :Trigger the buzzer beep to alert! \n", sensor_id);
						buzzer->on();
						sleep(1);
						buzzer->off();
					}
				}
				
				break;
			}
			default:
			printf("[TMP117TemperatureSensor::hasEvent() {%d}] : Unkown event\n", sensor_id);
		}
	}
	/**
	 * Function to read temperature from TMP117 over I2C
	 */
	double readTemperature() {
		printf("[TMP117TemperatureSensor::readTemperature() {%d}] : waiting for I2C lock.\n", sensor_id);
		std::lock_guard<std::mutex> lock(i2c_mutex); //acquire the lock to use I2C bus
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
        printf("[TMP117TemperatureSensor::readTemperature() {%d}] : Error : Failed to open I2C bus!\n", sensor_id);
        return NAN;
    }

	/**
	* Step#2: Setting the I2C Slave Address:
	* ---------------------------------
	* Function:
	*     int ioctl(int fd, unsigned long request, int arg);
	* 
	* Purpose:
	*     Configures which I2C slave device we are targeting (For example: TMP117 address 0x48).
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
	if (sensor_id == 1){
		if (ioctl(fd, I2C_SLAVE, TMP117_ADDR1) < 0) {
			printf("[TMP117TemperatureSensor::readTemperature() {%d}] : Error : Failed to set I2C address! %s\n", sensor_id, strerror(errno));
			close(fd);
			return NAN;
		}
	}
	else if (sensor_id == 2){
		if (ioctl(fd, I2C_SLAVE, TMP117_ADDR2) < 0) {
			printf("[TMP117TemperatureSensor::readTemperature() {%d}] : Error : Failed to set I2C address! %s\n", sensor_id, strerror(errno));
			close(fd);
			return NAN;
		}
	}
	/**
	 * Latest: Sytem is working fine without any delay being added
	 * Old: recommended to add a short delay after interrupt to ensure the TMP117 data is stable
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
		printf("[ TMP117TemperatureSensor %d ] ALERT should be cleared after this read.\n", sensor_id);
	
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

    printf("[ TMP117TemperatureSensor %d ] ALERT should now be cleared.\n", sensor_id);
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

	TMP117TemperatureSensor(int sensor_id, Buzzer* buzzer){
		this->sensor_id = sensor_id;
		this->buzzer = buzzer;
	}

	void readAndPrintStartupTemperature() {
		double temp = readTemperature();
		if (!std::isnan(temp)) {
			printf("[ TMP117TemperatureSensor %d ] : 🌡️ Startup Temperature: %.2f °C\n", sensor_id, temp);
		} else {
			printf("[ TMP117TemperatureSensor %d ] : Failed to read temperature at startup.\n", sensor_id);
		}
	}
	
	//To set the TMP117 into CR mode is a feature of the TMP117 temperature sensor where the ALERT pin (INT) is used to signal when a new temperature conversion is ready.
	  
	void initialize() {
		std::lock_guard<std::mutex> lock(i2c_mutex);
	
		int addr = (sensor_id == 1) ? 0x48 : 0x49;
		char devicePath[] = "/dev/i2c-1";
	
		int fd = open(devicePath, O_RDWR);
		if (fd < 0) {
			perror("I2C open failed \n");
			return;
		}
	
		if (ioctl(fd, I2C_SLAVE, addr) < 0) {
			perror("I2C address set failed \n");
			close(fd);
			return;
		}
	
		usleep(200000); // Wait for TMP117 boot
	
		// Read device ID (0x0F) to verify sensor presence
		unsigned char reg = 0x0F;
		unsigned char id_buf[2];
		if (write(fd, &reg, 1) != 1 || read(fd, id_buf, 2) != 2) {
			perror("Device ID read failed \n");
			close(fd);
			return;
		}
	
		uint16_t device_id = (id_buf[0] << 8) | id_buf[1];
		printf("[ TMP117TemperatureSensor %d ]: Detected TMP117 (ID: 0x%04X)\n", sensor_id, device_id);
	
		// Dummy temperature read (0x00)
		reg = 0x00;
		unsigned char temp_buf[2];
		if (write(fd, &reg, 1) == 1 && read(fd, temp_buf, 2) == 2) {
			printf("[ TMP117TemperatureSensor %d ]: Dummy temp read: 0x%02X 0x%02X\n", sensor_id, temp_buf[0], temp_buf[1]);
		} else {
			perror("Dummy temperature read failed \n");
		}
	
		usleep(5000);
	
		// Write T_HIGH and T_LOW manually
		uint16_t high_raw = static_cast<uint16_t>(20.0 / 0.0078125);
		uint16_t low_raw  = static_cast<uint16_t>(10.0 / 0.0078125);
	
		// Send thresholds
		i2c_smbus_write_word_data(fd, 0x02, high_raw);
		i2c_smbus_write_word_data(fd, 0x03, low_raw);
		printf("[ TMP117TemperatureSensor %d ]: T_HIGH set to ~20.0°C\n", sensor_id);
		printf("[ TMP117TemperatureSensor %d ]: T_LOW set to ~10.0°C\n", sensor_id);
	
		// Read temperature to clear ALERT pin
		reg = 0x00;
		write(fd, &reg, 1);
		read(fd, temp_buf, 2);
		usleep(10000);
	
		// Construct i2cset command string
		char cmd[128];
		//snprintf(cmd, sizeof(cmd), "i2cset -y 1 0x%02X 0x01 0x6662 w", addr);
		snprintf(cmd, sizeof(cmd), "i2cset -y 1 0x%02X 0x01 0x8403 w", addr); //every 16 seconds
		//snprintf(cmd, sizeof(cmd), "i2cset -y 1 0x%02X 0x01 0x0400 w", addr); //swap the bytes of config register, Lower first and high second
		printf("[ TMP117TemperatureSensor %d ]: Running: %s\n", sensor_id, cmd);
	
		int ret = system(cmd);
		if (ret != 0) {
			fprintf(stderr, "[ TMP117TemperatureSensor %d ]: i2cset command failed with return code %d\n", sensor_id, ret);
			close(fd);
			return;
		}
	
		close(fd);
	}

	static constexpr double HIGH_THRESHOLD = 25.0;
    static constexpr double LOW_THRESHOLD  = 5.0;

	private:
	int sensor_id;
	Buzzer* buzzer = nullptr;
	
};

int main(int argc, char *argv[]) {
	fprintf(stderr,"Press any key to stop.\n");

	Buzzer shared_buzzer = Buzzer(0, 25);
	
	TMP117TemperatureSensor *t1, *t2;
	GPIOPin gpiopin17, gpiopin27;
	const int gpioPinNo17 = 17;
	const int gpioPinNo27 = 27;

	t1 = new TMP117TemperatureSensor(1, &shared_buzzer);
	t2 = new TMP117TemperatureSensor(2, &shared_buzzer);

	t1->initialize();
	gpiopin17.registerCallback(t1);
	gpiopin17.start(gpioPinNo17, 0, "TMP117 Temperature Sensor");

	t2->initialize();
	gpiopin27.registerCallback(t2);
	gpiopin27.start(gpioPinNo27, 0, "TMP117 Temperature Sensor");
	
	// Force a startup temperature read for both sensors
	t1->readAndPrintStartupTemperature();
	t2->readAndPrintStartupTemperature();
	
	getchar(); //Blocking for the main program :: exit on key press

    gpiopin17.stop();
	gpiopin27.stop();

	return 0;
}
