#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <atomic>
#include <thread>
#include <opencv2/opencv.hpp>
#include <libcamera2opnecv.hpp>

/**
 * @brief Class that manages the Raspberry Pi camera stream.
 * Uses libcamera2opencv to get frames and OpenCV to display them.
 */
class CameraManager
{
public:
    /**
     * Constructor. Initializes the camera as not running.
     */
    CameraManager():running(false){};

    /**
     * Starts the camera stream in a separate thread.
     * If already running, does nothing.
     */
    void startCamera(){
        if(running){
            return;
        }
        running = true;
        cameraThread = std::thread(&CameraManager::cameraThreadFunction, this);
    }

    /**
     * Stops the camera stream and joins the thread.
     */
    void stopCamera(){
        running = false;
        if(cameraThread.joinable()){
            cameraThread.join();
        }
    }

    /**
     * Checks whether the camera is currently running.
     * @return true if camera is active
     */
    bool isRunning() const;{
        return running;
    }

    private:
        void cameraThreadFunction();
        std::mutex mtx; // Protects access to 'running'
        std::thread cameraThread;

        /**
         * Background loop that runs in a separate thread.
         * Captures frames from the camera and displays them using OpenCV.
         */
        void cameraLoop(){
            Libcamera20penCV camera;
            camera.openCamera(); // Start camera capture

            cv::Mat frame; // frame is declared to store the images captured from the camera.

            while(true){
                {
                    std::lock_guard<std::mutex> lock(mtx); // Thread-safe check: uses a std::mutex (mtx) to safely check a shared variable running.
                    if (!running)
                    {
                        break; // Exit if running flag is cleared
                }
                }
                if(!camera.getFrame(frame)){
                    break;
                }

                cv::imshow("Motion Camera", frame); // Display the frame
                if(cv::waitKey(1) == 27){ // Wait for 1ms for a key press. If the key is ESC (27), break the loop.
                    break;
                }
            }
            camera.closeCamera(); // Stop camera capture
            cv::destroyAllWindows(); // Close all OpenCV windows
        }
}
#endif // CAMERAMANAGER_H