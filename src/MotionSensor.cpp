#include "MotionSensor.h"
#include "SafePrint.h" //safe printf in multi-threaded environment
#include "gpioevent.h"
#include <memory>
#include <iostream>
#include <unistd.h>

MotionSensor::MotionSensor(Buzzer* buzzer){
    this->buzzer = buzzer;
}
void MotionSensor::hasEvent(gpiod_line_event& event) {
    if (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE) {
        SafePrint::printf("Motion Detected!\n\r");
        buzzer->on();
        sleep(2);
        buzzer->off();

    } else if (event.event_type == GPIOD_LINE_EVENT_FALLING_EDGE) {
        SafePrint::printf("Motion Ended or no motion\n\r");
    }
}

/*//For independent testing 
int main() {
    try {
        // Create GPIOPin object for GPIO 23 (physical pin 16), on chip 0
        GPIOPin gpio;
        gpio.start(23, 0, "MotionSensor");

        // Create a MotionSensor and register as callback
        MotionSensor sensor;
        gpio.registerCallback(&sensor);

        SafePrint::printf("📡 Motion sensor monitoring started on GPIO 23...\n\r");

        // Keep the program running (can also be replaced with event loop)
        while (true) {
            sleep(1);
        }

        gpio.stop(); // Optional cleanup if loop ever exits

    } catch (const char* err) {
        SafePrint::printf("Error: %s \n\r",err);
    }

    return 0;
}

*/