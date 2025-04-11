#include "gpioevent.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string>
#include "SafePrint.h" //safe printf in multi-threaded environment

void GPIOPin::start(int pinNo,
		    int chipNo, std::string processName) {
	
#ifdef DEBUG
    SafePrint::printf("[ERROR] GPIO pin %d on chip %d is being init.\n\r",pinNo,chipNo);
#endif

	/**
	 * @brief Open a gpiochip by number.
	 * @param num Number of the gpiochip.
	 * @return GPIO chip handle or NULL if an error occurred.
	 *
	 * This routine appends num to '/dev/gpiochip' to create the path.
	 */
    chipGPIO = gpiod_chip_open_by_number(chipNo);
    if (NULL == chipGPIO) {
#ifdef DEBUG
	SafePrint::printf("[ERROR] GPIO chip could not be accessed.\n\r");
#endif
	throw "GPIO chip error.\n\r";
    }
	
	/**
	 * @brief Get the handle to the GPIO line at given offset.
	 * @param chip The GPIO chip object.
	 * @param offset The offset of the GPIO line.
	 * @return Pointer to the GPIO line handle or NULL if an error occured.
	 */   
    pinGPIO = gpiod_chip_get_line(chipGPIO,pinNo);
    if (NULL == pinGPIO) {
#ifdef DEBUG
	SafePrint::printf("[ERROR] GPIO line could not be accessed.\n\r");
#endif
	throw "GPIO line error.\n\r";
    }

	/**
	 * @brief Request all event type notifications on a single line.
	 * @param line GPIO line object.
	 * @param consumer Name of the consumer.
	 * @return 0 if the operation succeeds, -1 on failure.
	 */
    int ret = gpiod_line_request_both_edges_events(pinGPIO, processName.c_str());
	//int ret = gpiod_line_request_falling_edge_events(pinGPIO, "Consumer");
    if (ret < 0) {
#ifdef DEBUG
	SafePrint::printf("[ERROR] Request event notification failed on pin %d and chip %d.\n\r",
		pinNo,chipNo);
#endif
	throw "Could not request event for IRQ.";
    }

    running = true;
	/*
	* Concurrently execute non-static member function GPIOPin::worker()
	* Pass fuction reference and object reference as params
	*/
    thr = std::thread(&GPIOPin::worker,this);
}

void GPIOPin::gpioEvent(gpiod_line_event& event) {
	/*
	* Loops over each element in the callbackInterfaces vector 
	* Using element reference and not copying element
	* Deduces the element type automatically
	*/
	for(auto &cb: callbackInterfaces) {
		SafePrint::printf("[GPIOPin::gpioEvent()] : GPIO event received...\n\r");
	    cb->hasEvent(event);
		//DEBUG
		usleep(1000);
		int level = gpiod_line_get_value(pinGPIO);
		SafePrint::printf("[GPIOPin::gpioEvent()] : [DEBUG] : GPIO pin state after read: %d\n\r", level);
	}
}


void GPIOPin::worker() {

	int value = gpiod_line_get_value(pinGPIO);
	if (value == 0) {
		SafePrint::printf("[GPIOPin::worker()] : GPIO Pin low at startup — force read to clear ALERT\n\r");
		gpiod_line_event fake_event;
        fake_event.event_type = GPIOD_LINE_EVENT_FALLING_EDGE;

        // Set dummy timestamp (optional)
        fake_event.ts.tv_sec = 0;
        fake_event.ts.tv_nsec = 0;

        gpioEvent(fake_event);  
	}
    while (running) {
	const timespec ts = { ISR_TIMEOUT, 0 };

	/**
	 * this blocks till an interrupt has happened!
	 * @brief Wait for an event on a single line.
	 * @param line GPIO line object.
	 * @param timeout Wait time limit.
	 * @return 0 if wait timed out, -1 if an error occurred, 1 if an event
	 *         occurred.
	 */
	int r = gpiod_line_event_wait(pinGPIO, &ts);

	// check if it really has been an event
	if (1 == r) {
	    gpiod_line_event event;

		/**
		 * @brief Read next pending event from the GPIO line.
		 * @param line GPIO line object.
		 * @param event Buffer to which the event data will be copied.
		 * @return 0 if the event was read correctly, -1 on error.
		 * @note This function will block if no event was queued for this line.
		 */
	    gpiod_line_event_read(pinGPIO, &event);

		/**
		 * @brief Read current value of a single GPIO line.
		 * @param line GPIO line object.
		 * @return 0 or 1 if the operation succeeds. On error this routine returns -1
		 *         and sets the last error number.
		 */
		gpiod_line_get_value(pinGPIO);
		
		//call the event handler
	    gpioEvent(event);
	} else if (r < 0) {
#ifdef DEBUG
	    SafePrint::printf("[ERROR] GPIO error while waiting for event.\n\r");

#endif
	}
    }
}


void GPIOPin::stop() {
    
	if (!running) return;
	
	// set running to false to signal the worker() function to exit the loop so Thread can finish processing
    running = false;
	thr.join(); //wait for concurrent thread to finish processing 

	/**
	 * @brief Release a previously reserved line.
	 * @param line GPIO line object.
	 */
    gpiod_line_release(pinGPIO);

	/**
	 * @brief Close a GPIO chip handle and release all allocated resources.
	 * @param chip The GPIO chip object.
	 */
    gpiod_chip_close(chipGPIO);
}

gpiod_line* GPIOPin::getLine() {
	return pinGPIO;
}
