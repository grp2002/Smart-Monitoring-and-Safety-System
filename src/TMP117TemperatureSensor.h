#ifndef TMP117_TEMPERATURE_SENSOR_H
#define TMP117_TEMPERATURE_SENSOR_H

#include "gpioevent.h"
#include "buzzer.h"
#include "SensorMsgPublisher.h"
#include "SensorMsg.h"
#include <functional>

class TMP117TemperatureSensor : public GPIOPin::GPIOEventCallbackInterface {
public:
    TMP117TemperatureSensor(int sensor_id, Buzzer* buzzer);
    void initialize();
    void readAndPrintStartupTemperature();
    double readTemperature();
    void hasEvent(gpiod_line_event& e) override;
    std::function<void(double)> onTemperatureRead;
    void setSensorMsgPublisher(SensorMsgPublisher* pub);

    static constexpr double HIGH_THRESHOLD = 30.0;
    static constexpr double LOW_THRESHOLD = 15.0;

private:
    int sensor_id; // to uniquely identify the TMP117 sensor instance
    Buzzer* buzzer;
    SensorMsgPublisher* msgPublisher;
    SensorMsg message;
};

#endif // TMP117_TEMPERATURE_SENSOR_H
