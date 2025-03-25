#include <iostream>              // For std::cout, std::cerr
#include "gpioevent.h"           // Your GPIO event class
#include "CameraManager.h"       // The camera control class
#include "MotionSensorHandler.h" // Handles motion event + camera logic

int main()
{
    // GPIO pin number where the motion sensor's OUT pin is connected
    const int motionPin = 22;

    // Create a GPIOPin object for monitoring the motion sensor pin
    GPIOPin motionGPIO;

    // Create a CameraManager object to handle the camera preview
    CameraManager camera;

    // Create a MotionSensorHandler which responds to motion
    // Pass the CameraManager reference so it can start/stop the camera
    MotionSensorHandler motionHandler(camera);

    // Register the motion handler as a callback with the GPIO pin
    // So when a motion event happens, hasEvent() is triggered
    motionGPIO.registerCallback(&motionHandler);

    // Start listening for events on GPIO pin 22
    motionGPIO.start(motionPin);

    // Start a separate thread that checks for motion timeout (10s)
    motionHandler.startMonitoring();

    // Wait for user input to end the program (press Enter)
    std::cout << "System running. Press Enter to stop..." << std::endl;
    getchar();

    // Cleanly stop the GPIO pin monitoring thread
    motionGPIO.stop();

    // Stop the motion sensor's timer thread and the camera if running
    motionHandler.stopMonitoring();

    std::cout << "System exited cleanly." << std::endl;

    return 0;
}
