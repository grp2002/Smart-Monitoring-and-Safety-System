#include "buzzer.h"

// Mutex to protect the shared buzzer resoure from race conditions in a multi-threaded environment
std::mutex Buzzer::buzzer_mtx;

//Constructor definition
Buzzer::Buzzer(int chip_num, int line_num) {
    chip = gpiod_chip_open_by_number(chip_num);
    if (!chip) {
        std::cerr << "[ Buzzer ] :: Error: Failed to open GPIO chip: " << chip_num << " for Buzzer\n";
        return;
    }

    line = gpiod_chip_get_line(chip, line_num);
    if (!line) {
        std::cerr << "[ Buzzer ] :: Error: Failed to get GPIO line " << line_num << " for Buzzer\n";
        gpiod_chip_close(chip);
        chip = nullptr;
        return;
    }

    if (gpiod_line_request_output(line, "buzzer", 0) < 0) {
        std::cerr << "[ Buzzer ] :: Error: Failed to request buzzer line " << line_num << " as output\n";
        gpiod_chip_close(chip);
        chip = nullptr;
        line = nullptr;
    }
}

//Destructor definition
Buzzer::~Buzzer() {
    if (line) gpiod_line_release(line);
    if (chip) gpiod_chip_close(chip);
}

void Buzzer::on() {
    std::lock_guard<std::mutex> lock(buzzer_mtx); //acquire the lock to perform the buzzer operation
    if (line) {
        printf("[ Buzzer ] :: Beep the buzzer\n");
        gpiod_line_set_value(line, 1);
    }
} //lock released on exit

void Buzzer::off() {
    std::lock_guard<std::mutex> lock(buzzer_mtx); //acquire the lock to perform the buzzer operation
    if (line) {
        printf("[ Buzzer ] :: Turn-Off the beep\n");
        gpiod_line_set_value(line, 0);
    }
} //lock released on exit