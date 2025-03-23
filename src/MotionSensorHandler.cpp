#include "gpioevent.h"
#include "CameraManager.h"
/**
 * @brief Handles GPIO events from a motion sensor.
 *        Starts the camera on motion and stops it after 10 seconds of no motion.
 */
class MotionSensorHandler: public GPIOPin::GPIOEventCallbackInterface {
	public:
        /**
         * @param cam Reference to the CameraManager used to start/stop the camera.
         */
        MotionSensorHandler(CameraManager &cam)
            : camera(cam), running(false), cameraActive(false) {}

        /**
         * @brief Called when a GPIO event is detected.
         *        Starts or keeps the camera running on motion detection.
         */
        void hasEvent(gpiod_line_event& e) override {
            if(e.event_type == GPIOD_LINE_EVENT_RISING_EDGE){
                if(!running){
                    std::lock_guard<std::mutex> lock(mtx);
                    printf("Motion detected!\n");

                    //Update the time of the last motion detection
                    lastMotionTime = std::chrono::steady_clock::now();

                    //start camera if it is not already running
                    if(!camera.isRunning()){
                        camera.startCamera();
                        cameraActive = true;
                    }
                }
            }
        }
        /**
         * @brief Starts the background timer thread to monitor motion timeout.
         */
        void startMonitoring(){
            std::lock_guard<std::mutex> lock(mtx);
            running = true;
            timerThread = std::thread(&MotionSensorHandler::monitorMotionTimeout, this);
        }
        /**
         * @brief Stops the monitoring thread and the camera if it’s still running.
         */
        void stopMonitoring(){
            {
                std::lock_guard<std::mutex> lock(mtx);
                running = false;
            }
            if(timerThread.joinable()){
                timerThread.join();
            }
            std::lock_guard<std::mutex> lock(mtx);
            if(cameraActive){
                camera.stopCamera();
                cameraActive = false;
            }
        }
    private:
        std::chrono::steady_clock::time_point lastMotionTime; // Time of last motion detection
        bool running; // Flag to indicate if the monitoring thread is running
        bool cameraActive; // Flag to indicate if the camera is active
        std::thread timerThread; // Thread to monitor motion timeout
        std::mutex mtx; // Mutex to protect shared variables
        CameraManager& camera; // Reference to the CameraManager

        /**
        * @brief Runs in a thread to monitor if 10 seconds have passed since the last motion.
         *        If so, stops the camera.
         */
        void timerLoop(){
            while(true){
                std::lock_guard<std::mutex> lock(mtx);
                if(!running){
                    break;
                }

                if(cameraActive){
                    auto now = std::chrono::steady_clock::now();
                    auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - lastMotionTime).count();
                    if(diff >= 10){
                        printf("No motion. Stopping camera.\n");
                        camera.stopCamera();
                        cameraActive = false;
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

};