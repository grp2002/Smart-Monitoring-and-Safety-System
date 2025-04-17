#include "CameraController.h"
#include <iostream>

// Constructor: initializes state variables and frame callback
CameraController::CameraController()
    : motionDetected_(false), cameraRunning_(false), running_(false)
{
    frameCallback_ = FrameCallback();
}

// Destructor: ensures camera and thread are properly stopped
CameraController::~CameraController()
{
    stop();
}

// This function is called when the motion sensor detects movement
void CameraController::onMotionDetected()
{
    motionDetected_ = true;

    // Ensure only one thread can start/stop the camera at a time
    std::lock_guard<std::mutex> lock(camMutex_);

    // If the camera isn't running, start it and attach the frame callback
    if (!cameraRunning_)
    {
        std::cout << "[Camera] Motion detected. Starting camera.\n";
        camera_.registerCallback(&frameCallback_);
        camera_.start();
        cameraRunning_ = true;
    }
}

// Called every time a new frame is available from the camera
void CameraController::FrameCallback::hasFrame(const cv::Mat &frame, const libcamera::ControlList &)
{
    // Display the live camera feed in a window
    cv::imshow("Camera View", frame);
    cv::waitKey(1); // Allow OpenCV to handle GUI events
}

// Starts the internal monitoring thread that watches for motion inactivity
void CameraController::start()
{
    running_ = true;
    timeoutThread_ = std::thread(&CameraController::monitorTimeout, this);
}

// Stops the monitoring thread and turns off the camera if it is still on
void CameraController::stop()
{
    running_ = false;

    // Wait for the timeout thread to finish if it's still running
    if (timeoutThread_.joinable())
        timeoutThread_.join();

    std::lock_guard<std::mutex> lock(camMutex_);

    // Stop the camera if it's running
    if (cameraRunning_)
    {
        camera_.stop();
        cameraRunning_ = false;
    }

    // Close any OpenCV display windows
    cv::destroyAllWindows();
}

// Monitors inactivity: if no motion for 10 seconds, stops the camera
void CameraController::monitorTimeout()
{
    while (running_)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));

        // If no motion has been detected in the last 10 seconds
        if (!motionDetected_)
        {
            std::lock_guard<std::mutex> lock(camMutex_);

            // Stop the camera if it was running
            if (cameraRunning_)
            {
                std::cout << "[Camera] No motion for 10s. Stopping camera.\n";
                camera_.stop();
                cameraRunning_ = false;
                cv::destroyAllWindows();
            }
        }

        // Reset the motion detection flag for the next 10s interval
        motionDetected_ = false;
    }
}
