#include "CameraController.h"
#include <iostream>

// Constructor: initializes state variables and frame callback
CameraController::CameraController()
    : cameraRunning_(false), running_(false)
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
    timeoutSeconds_ = 10; // Top up the motion timer
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
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "[Timer] Seconds left: " << timeoutSeconds_ << std::endl;
        int t = timeoutSeconds_;
        if (t > 0)
        {
            timeoutSeconds_--; // Decrement the timer

            // Start camera if it's not running
            if (!cameraRunning_)
            {
                std::lock_guard<std::mutex> lock(camMutex_);
                if (!cameraRunning_)
                {
                    std::cout << "[Camera] Timer active. Starting camera.\n";
                    camera_.registerCallback(&frameCallback_);
                    camera_.start();
                    cameraRunning_ = true;
                }
            }
        }
        else // timeoutSeconds_ == 0
        {
            // Stop the camera if it's running
            if (cameraRunning_)
            {
                std::lock_guard<std::mutex> lock(camMutex_);
                if (cameraRunning_)
                {
                    std::cout << "[Camera] Timer expired. Stopping camera.\n";
                    camera_.stop();
                    cameraRunning_ = false;
                    cv::destroyAllWindows();
                }
            }
        }
    }
}
