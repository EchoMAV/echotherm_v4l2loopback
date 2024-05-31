/*Copyright (c) [2020] [Seek Thermal, Inc.]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The Software may only be used in combination with Seek cores/products.

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Project:	 Seek Thermal SDK Demo
 * Purpose:	 Demonstrates how to communicate with Seek Thermal Cameras
 * Author:	 Seek Thermal, Inc.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// C includes
#include <csignal>
#include <cstring>

// C++ includes
#include <algorithm>
#include <array>
#include <atomic>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <condition_variable>
#include <mutex>
#include <fcntl.h>


#ifdef __linux__
#include <linux/videodev2.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

// Seek SDK includes
#include "seekcamera/seekcamera.h"
#include "seekcamera/seekcamera_manager.h"
#include "seekframe/seekframe.h"

// Structure representing a renderering interface.
// It uses SDL and all rendering is done on the calling thread.
struct seekrenderer_t
{
	seekcamera_t* camera{};

	// Synchronization data
	std::mutex the_mutex;
	std::atomic<bool> is_active;
	std::atomic<bool> is_dirty;
	seekcamera_frame_t* frame{};
};

// Define the global variables
static std::atomic<bool> g_exit_requested;                       // Controls application shutdown.
static std::map<std::string, seekrenderer_t*> g_renderers;		 // Tracks all renderers.
static std::mutex g_mutex;										 // Synchronizes camera events that need to be coordinated with main thread.
static std::condition_variable g_condition_variable;			 // Synchronizes camera events that need to be coordinated with main thread.
static std::atomic<bool> g_is_dirty;



void seekrenderer_close_all()
{
	for(auto& kvp : g_renderers)
	{
		if(kvp.second != nullptr)
		{
			if(kvp.second->is_active.load())
			{
				seekcamera_capture_session_stop(kvp.second->camera);
			}
			kvp.second->is_active.store(false);
			kvp.second->is_dirty.store(false);
			kvp.second->frame = nullptr;

			// Invalidate references that rely on the camera lifetime.
			kvp.second->camera = nullptr;
		}
	}
}


// Handles frame available events.
void handle_camera_frame_available(seekcamera_t* camera, seekcamera_frame_t* camera_frame, void* user_data)
{
	(void)camera;
	auto* renderer = (seekrenderer_t*)user_data;

	// This mutex is used to serialize access to the frame member of the renderer.
	// In the future the single frame member should be replaced with something like a FIFO
	std::lock_guard<std::mutex> guard(renderer->the_mutex);

	// Lock the seekcamera frame for safe use outside of this callback.
	seekcamera_frame_lock(camera_frame);
	renderer->is_dirty.store(true);

	// Note that this will always render the most recent frame. There could be better buffering here but this is a simple example.
	renderer->frame = camera_frame;
	g_is_dirty.store(true);
	g_condition_variable.notify_one();
}

// Handles camera connect events.
void handle_camera_connect(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	(void)event_status;
	(void)user_data;
	seekcamera_chipid_t cid{};
	seekcamera_get_chipid(camera, &cid);
	std::string chipID((char*)&cid);
	seekrenderer_t* renderer = g_renderers[chipID] == nullptr ? new seekrenderer_t() : g_renderers[chipID];
	renderer->is_active.store(true);
	renderer->camera = camera;

	// Register a frame available callback function.
	seekcamera_error_t status = seekcamera_register_frame_available_callback(camera, handle_camera_frame_available, (void*)renderer);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to register frame callback: " << seekcamera_error_get_str(status) << std::endl;
		renderer->is_active.store(false);
		return;
	}

	// set pipeline mode to SeekVision
	status = seekcamera_set_pipeline_mode(camera, SEEKCAMERA_IMAGE_SEEKVISION);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to set image pipeline mode: " << seekcamera_error_get_str(status) << std::endl;
		renderer->is_active.store(false);
		return;
	}

	// Start the capture session.
	//status = seekcamera_capture_session_start(camera, SEEKCAMERA_FRAME_FORMAT_COLOR_ARGB8888);
	status = seekcamera_capture_session_start(camera, SEEKCAMERA_FRAME_FORMAT_COLOR_YUY2);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to start capture session: " << seekcamera_error_get_str(status) << std::endl;
		renderer->is_active.store(false);
		return;
	}

	g_renderers[chipID] = renderer;
}

// Handles camera disconnect events.
void handle_camera_disconnect(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	(void)event_status;
	(void)user_data;
	seekcamera_chipid_t cid{};
	seekcamera_get_chipid(camera, &cid);
	std::string chipID((char*)&cid);
	auto renderer = g_renderers[chipID];

	seekcamera_capture_session_stop(camera);

	if(renderer != nullptr)
	{
		renderer->is_active.store(false);
		g_is_dirty.store(true);
	}
}

// Handles camera error events.
void handle_camera_error(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	(void)user_data;
	seekcamera_chipid_t cid{};
	seekcamera_get_chipid(camera, &cid);
	std::cerr << "unhandled camera error: (CID: " << cid << ")" << seekcamera_error_get_str(event_status) << std::endl;
}

// Handles camera ready to pair events
void handle_camera_ready_to_pair(seekcamera_t* camera, seekcamera_error_t event_status, void* user_data)
{
	// Attempt to pair the camera automatically.
	// Pairing refers to the process by which the sensor is associated with the host and the embedded processor.
	const seekcamera_error_t status = seekcamera_store_calibration_data(camera, nullptr, nullptr, nullptr);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to pair device: " << seekcamera_error_get_str(status) << std::endl;
	}

	// Start imaging.
	handle_camera_connect(camera, event_status, user_data);
}

// Callback function for the camera manager; it fires whenever a camera event occurs.
void camera_event_callback(seekcamera_t* camera, seekcamera_manager_event_t event, seekcamera_error_t event_status, void* user_data)
{
	seekcamera_chipid_t cid{};
	seekcamera_get_chipid(camera, &cid);
	std::cout << seekcamera_manager_get_event_str(event) << " (CID: " << cid << ")" << std::endl;

	// Handle the event type.
	switch(event)
	{
		case SEEKCAMERA_MANAGER_EVENT_CONNECT:
			handle_camera_connect(camera, event_status, user_data);
			break;
		case SEEKCAMERA_MANAGER_EVENT_DISCONNECT:
			handle_camera_disconnect(camera, event_status, user_data);
			break;
		case SEEKCAMERA_MANAGER_EVENT_ERROR:
			handle_camera_error(camera, event_status, user_data);
			break;
		case SEEKCAMERA_MANAGER_EVENT_READY_TO_PAIR:
			handle_camera_ready_to_pair(camera, event_status, user_data);
			break;
		default:
			break;
	}
}

static void signal_callback(int signum)
{
	(void)signum;
	std::cout << std::endl;
	std::cout << "Caught termination signal";
	std::cout << std::endl;
	g_exit_requested.store(true);
}



#ifdef __linux__
// Function to setup output to a v4l2 device
int setup_v4l2(int width,int height) {
    //TODO un-hard-code this
    ::std::string output = "/dev/video2";

    int v4l2 = open(output.c_str(), O_RDWR); 
    if(v4l2 < 0) {
        std::cout << "Error opening v4l2 device: " << strerror(errno) << std::endl;
        exit(1);
    }

    struct v4l2_format v;
    int t;
    v.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    
    t = ioctl(v4l2, VIDIOC_G_FMT, &v);
    if( t < 0 ) {
        std::cout << "VIDIOC_G_FMT error: " << strerror(errno) << std::endl;
        exit(t);
    }
    
    v.fmt.pix.width = width;
    v.fmt.pix.height = height;
    v.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    v.fmt.pix.sizeimage = width * height * 2;
    t = ioctl(v4l2, VIDIOC_S_FMT, &v);
    if( t < 0 ) {
        std::cout << "VIDIOC_S_FMT error: " << strerror(errno) << std::endl;
        exit(t);
    }

    std::cout << "Opened v4l2 device" << std::endl;
    return v4l2;
}

#else
int setup_v4l2(std::string output, int width, int height) {
    std::cout << "v4l2 is not supported on this platform" << std::endl;
    exit(1);
    return -1; 
}
#endif // __linux__



// Application entry point.
int main()
{
	int v4l2=-2;
	// Initialize global variables.
	g_exit_requested.store(false);
	g_is_dirty.store(false);

	// Install signal handlers.
	signal(SIGINT, signal_callback);
	signal(SIGTERM, signal_callback);

	std::cout << "seekcamera_capture prototype starting\n";

	// Create the camera manager.
	// This is the structure that owns all Seek camera devices.
	seekcamera_manager_t* manager = nullptr;
	seekcamera_error_t status = seekcamera_manager_create(&manager, SEEKCAMERA_IO_TYPE_USB);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to create camera manager: " << seekcamera_error_get_str(status) << std::endl;
		return 1;
	}

	// Register an event handler for the camera manager to be called whenever a camera event occurs.
	status = seekcamera_manager_register_event_callback(manager, camera_event_callback, nullptr);
	if(status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to register camera event callback: " << seekcamera_error_get_str(status) << std::endl;
		return 1;
	}

	// Poll for events until told to stop.
	// Both renderer events and SDL events are polled.
	// Events are polled on the main thread because SDL events must be handled on the main thread.
	while(!g_exit_requested.load())
	{
		std::unique_lock<std::mutex> event_lock(g_mutex);
		if(g_condition_variable.wait_for(event_lock, std::chrono::milliseconds(150), [=] { return g_is_dirty.load(); }))
		{
			g_is_dirty.store(false);
			for(const auto& kvp : g_renderers)
			{
				auto renderer = kvp.second;
				// Check if renderer is inactive
				if(renderer != nullptr && renderer->is_active.load())
				{
					// This mutex is used to serialize access to the frame member of the renderer.
					// In the future the single frame member should be replaced with something like a FIFO
					std::lock_guard<std::mutex> guard(renderer->the_mutex);

					// Render frame if necessary
					if(renderer->is_dirty.load() && renderer->frame != nullptr && renderer->is_active.load())
					{

						// Get the frame to draw.
						seekframe_t* frame = nullptr;
						status = seekcamera_frame_get_frame_by_format(renderer->frame, SEEKCAMERA_FRAME_FORMAT_COLOR_YUY2, &frame);
						if(status != SEEKCAMERA_SUCCESS)
						{
							std::cerr << "failed to get frame: " << seekcamera_error_get_str(status) << std::endl;
							seekcamera_frame_unlock(renderer->frame);
							continue;
						}
						if(v4l2==-2)
						{
							int const frame_width = (int)seekframe_get_width(frame);
							int const frame_height = (int)seekframe_get_height(frame);
							::std::cout<<"frame_width = "<<frame_width<<", frame_height = "<<frame_height<<::std::endl;
							v4l2 = setup_v4l2(frame_width,frame_height);
						}
						int written = write(v4l2, seekframe_get_data(frame), seekframe_get_data_size(frame));
						if (written < 0) 
						{
							std::cout << "Error writing v4l2 device" << std::endl;
							close(v4l2);
							exit(1);
						}

						// Unlock the camera frame.
						seekcamera_frame_unlock(renderer->frame);
						renderer->is_dirty.store(false);
						renderer->frame = nullptr;
					}
				}
			}
		}
		else
		{
			event_lock.unlock();
		}
	}

	std::cout << "Destroying camera manager" << std::endl;

	// Teardown the camera manager.
	seekcamera_manager_destroy(&manager);

	seekrenderer_close_all();
	for(auto& kvp : g_renderers)
	{
		// free the renderer pointers before clearing the map
		delete kvp.second;
	}
	// removes kvp from the mapd
	g_renderers.clear();

	std::cout << "done" << std::endl;
	return 0;
}
