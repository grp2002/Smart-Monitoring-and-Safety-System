cmake_minimum_required(VERSION 3.6)

project(FastDDSExample)

find_package(fastcdr REQUIRED)
find_package(fastrtps REQUIRED)

set (CMAKE_CXX_STANDARD 17)

message(STATUS "Configuring publisher/subscriber example...")


# Create a library for the message / topic
add_custom_target(
    messages
    COMMAND fastddsgen SensorMsg.idl
)
add_library(SensorMsg STATIC SensorMsg.cxx SensorMsgPubSubTypes.cxx)
add_dependencies(SensorMsg messages)


# Publisher
add_executable(SensorMsgPublisher SensorMsgPublisher.cpp)
target_link_libraries(SensorMsgPublisher fastcdr fastrtps SensorMsg)


# Subscriber
add_executable(SensorMsgSubscriber SensorMsgSubscriber.cpp)
target_link_libraries(SensorMsgSubscriber fastcdr fastrtps SensorMsg)
