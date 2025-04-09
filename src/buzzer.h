#include <gpiod.h>
#include <iostream>
#include <unistd.h> // for sleep()

#define GPIO_CHIP "/dev/gpiochip0"  // Chip 0 maps to GPIO pins 0–31 on Pi 5
#define GPIO_LINE 25                // Change this to your actual GPIO pin number

int static buzzer() {
    gpiod_chip *chip;
    gpiod_line *line;
    int ret;

    // Open GPIO chipsm 
    chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) {
        std::cerr << "Failed to open GPIO chip\n";
        return 1;
    }

    // Get GPIO line
    line = gpiod_chip_get_line(chip, GPIO_LINE);
    if (!line) {
        std::cerr << "Failed to get GPIO line\n";
        gpiod_chip_close(chip);
        return 1;
    }

    // Request line as output
    ret = gpiod_line_request_output(line, "buzzer", 0);
    if (ret < 0) {
        std::cerr << "Failed to request line as output\n";
        gpiod_chip_close(chip);
        return 1;
    }

    // Turn ON buzzer
    std::cout << "Buzzer ON\n";
    gpiod_line_set_value(line, 1);
    sleep(1);  // keep it on for 1 seconds

    // Turn OFF buzzer
    std::cout << "Buzzer OFF\n";
    gpiod_line_set_value(line, 0);

    // Release resources
    gpiod_line_release(line);
    gpiod_chip_close(chip);

    return 0;
}
