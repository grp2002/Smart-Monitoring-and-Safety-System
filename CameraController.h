#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "MotionSensor.h"     // For motion sensor interface and callback
#include "libcam2opencv.h"    // For camera control using libcamera
#include <opencv2/opencv.hpp> // For image display with OpenCV
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>

/**
 * @brief CameraController handles camera activation based on motion events.
 *
 * Starts the camera when motion is detected, and stops it after 10 seconds
 * of inactivity. Uses libcamera2opencv for camera integration and OpenCV for
 * frame display.
 */
class CameraController : public MotionSensor::CallbackInterface
{
public:
    CameraController();  // Constructor
    ~CameraController(); // Destructor

    // Called when motion is detected by the MotionSensor
    void onMotionDetected() override;

    // Starts the background monitoring thread
    void start();

    // Stops the monitoring thread and shuts down the camera
    void stop();

private:
    // Internal class to handle camera frame display
    struct FrameCallback : public Libcam2OpenCV::Callback
    {
        // Called by libcamera2opencv when a new frame is ready
        void hasFrame(const cv::Mat &frame, const libcamera::ControlList &) override;
    };

    // Monitors inactivity and stops the camera if no motion for 10 seconds
    void monitorTimeout();

    Libcam2OpenCV camera_;             // Camera wrapper
    FrameCallback frameCallback_;      // Object to handle camera frames
    std::atomic<bool> motionDetected_; // Tracks recent motion detection
    std::atomic<bool> cameraRunning_;  // Indicates if the camera is active
    std::mutex camMutex_;              // Synchronizes access to camera start/stop
    std::thread timeoutThread_;        // Thread that monitors motion timeout
    std::atomic<bool> running_;        // Controls the monitor thread lifecycle
};

#endif // CAMERA_CONTROLLER_H
