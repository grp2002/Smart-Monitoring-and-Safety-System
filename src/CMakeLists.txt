cmake_minimum_required(VERSION 3.10)
project(smart_system)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD 11)

# Enable Qt's automoc, autouic, autorcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)
#pkg_check_modules(I2C REQUIRED i2c-dev)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(fastcdr REQUIRED)
find_package(fastrtps REQUIRED)
#find_package(OpenCV REQUIRED)

#pkg_check_modules(LIBCAMERA REQUIRED IMPORTED_TARGET libcamera)
#message(STATUS "libcamera library found:")
#message(STATUS "    version: ${LIBCAMERA_VERSION}")
#message(STATUS "    libraries: ${LIBCAMERA_LINK_LIBRARIES}")
#message(STATUS "    include path: ${LIBCAMERA_INCLUDE_DIRS}")

#message(STATUS "opencv:")
#message(STATUS "    version: ${OpenCV_VERSION}")
#message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

#include_directories(${CMAKE_SOURCE_DIR} ${LIBCAMERA_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS})

# Include directories
include_directories(
    ${GPIOD_INCLUDE_DIRS}
    i2c
    ${Qt5Widgets_INCLUDE_DIRS}
    ${QWT_INCLUDE_DIRS}
#    ${LIBCAMERA_INCLUDE_DIRS}
#    ${OPENCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
include(GNUInstallDirs)

# Create a library for the message / topic
add_custom_target(
    messages
    COMMAND fastddsgen SensorMsg.idl
)

add_library(SensorMsg STATIC SensorMsg.cxx SensorMsgPubSubTypes.cxx)
add_dependencies(SensorMsg messages)

# Source files
set(SOURCES
    window.cpp
    SmartMonitoringAndSafetySystem.cpp
    TMP117TemperatureSensor.cpp
    gpioevent.cpp
    buzzer.cpp
    SensorMsgPublisher.cpp
#    libcam2opencv.cpp
    MotionSensor.cpp
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
        fastcdr 
        fastrtps 
        SensorMsg
   #     PkgConfig::LIBCAMERA
   #     ${OpenCV_LIBS}
)

# Install target
install(TARGETS smart_system)

# Subscriber
add_executable(SensorMsgSubscriber SensorMsgSubscriber.cpp window.cpp)
target_link_libraries(SensorMsgSubscriber 
            Qt5::Widgets
            qwt-qt5 #qwt
            fastcdr 
            fastrtps 
            SensorMsg
)

