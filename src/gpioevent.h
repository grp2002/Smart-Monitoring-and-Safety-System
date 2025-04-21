#ifndef __GPIOEVENT_H
#define __GPIOEVENT_H

/*
 * Copyright (c) 2025 Bernd Porr <mail@berndporr.me.uk>
 * Copyright (c) 2025 Pragya Shilakari, Gregory Paphiti, Abhishek Jain, Ninad Shende, Ugochukwu Elvis Som Anene, Hankun Ma
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. See the file LICENSE.
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <thread>
#include <gpiod.h>
#include <vector>
#include <string>

// enable debug messages and error messages to stderr
#ifndef NDEBUG
#define DEBUG
#endif

#define ISR_TIMEOUT 1 // sec

/*GPIOPin class instance represent the GPIO Pin of a given chip and provide funtionality 
  to implement blocking IO event handling for events on the GPIO pin */
class GPIOPin {

public:
    /**
     * Destructor which makes sure the data acquisition
     * stops on exit.
     **/
    ~GPIOPin() {
	stop();
    }

    struct GPIOEventCallbackInterface {
	    /**
	     * Called when a new sample is available.
	     * This needs to be implemented in a derived
	     * class by the client. Defined as abstract.
	     * \param e If falling or rising.
	     **/
	virtual void hasEvent(gpiod_line_event& e) = 0;
    };

    void registerCallback(GPIOEventCallbackInterface* ci) {
	callbackInterfaces.push_back(ci);
    }

    /**
     * Starts listening on the GPIO pin.
     * \param chipNo GPIO Chip number. It's usually 0.
     * \param pinNo GPIO Pin number.
     * \param processName Process controlling / monitoring this Pin.
     **/
    void start(int pinNo,
	       int ChipNo = 0, std::string processName = "Consumer");

    /**
     * Stops listening to the pin.
     **/
    void stop();

    gpiod_line* getLine();

private:

    // gpiod  related stuff
    gpiod_chip *chipGPIO = nullptr; //GPIO chip handle
    gpiod_line *pinGPIO = nullptr; //handle to the GPIO line(pin)
    
    //pollfd pfd; 

    // thread reference for concurrent execution
    std::thread thr; 
    
    // flag that it's running
    bool running = false;
    
    // vector to hold the instances of GPIO event callback interfaces
    std::vector<GPIOEventCallbackInterface*> callbackInterfaces;
    
    //this fucntion call the event handler(s)
    void gpioEvent(gpiod_line_event& event);
    
    void worker();
    

};


#endif
