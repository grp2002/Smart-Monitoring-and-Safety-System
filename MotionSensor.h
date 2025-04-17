#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

#include <gpiod.h> // For GPIO access via libgpiod
#include <thread>  // For background monitoring thread
#include <vector>  // To hold multiple registered callbacks
#include <atomic>  // For safe concurrent flag handling

/**
 * @brief MotionSensor class
 *
 * Monitors a GPIO pin connected to a PIR motion sensor.
 * When motion is detected (via a rising edge), it notifies all registered callbacks.
 */
class MotionSensor
{
public:
    /**
     * @brief Interface for objects that want to receive motion detection events.
     */
    struct CallbackInterface
    {
        virtual void onMotionDetected() = 0;
    };

    /**
     * @brief Constructs a MotionSensor object.
     *
     * @param chipNumber Index of the GPIO chip (usually 0 on Raspberry Pi).
     * @param lineNumber GPIO line number (e.g., BCM pin number).
     */
    MotionSensor(int chipNumber, int lineNumber);

    /**
     * @brief Destructor — stops the thread and releases resources.
     */
    ~MotionSensor();

    /**
     * @brief Registers a listener for motion detection events.
     *
     * @param cb Pointer to an object implementing CallbackInterface.
     */
    void registerCallback(CallbackInterface *cb);

    /**
     * @brief Starts the monitoring thread.
     */
    void start();

    /**
     * @brief Stops the monitoring thread.
     */
    void stop();

private:
    /**
     * @brief Thread function that monitors GPIO line events.
     */
    void run();

    // Hardware configuration
    int chipNumber_; ///< GPIO chip number
    int lineNumber_; ///< GPIO line number

    // libgpiod GPIO handles
    gpiod_chip *chip_; ///< Pointer to GPIO chip object
    gpiod_line *line_; ///< Pointer to specific GPIO line

    // Callback system
    std::vector<CallbackInterface *> callbacks_; ///< List of registered callbacks

    // Threading
    std::thread thread_;        ///< Background thread that runs `run()`
    std::atomic<bool> running_; ///< Flag to control thread loop
};

#endif // MOTION_SENSOR_H
