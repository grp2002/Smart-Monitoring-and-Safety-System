#ifndef BUZZER_H
#define BUZZER_H

#include <gpiod.h>
#include <iostream>
#include <mutex>

class Buzzer {
public:
    Buzzer(int chip_num, int line_num);
    ~Buzzer();

    void on();
    void off();

private:
    struct gpiod_chip* chip = nullptr;
    struct gpiod_line* line = nullptr;
    static std::mutex buzzer_mtx;
};

#endif // BUZZER_H
