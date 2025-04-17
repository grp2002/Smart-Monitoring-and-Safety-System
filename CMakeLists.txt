cmake_minimum_required(VERSION 3.10)
project(camera_test)

# Find OpenCV (needed for displaying frames)
find_package(OpenCV REQUIRED)

# Include header directories
include_directories(
    ${OpenCV_INCLUDE_DIRS}       # OpenCV headers
    /usr/include/libcamera       # Ensure libcamera headers are found
)

# Define the executable and source files
add_executable(camera_test
    main.cpp
    MotionSensor.cpp    
    CameraController.cpp
)

# Link against required libraries
target_link_libraries(camera_test
    ${OpenCV_LIBS}        # OpenCV
    libcam2opencv         # Static lib you built from the GitHub repo
    camera                # libcamera core
    camera-base           # base library (needed for signal, event handling)
    gpiod                 # for GPIO access
)
