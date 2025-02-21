cmake_minimum_required(VERSION 3.10)
project(smart_system)

set(CMAKE_CXX_STANDARD 17)

# Find libgpiod
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)

add_executable(smart_system gpioevent.cpp SmartMonitoringAndSafetySystem.cpp)

# Link libgpiod
target_link_libraries(smart_system PRIVATE ${GPIOD_LIBRARIES})
