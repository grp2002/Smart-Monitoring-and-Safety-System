cmake_minimum_required(VERSION 3.10)
project(smart_system)

set(CMAKE_CXX_STANDARD 17)

# Find libgpiod
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)
#pkg_check_modules(I2C REQUIRED i2c-dev)

include_directories(${GPIOD_INCLUDE_DIRS})
include_directories(${I2C_INCLUDE_DIRS})
find_path(I2C_INCLUDE_DIR i2c/smbus.h)
include_directories(${I2C_INCLUDE_DIR})


add_executable(smart_system gpioevent.cpp buzzer.h SmartMonitoringAndSafetySystem.cpp)
#add_executable(sensor_0x48 gpioevent.cpp TMP117Sensor0x48.cpp)
#add_executable(sensor_0x49 gpioevent.cpp TMP117Sensor0x49.cpp)
#add_executable(buzzer buzzer_control.cpp)

# Link libgpiod
#target_link_libraries(smart_system PRIVATE ${GPIOD_LIBRARIES} ${I2C_LIBRARIES})
target_link_libraries(smart_system PRIVATE ${GPIOD_LIBRARIES} i2c)
#target_link_libraries(smart_system PRIVATE ${GPIOD_LIBRARIES} /usr/lib/arm-linux-gnueabihf/libi2c.so)
#target_link_libraries(sensor_0x48 PRIVATE ${GPIOD_LIBRARIES})
#target_link_libraries(sensor_0x49 PRIVATE ${GPIOD_LIBRARIES})
#target_link_libraries(buzzer PRIVATE ${GPIOD_LIBRARIES})
#target_include_directories(buzzer PRIVATE ${GPIOD_INCLUDE_DIRS})


