cmake_minimum_required(VERSION 3.10)
project(smart_system)

set(CMAKE_CXX_STANDARD 17)

# Find libgpiod
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)
#pkg_check_modules(I2C REQUIRED i2c-dev)

include_directories(${GPIOD_INCLUDE_DIRS})
include_directories(${I2C_INCLUDE_DIRS})
#find_path(I2C_INCLUDE_DIR i2c/smbus.h)
#include_directories(${I2C_INCLUDE_DIR})


add_executable(smart_system 
SmartMonitoringAndSafetySystem.cpp
gpioevent.cpp
buzzer.cpp
)

# Link libgpiod
target_link_libraries(smart_system PRIVATE ${GPIOD_LIBRARIES} i2c)


