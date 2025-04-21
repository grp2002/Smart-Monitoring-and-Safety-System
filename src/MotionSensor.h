#ifndef MOTION_SENSOR_H
#define MOTION_SENSOR_H

/*
 * Copyright (c) 2025 Bernd Porr <mail@berndporr.me.uk>
 * Copyright (c) 2025 Pragya Shilakari, Gregory Paphiti, Abhishek Jain, Ninad Shende, Ugochukwu Elvis Som Anene, Hankun Ma
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. See the file LICENSE.
 */

#include "gpioevent.h"
#include "buzzer.h"

class MotionSensor : public GPIOPin::GPIOEventCallbackInterface {
public:
    MotionSensor(Buzzer* buzzer);
    void hasEvent(gpiod_line_event& event) override;
private:
    Buzzer* buzzer;
};

#endif // MOTION_SENSOR_H
