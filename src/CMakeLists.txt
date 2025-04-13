cmake_minimum_required(VERSION 3.10)
project(smart_system)

#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 11)

# Enable Qt's automoc, autouic, autorcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)
#pkg_check_modules(I2C REQUIRED i2c-dev)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# Include directories
include_directories(
    ${GPIOD_INCLUDE_DIRS}
    i2c
    ${Qt5Widgets_INCLUDE_DIRS}
    ${QWT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Source files
set(SOURCES
    window.cpp
    SmartMonitoringAndSafetySystem.cpp
    gpioevent.cpp
    buzzer.cpp
)

# Add executable
add_executable(smart_system ${SOURCES})

# Link libraries
target_link_libraries(smart_system
    PRIVATE
        ${GPIOD_LIBRARIES}
        i2c           # This links -li2c
        Qt5::Widgets
        qwt-qt5 #qwt
)

# Install target
install(TARGETS smart_system)
