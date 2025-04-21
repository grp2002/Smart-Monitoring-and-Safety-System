#ifndef BUZZER_H
#define BUZZER_H

/**
 * 
 * Copyright 2025 Pragya Shilakari, Gregory Paphiti, Abhishek Jain, Ninad Shende, Ugochukwu Elvis Som Anene, Hankun Ma
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
