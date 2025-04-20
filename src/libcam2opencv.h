#ifndef __LIBCAM2OPENCV
#define __LIBCAM2OPENCV

/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020, Ideas on Board Oy.
 * Copyright (C) 2024, Bernd Porr
 * Copyright (C) 2021, kbarni https://github.com/kbarni/
 */

#include <iomanip>
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <memory>
#include <sys/mman.h>
#include <opencv2/opencv.hpp>

// need to undefine QT defines here as libcamera uses the same expressions (!).
#undef signals
#undef slots
#undef emit
#undef foreach

#include <libcamera/libcamera.h>

/**
 * Settings
 **/
struct Libcam2OpenCVSettings {
    /**
     * Width of the video capture. A zero lets libcamera decide the width.
     **/
    unsigned int width = 0;
    
    /**
     * Height of the video capture. A zero lets libcamera decide the height.
     **/
    unsigned int height = 0;

    /**
     * Framerate. A zero lets libcamera decide the framerate.
     **/
    unsigned int framerate = 0;

    /**
     * Brightness
     **/
    float brightness = 0.0;

    /**
     * Contrast
     **/
    float contrast = 1.0;
};

class Libcam2OpenCV {
public:
    struct Callback {
	virtual void hasFrame(const cv::Mat &frame, const libcamera::ControlList &metadata) = 0;
	virtual ~Callback() {}
    };

    /**
     * Register the callback for the frame data
     **/
    void registerCallback(Callback* cb) {
	callback = cb;
    }

    /**
     * Starts the camera and the callback at default resolution and framerate
     **/
    void start(Libcam2OpenCVSettings settings = Libcam2OpenCVSettings() );

    /**
     * Stops the camera and the callback
     **/
    void stop();
    
private:
    // Shared pointer to a camera device.
    std::shared_ptr<libcamera::Camera> camera; 
    // Unique pointer for Camera configuration so that only one object manage and modify the config.
    std::unique_ptr<libcamera::CameraConfiguration> config;
    // Image data in OpenCV matrix format
    cv::Mat frame;
    // Callback handler
    Callback* callback = nullptr;
    // Libcamera frame buffer to store incoming image frames
    libcamera::FrameBufferAllocator* allocator = nullptr;
    // Libcamera stream pointer. Camera supports mulitple streams e.g., preview, capture etc.
    libcamera::Stream *stream = nullptr;
    /*
    * Unique pointer to libcamera Camera Manager, used to manage camera devices connected to the system.
    * Used to enumerate, acquire, and release camera hardware.
    */
    std::unique_ptr<libcamera::CameraManager> cm;
    /*
    * Vector to store list of capture requests sent to Camera.
    * Request object wraps buffers, controls, and settings for a frame capture.
    */
    std::vector<std::unique_ptr<libcamera::Request>> requests;
    /*
    * Libcamera control list (settings) to be applied to camera hardware.
    * Sent along with a request to configure each capture.
    */
    libcamera::ControlList controls;
    /*
    * Map to links each frame buffer to its corresponding memory spans, to access the actual image data 
    * for processing.
    */
    std::map<libcamera::FrameBuffer *, std::vector<libcamera::Span<uint8_t>>> mapped_buffers;

    /**
     * @brief Maps a given FrameBuffer to its associated memory spans.
     *
     * This function retrieves the memory regions (spans) that were previously
     * mapped and associated with the given libcamera::FrameBuffer.
     * 
     * The mapping is used to access the raw image data captured by the camera.
     * If the provided buffer is not found in the map, an empty vector is returned.
     *
     * @param buffer Pointer to the FrameBuffer to retrieve mapped memory for.
     * @return A vector of libcamera::Span<uint8_t> representing the memory regions,
     *         or an empty vector if the buffer was not found.
     */
    std::vector<libcamera::Span<uint8_t>> Mmap(libcamera::FrameBuffer *buffer) const
    {
	auto item = mapped_buffers.find(buffer);
	if (item == mapped_buffers.end())
	    return {};
	return item->second;
    }

    /*
     * --------------------------------------------------------------------
     * Handle RequestComplete
     *
     * For each Camera::requestCompleted Signal emitted from the Camera the
     * connected Slot is invoked.
     */
    void requestComplete(libcamera::Request *request);
};

#endif
