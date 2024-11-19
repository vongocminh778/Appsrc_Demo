#include <opencv2/opencv.hpp>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <iostream>

// Function to push a video frame to GStreamer's appsrc
static gboolean push_frame(GstElement* appsrc, cv::Mat frame) {
    // Debug: Display frame information
    std::cout << "Pushing frame: " << frame.cols << "x" << frame.rows 
              << " channels: " << frame.channels() << std::endl;

    GstFlowReturn ret;

    // Create a GStreamer buffer from the OpenCV frame
    GstBuffer* buffer = gst_buffer_new_wrapped_full(
        GST_MEMORY_FLAG_PHYSICALLY_CONTIGUOUS,  // Ensure memory is contiguous
        frame.data,                             // Frame data
        frame.total() * frame.elemSize(),       // Data size in bytes
        0,                                      // Offset within data
        frame.total() * frame.elemSize(),       // Length of valid data
        nullptr,                                // Free function (not needed here)
        nullptr                                 // Free function data
    );

    // Push the buffer to appsrc
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);

    // Unreference the buffer to free memory
    gst_buffer_unref(buffer);

    return G_SOURCE_CONTINUE;
}

int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);

    // Create GStreamer pipeline
    GstElement* pipeline = gst_parse_launch(
        "appsrc name=source format=time stream-type=stream is-live=true do-timestamp=true "
        "! video/x-raw,framerate=30/1,width=1920,height=1080,format=RGBA "
        "! nvvidconv interpolation-method=4 "
        "! video/x-raw(memory:NVMM),width=1920,height=1080,format=NV12 "
        "! nvoverlaysink sync=false async=false ", nullptr);
    if (!pipeline) {
        std::cerr << "Failed to create GStreamer pipeline" << std::endl;
        return -1;
    }

    // Get the appsrc element from the pipeline
    GstElement* appsrc = gst_bin_get_by_name(GST_BIN(pipeline), "source");
    if (!appsrc) {
        std::cerr << "Failed to get appsrc element" << std::endl;
        gst_object_unref(pipeline);
        return -1;
    }

    // Set pipeline to the PLAYING state
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Generate video frames using OpenCV
    cv::Mat frame(1080, 1920, CV_8UC4); // Create a 1920x1080 RGBA frame
    int frame_count = 0;

    while (frame_count < 1000) { // Push 300 frames (~10 seconds at 30 FPS)
        // Generate a gradient effect for demonstration
        frame.forEach<cv::Vec4b>([&](cv::Vec4b &pixel, const int *pos) {
            pixel[0] = frame_count % 255;       // Blue channel
            pixel[1] = (frame_count * 2) % 255; // Green channel
            pixel[2] = (frame_count * 3) % 255; // Red channel
            pixel[3] = 255;                     // Alpha channel
        });

        // Push the frame to the appsrc
        push_frame(appsrc, frame);

        frame_count++;
    }

    // Stop the pipeline
    gst_element_set_state(pipeline, GST_STATE_NULL);

    // Clean up resources
    gst_object_unref(appsrc);
    gst_object_unref(pipeline);

    return 0;
}
