/**
 * @about: 
 * SafePrint.h provides a simple, thread-safe wrapper around printf() for multi-threaded C++ programs.
 * It ensures that log messages from multiple threads do not interleave or overwrite each other on the console.
 */

#ifndef SAFE_PRINTF_H
#define SAFE_PRINTF_H

/*
 * Copyright (c) 2025 Pragya Shilakari, Gregory Paphiti, Abhishek Jain, Ninad Shende, Ugochukwu Elvis Som Anene, Hankun Ma
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation. See the file LICENSE.
 */

#include <cstdio>
#include <cstdarg>
#include <mutex>

namespace SafePrint {

    // Shared mutex for all printf calls across threads
    inline std::mutex& getMutex() {
        static std::mutex print_mutex;
        return print_mutex;
    }

    // Thread-safe printf function
    inline void printf(const char* format, ...) {
        std::lock_guard<std::mutex> lock(getMutex());

        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);

        fflush(stdout);  // Optional: ensure it flushes immediately
    }

}

#endif // SAFE_PRINTF_H
