/**
 * Version Details: 
 * ABOUT: Smart Monitoring and Safety System software to monitor temperature sensor 
 * via blocking IO in an interrupt driven model and calling the event handler callback
 * functions for event notification.
 * INPUT: two TMP117 temperature sensors
 * OUTPUT: 
 * (1) Console output on the terminal
 * (2) Trigger the buzzer beep 
 * (3) Display the real time temperature values using QT Thermo, Plot and Table widgets
 */

#include <cstdlib> // for system()
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
#include "gpioevent.h"
#include "buzzer.h"
#include "SafePrint.h" //safe printf in multi-threaded environment
#include <mutex>
//For QT:
#include <QApplication>
#include <csignal>
#include "window.h"

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
		std::string timeStr = std::ctime(&currentTime);
		timeStr.pop_back(); // removes the '\n' from the end
		SafePrint::printf("\n[ %s ] :: [TMP117TemperatureSensor::hasEvent() {%d}] : interrupt received!\n\r", timeStr.c_str(), sensor_id);
		
		/*Read the Temperature value when DATA READY INTERRUPT Pin is Low*/
	    switch (e.event_type) {
			case GPIOD_LINE_EVENT_RISING_EDGE:
			{	/**
				* TMP117 sends the rising event after the 0x00 or 0x01 register has been read. 
				*/

				SafePrint::printf("[TMP117TemperatureSensor::hasEvent() {%d}] : Rising!\n\r", sensor_id);
				break;
			}
			case GPIOD_LINE_EVENT_FALLING_EDGE:
			{	/**
				 * TMP117 sends the falling event after temperature conversion result is ready in the
				 * 0x00 register.
				 */
				SafePrint::printf("[TMP117TemperatureSensor::hasEvent() {%d}] : Falling\n\r", sensor_id);
				double temperature = readTemperature();

				if (std::isnan(temperature)) { //Error handling: Invalid value
					SafePrint::printf("[TMP117TemperatureSensor::hasEvent() {%d}] : Failed to read temperature.\n\r", sensor_id);
				} else {
					SafePrint::printf("[TMP117TemperatureSensor::hasEvent() {%d}] : 🌡️ Temperature: %.2f °C\n\r", sensor_id, temperature);
					
					/**
					 * [ALERT!] Send the Buzzer beep alert when Temperature has crossed the threshold range
					 */
					if (temperature > HIGH_THRESHOLD || temperature < LOW_THRESHOLD) {
						SafePrint::printf("⚠️ [ALERT!] [TMP117TemperatureSensor::hasEvent() {%d}] :Trigger the buzzer beep to alert! \n\r", sensor_id);
						buzzer->on();
						sleep(1);
						buzzer->off();
					}
					
					onTemperatureRead(temperature); //QT
					
					
				}
				
				break;
			}
			default:
			SafePrint::printf("[TMP117TemperatureSensor::hasEvent() {%d}] : Unkown event\n\r", sensor_id);
		}
	}
	/**
	 * Function to read temperature from TMP117 over I2C
	 */
	double readTemperature() {
		SafePrint::printf("[TMP117TemperatureSensor::readTemperature() {%d}] : waiting for I2C lock.\n\r", sensor_id);
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
        SafePrint::printf("[TMP117TemperatureSensor::readTemperature() {%d}] : Error : Failed to open I2C bus!\n\r", sensor_id);
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
			SafePrint::printf("[TMP117TemperatureSensor::readTemperature() {%d}] : Error : Failed to set I2C address! %s\n\r", sensor_id, strerror(errno));
			close(fd);
			return NAN;
		}
	}
	else if (sensor_id == 2){
		if (ioctl(fd, I2C_SLAVE, TMP117_ADDR2) < 0) {
			SafePrint::printf("[TMP117TemperatureSensor::readTemperature() {%d}] : Error : Failed to set I2C address! %s\n\r", sensor_id, strerror(errno));
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

   unsigned char reg = TMP117_TEMP_REG;
    if (write(fd, &reg, 1) != 1) {
        SafePrint::printf("[TMP117TemperatureSensor::readTemperature() {%d}] : Error : Failed to write to TMP117! %s\n\r", sensor_id, strerror(errno));
        close(fd);
        return NAN;
    }

	usleep(1000); //recommended to add a delay between write() and read()
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
        SafePrint::printf("[TMP117TemperatureSensor::readTemperature() {%d}] : Error : Failed to read temperature data!\n\r", sensor_id);
        close(fd);
        return NAN;
    }
	//DEBUG START
	//SafePrint::printf("[TMP117TemperatureSensor::readTemperature() {%d}] : DATA READY Alert should be cleared after this read.\n\r", sensor_id);
	//DEBUG END

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
	 * Refer TMP117 Data sheet for more information
	 */
    int16_t rawTemp = (buffer[0] << 8) | buffer[1];
    return rawTemp * 0.0078125;  // Conversion formula from TMP117 datasheet
	}
	
	public:

	TMP117TemperatureSensor(int sensor_id, Buzzer* buzzer){
		this->sensor_id = sensor_id;
		this->buzzer = buzzer;
	}

	void readAndPrintStartupTemperature() {
		double temp = readTemperature();
		if (!std::isnan(temp)) {
			SafePrint::printf("[TMP117TemperatureSensor::readAndPrintStartupTemperature() {%d}] : 🌡️ Startup Temperature: %.2f °C\n\r", sensor_id, temp);
		} else {
			SafePrint::printf("[TMP117TemperatureSensor::readAndPrintStartupTemperature() {%d}] : Failed to read temperature at startup.\n\r", sensor_id);
		}
	}
	
	/**
	 * IMPORTANT: [From TMP117 data sheet]
	 * Set the bits 11-10 of TMP117 16 bits config register [0x01] as 00 or 10 to keep the sensor in Continuous conversion (CC) mode
	 * When the MOD[1:0] bits are set to 00 or 10 in the configuration register, the device operates in continuous conversion mode. 
	 * The device continuously performs temperature conversions in this mode and updates the temperature result register at the end 
	 * of every active conversion. The user can read the configuration register or the temperature result register to clear the 
	 * Data_Ready flag. Therefore, the Data_Ready flag (INT) can be used to determine when the conversion completes so that an 
	 * external controller can synchronize reading the result register with conversion result updates. The user can set the 
	 * DR/nAlert_EN bit in the configuration register to monitor the state of the Data_Ready flag on the ALERT pin.
	 */
	 
	void initialize() {
		std::lock_guard<std::mutex> lock(i2c_mutex);
	
		int addr = (sensor_id == 1) ? 0x48 : 0x49;
		char devicePath[] = "/dev/i2c-1";
	
		int fd = open(devicePath, O_RDWR);
		if (fd < 0) {
			SafePrint::printf("[TMP117TemperatureSensor::initialize()] : [ERROR] : I2C open failed \n\r");
			return;
		}
	
		if (ioctl(fd, I2C_SLAVE, addr) < 0) {
			SafePrint::printf("[TMP117TemperatureSensor::initialize()] : [ERROR] : I2C address set failed \n\r");
			close(fd);
			return;
		}
	
		usleep(200000); // Wait for TMP117 boot
	
		// Read device ID (0x0F) to verify sensor presence
		unsigned char reg = 0x0F; //device ID register
		unsigned char id_buf[2];
		if (write(fd, &reg, 1) != 1 || read(fd, id_buf, 2) != 2) {
			SafePrint::printf("[TMP117TemperatureSensor::initialize()] : [ERROR] : Device ID read failed \n\r");
			close(fd);
			return;
		}
	
		uint16_t device_id = (id_buf[0] << 8) | id_buf[1];
		SafePrint::printf("[TMP117TemperatureSensor::initialize() {%d}] : Detected TMP117 (ID: 0x%04X)\n\r", sensor_id, device_id);
	
		/**
		 * Construct i2cset command string to configure the TMP117 config register 0x01
		 * For: 
		 * Continuous conversion (CC) mode, and
		 * 16 seconds conversion cycle time 
		 * IMPORTANT:
		 * Config value should be in little-endian representation. Take the lower byte first, then the higher byte second.
		 * Examples:
		 * 0x8403 : CC mode and 16 seconds conversion cycle
		 * 0x0400 : CC mode and 1 second conversion cycle
		 */ 
		char cmd[128];
		//snprintf(cmd, sizeof(cmd), "i2cset -y 1 0x%02X 0x01 0x8403 w", addr);
		snprintf(cmd, sizeof(cmd), "i2cset -y 1 0x%02X 0x01 0x0400 w", addr);
		SafePrint::printf("[TMP117TemperatureSensor::initialize() {%d}] : Running : %s\n\r", sensor_id, cmd);
	
		int ret = system(cmd);
		if (ret != 0) {
			SafePrint::printf("[TMP117TemperatureSensor::initialize() {%d}] : [ERROR] : i2cset command failed with return code %d\n\r", sensor_id, ret);
			close(fd);
			return;
		}
	
		close(fd);
	}

	std::function<void(double)> onTemperatureRead;
	static constexpr double HIGH_THRESHOLD = 30.0; //High Temperature Threshold value
    static constexpr double LOW_THRESHOLD  = 15.0;  //Low Temperature Threshold value

	private:
	int sensor_id;	//introduced to uniquely identify the TMP117 sensor instance
	Buzzer* buzzer = nullptr;
	
};

//For QT safe exit:
void handleSignal(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        qApp->quit();  // Safe Qt application exit
    }
}

int main(int argc, char *argv[]) {
//QT Start
	QApplication app(argc, argv);
    
    Window sensor1Window, sensor2Window;
	sensor1Window.setWindowTitle("TMP117 Sensor 1");
	sensor2Window.setWindowTitle("TMP117 Sensor 2");

	// Position windows
	sensor1Window.move(100, 100);     // Top window
	sensor2Window.move(100, 500);     // Below the first window

	sensor1Window.show();
	sensor2Window.show();
//QT End
	//SafePrint::printf("****Press any key to stop.****\n\r");
	SafePrint::printf("****Close all the QT windows to stop.****\n\r");
	SafePrint::printf("****If running from a remote SSH window using 'export DISPLAY=:0' then run 'reset' after stop.****\n\r");

	Buzzer shared_buzzer = Buzzer(0, 25);
	
	GPIOPin gpiopin17, gpiopin27;
	const int gpioPinNo17 = 17;
	const int gpioPinNo27 = 27;

	TMP117TemperatureSensor t1 = TMP117TemperatureSensor(1, &shared_buzzer);
	/**
	 * Callback implementation using a lambda (inline) function. 
	 * It connects non-Qt backend logic (t1, the TMP117 sensor) to a Qt GUI method (updateTemperature).
	 * @QMetaObject::invokeMethod() ensures that the GUI update runs in the Qt main thread, which is 
	 * thread-safe and essential when callback runs from a separate thread (main()).
	 */
	t1.onTemperatureRead = [&](double t) {
        QMetaObject::invokeMethod(&sensor1Window, "updateTemperature", Qt::QueuedConnection,
                                  Q_ARG(double, t));
    };

	TMP117TemperatureSensor t2 = TMP117TemperatureSensor(2, &shared_buzzer);
	t2.onTemperatureRead = [&](double t) {
        QMetaObject::invokeMethod(&sensor2Window, "updateTemperature", Qt::QueuedConnection,
                                  Q_ARG(double, t));
    };

	t1.initialize();
	gpiopin17.registerCallback(&t1);
	gpiopin17.start(gpioPinNo17, 0, "TMP117 Temperature Sensor");

	t2.initialize();
	gpiopin27.registerCallback(&t2);
	gpiopin27.start(gpioPinNo27, 0, "TMP117 Temperature Sensor");
	
	// Force a startup temperature read for both sensors
	t1.readAndPrintStartupTemperature();
	t2.readAndPrintStartupTemperature();
	
	/**
	 * Qt applications must return app.exec() without blocking the main thread. 
	 * getchar() is blocking the Qt event loop; it's not needed with QT else it will lead to 
	 * Crashes or hangs. 
	 */
	//getchar(); //Blocking for the main program :: exit on key press

    // shutdown with QT
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
        gpiopin17.stop();
        gpiopin27.stop();
    });

	//return 0;
	return app.exec();
}
