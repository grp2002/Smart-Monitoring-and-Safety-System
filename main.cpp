#include "libcam2opencv.h"
#include "CameraController.h"
#include "MotionSensor.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>

/**
 * @brief Entry point for the Smart Monitoring System.
 *
 * Initializes the motion sensor and camera controller.
 * When motion is detected, the camera is activated.
 * If no motion is detected for 10 seconds, the camera stops.
 */
int main()
{
    try
    {
        // Initialize motion sensor on GPIO chip 0, line 27
        // Adjust the line number to match your wiring setup
        MotionSensor motionSensor(0, 27);

        // Create the camera controller (connects motion to camera behavior)
        CameraController cameraController;

        // Register camera controller as a listener for motion events
        motionSensor.registerCallback(&cameraController);

        // Start the motion sensor and background monitoring thread
        motionSensor.start();
        cameraController.start();

        std::cout << "System running... press Ctrl+C to exit.\n";

        // Keep the program alive (simulate embedded main loop)
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Cleanup (unreachable unless using signals or future exit mechanism)
        motionSensor.stop();
        cameraController.stop();
    }
    catch (const std::exception &ex)
    {
        // Handle any initialization or runtime errors
        std::cerr << "Error: " << ex.what() << "\n";
    }

    return 0;
}
