#include "MotionSensor.h"
#include <iostream>

/**
 * @brief Constructor for MotionSensor
 *
 * Initializes the GPIO chip and line, and configures it to listen for
 * both rising and falling edge events (i.e., changes in sensor state).
 *
 * @param chipNumber GPIO chip index (typically 0 on Raspberry Pi)
 * @param lineNumber GPIO line number (BCM pin number)
 */
MotionSensor::MotionSensor(int chipNumber, int lineNumber)
    : chipNumber_(chipNumber), lineNumber_(lineNumber), chip_(nullptr), line_(nullptr), running_(false)
{
    // Open the GPIO chip
    chip_ = gpiod_chip_open_by_number(chipNumber_);
    if (!chip_)
    {
        throw std::runtime_error("Failed to open GPIO chip");
    }

    // Get a specific GPIO line
    line_ = gpiod_chip_get_line(chip_, lineNumber_);
    if (!line_)
    {
        gpiod_chip_close(chip_);
        throw std::runtime_error("Failed to get GPIO line");
    }

    // Request edge events (both rising and falling) on the line
    if (gpiod_line_request_both_edges_events(line_, "MotionSensor") < 0)
    {
        gpiod_chip_close(chip_);
        throw std::runtime_error("Failed to request GPIO events");
    }
}

/**
 * @brief Destructor
 *
 * Ensures the sensor thread is stopped and the GPIO line/chip are released properly.
 */
MotionSensor::~MotionSensor()
{
    stop();
    gpiod_line_release(line_);
    gpiod_chip_close(chip_);
}

/**
 * @brief Registers a callback listener that will be notified on motion detection
 *
 * @param cb A pointer to an object that implements the CallbackInterface
 */
void MotionSensor::registerCallback(CallbackInterface *cb)
{
    callbacks_.push_back(cb);
}

/**
 * @brief Starts the motion sensor event monitoring thread
 */
void MotionSensor::start()
{
    if (running_)
        return;
    running_ = true;
    thread_ = std::thread(&MotionSensor::run, this);
}

/**
 * @brief Stops the monitoring thread safely
 */
void MotionSensor::stop()
{
    running_ = false;
    if (thread_.joinable())
    {
        thread_.join();
    }
}

/**
 * @brief Internal thread loop
 *
 * Waits for GPIO edge events and triggers callbacks on rising edges (motion detected).
 */
void MotionSensor::run()
{
    const timespec timeout = {5, 0}; // 5 second timeout for event wait
    while (running_)
    {
        // Wait for an edge event on the GPIO line
        int ret = gpiod_line_event_wait(line_, &timeout);
        if (ret == 1)
        {
            gpiod_line_event event;

            // Read the actual event from the line
            if (gpiod_line_event_read(line_, &event) == 0 &&
                event.event_type == GPIOD_LINE_EVENT_RISING_EDGE)
            {
                // Notify all registered callbacks
                for (auto &cb : callbacks_)
                {
                    cb->onMotionDetected();
                }
            }
        }
    }
}
