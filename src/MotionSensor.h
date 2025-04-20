#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

#include "gpioevent.h"

class MotionSensor : public GPIOPin::GPIOEventCallbackInterface {
public:
    void hasEvent(gpiod_line_event& event) override;
};

#endif // MOTION_SENSOR_H
