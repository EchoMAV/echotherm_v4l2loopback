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

// C++ includes
#include <iostream>
#include <unordered_map>
#include <mutex>


#ifdef __linux__
#include <linux/videodev2.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

// Seek SDK includes
#include "seekcamera/seekcamera.h"
#include "seekcamera/seekcamera_manager.h"
#include "seekframe/seekframe.h"




class SeekCamera
{
public:
	SeekCamera(std::string const& devicePath, seekcamera_frame_format_t format, seekcamera_color_palette_t colorPalette);
	~SeekCamera();
	seekcamera_error_t connect(seekcamera_t* p_camera);
	seekcamera_error_t disconnect();
	seekcamera_error_t handleReadyToPair(seekcamera_t* p_camera);

private:
	int openDevice(int frameWidht, int frameHeight);
	void handleCameraFrameAvailable(seekcamera_frame_t* p_cameraframe);


	std::string m_devicePath;
	seekcamera_frame_format_t m_format;
	seekcamera_color_palette_t m_colorPalette;
	seekcamera_t* mp_camera;
	std::mutex m_mut;
	int m_device;
};


SeekCamera::SeekCamera(std::string const& devicePath, seekcamera_frame_format_t format, seekcamera_color_palette_t colorPalette)
	: m_devicePath{devicePath}
	, m_format{format}
	, m_colorPalette{ colorPalette }
	, mp_camera{nullptr}
	, m_mut{}
	, m_device{ -1 }
{
}

SeekCamera::~SeekCamera()
{
	disconnect();
}

seekcamera_error_t SeekCamera::connect(seekcamera_t* p_camera)
{
	disconnect();
	m_devicePath = devicePath;
	mp_camera = p_camera;
	// Register a frame available callback function.
	seekcamera_error_t status = seekcamera_register_frame_available_callback(mp_camera, [](seekcamera_t*, seekcamera_frame_t* p_cameraframe, void* p_userData) {
		auto* p_cameraData = (SeekCamera*)p_userData;
		p_cameraData->handleCameraFrameAvailable(p_cameraFrame);
		}, (void*)this);
	if (status == SEEKCAMERA_SUCCESS)
	{
		// set pipeline mode to SeekVision
		status = seekcamera_set_pipeline_mode(mp_camera, SEEKCAMERA_IMAGE_SEEKVISION);
		if (status == SEEKCAMERA_SUCCESS)
		{
			// Start the capture session.
			status = seekcamera_capture_session_start(mp_camera, m_format);
			if (status == SEEKCAMERA_SUCCESS)
			{
				status = seekcamera_set_color_palette(mp_camera, m_colorPalette);
				if (status != SEEKCAMERA_SUCCESS)
				{
					::std::cout << "failed to set color palette to " << seekcamera_color_palette_get_str(m_colorPalette) << ": " << seekcamera_error_get_str(status) << std::endl;
				}
			}
			else
			{
				std::cerr << "failed to start capture session: " << seekcamera_error_get_str(status) << std::endl;

			}
		}
		else
		{
			std::cerr << "failed to set image pipeline mode: " << seekcamera_error_get_str(status) << std::endl;
		}
	}
	else
	{
		std::cerr << "failed to register frame callback: " << seekcamera_error_get_str(status) << std::endl;
	}
	return status;
}

seekcamera_error_t SeekCamera::disconnect()
{
	seekcamera_error_t status = SEEKCAMERA_SUCCESS;
	if (mp_camera)
	{
		status = seekcamera_capture_session_stop(mp_camera);
		if (status != SEEKCAMERA_SUCCESS)
		{
			std::cerr << "failed to stop capture session: " << seekcamera_error_get_str(status) << std::endl;
		}
		mp_camera = nullptr;
	}
	if (m_device >= 0)
	{
		close(m_device);
		m_device = -1;
	}
	return status;
}

seekcamera_error_t SeekCamera::handleReadyToPair(seekcamera_t* p_camera)
{
	// Attempt to pair the camera automatically.
	// Pairing refers to the process by which the sensor is associated with the host and the embedded processor.
	seekcamera_error_t status = seekcamera_store_calibration_data(p_camera, nullptr, nullptr, nullptr);
	if (status != SEEKCAMERA_SUCCESS)
	{
		std::cerr << "failed to pair device: " << seekcamera_error_get_str(status) << std::endl;
	}
	// Start imaging.
	status = connect(p_camera);
	return status;
}

int SeekCamera::openDevice(int width, int height)
{
	//TODO find a way to detect the format automatically
#ifdef __linux__
	int returnCode = 0;
	m_device = open(m_devicePath.c_str(), O_RDWR);
	if (m_device < 0)
	{
		returnCode = m_device;
		std::cerr << "Error opening v4l2 device: " << strerror(errno) << " on device with path '" << m_devicePath << "'" << std::endl;
	}
	else
	{
		struct v4l2_format v;
		v.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		returnCode = ioctl(m_device, VIDIOC_G_FMT, &v);
		if (returnCode < 0)
		{
			std::cerr << "VIDIOC_G_FMT error: " << strerror(errno) << " on device with path '" << m_devicePath << "'" << std::endl;
		}
		else
		{
			v.fmt.pix.width = width;
			v.fmt.pix.height = height;
			switch (m_format)
			{
			case SEEKCAMERA_FRAME_FORMAT_COLOR_YUY2:
				v.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
				v.fmt.pix.sizeimage = width * height * 2;
				break;
			default:
				std::cerr << "Unknown format " << m_format << ::std::endl;
				returnCode = -1;
				break;
			}

			if(returnCode>=0)
			{
				returnCode = ioctl(m_device, VIDIOC_S_FMT, &v);
				if (returnCode < 0)
				{
					std::cerr << "VIDIOC_S_FMT error: " << strerror(errno) << " on device with path '" << m_devicePath << "'" << std::endl;
				}
				else
				{
					std::cout << "Opened v4l2 device with path '" << m_devicePath << "'" << std::endl;
				}
			}
		}
	}

#else
	int returnCode = = -1;
	std::cout << "v4l2 is not supported on this platform" << std::endl;
#endif
	return returnCode;
}

void SeekCamera::handleCameraFrameAvailable(seekcamera_frame_t* p_cameraframe)
{
	// This mutex is used to serialize access to the frame member of the renderer.
	// In the future the single frame member should be replaced with something like a FIFO
	std::lock_guard<std::mutex> guard(m_mut);
	// Lock the seekcamera frame for safe use outside of this callback.
	seekcamera_frame_lock(p_cameraframe);
	// Get the frame to draw.
	seekframe_t* p_frame = nullptr;
	seekcamera_error_t status = seekcamera_frame_get_frame_by_format(p_cameraframe, m_format, &p_frame);
	if (status == SEEKCAMERA_SUCCESS)
	{
		if (m_device < 0)
		{
			int const frameWidth = (int)seekframe_get_width(p_frame);
			int const frameHeight = (int)seekframe_get_height(p_frame);
			openDevice(frameWidth, frameHeight);
		}
		if (m_device >= 0)
		{
			int written = write(m_device, seekframe_get_data(p_frame), seekframe_get_data_size(p_frame));
			if (written < 0)
			{
				std::cerr << "Error writing v4l2 device : " << strerror(errno) << " on device with path '" << m_devicePath << "'" << std::endl;
			}
		}
	}
	else
	{
		std::cerr << "failed to get frame: " << seekcamera_error_get_str(status) << std::endl;
	}
	seekcamera_frame_unlock(p_cameraframe);
}






class SeekCameraLoopHandler
{
public:
	SeekCameraLoopHandler();
	~SeekCameraLoopHandler();

	seekcamera_error_t start(
		std::unordered_map< std::string, SeekCamera> cameraMap
		, std::string defaultDeviceName = "/dev/video2"
		, seekcamera_frame_format_t defaultFormat = SEEKCAMERA_FRAME_FORMAT_COLOR_YUY2
	    , seekcamera_color_palette_t defaultColorPalette = SEEKCAMERA_COLOR_PALETTE_WHITE_HOT);
	void stop();


private:

	static void cameraEventCallback(seekcamera_t* p_camera, seekcamera_manager_event_t event, seekcamera_error_t eventStatus, void* p_userData);


	//chip ids to cameras
	std::unordered_map< std::string, SeekCamera> m_cameraMap;
	std::string m_defaultDeviceName;
	seekcamera_color_palette_t m_defaultColorPalette;
	seekcamera_frame_format_t m_defaultFormat;
	seekcamera_manager_t* mp_cameraManager;
};


SeekCameraLoopHandler::SeekCameraLoopHandler()
	: m_cameraMap{}
	, m_defaultDeviceName{}
	, m_defaultFormat{ }
	, mp_cameraManager{nullptr}
{

}

SeekCameraLoopHandler::~SeekCameraLoopHandler()
{
	stop();
}

seekcamera_error_t SeekCameraLoopHandler::start(std::unordered_map< std::string, SeekCamera> cameraMap, std::string defaultDeviceName, seekcamera_frame_format_t defaultFormat, seekcamera_color_palette_t defaultColorPalette)
{
	m_cameraMap = ::std::move(cameraMap);
	m_defaultDeviceName = ::std::move(defaultDeviceName);
	m_defaultFormat = defaultFormat;
	m_defaultColorPalette = defaultColorPalette;
	stop();
	std::cout << "seekcamera_capture prototype starting" << std::endl;
	seekcamera_error_t status = seekcamera_manager_create(&mp_cameraManager, SEEKCAMERA_IO_TYPE_USB);
	if (status == SEEKCAMERA_SUCCESS)
	{
		// Register an event handler for the camera manager to be called whenever a camera event occurs.
		status = seekcamera_manager_register_event_callback(mp_cameraManager, cameraEventCallback, (void*)this);
		if (status != SEEKCAMERA_SUCCESS)
		{
			std::cerr << "failed to register camera event callback: " << seekcamera_error_get_str(status) << std::endl;
		}
	}
	else
	{
		std::cerr << "failed to create camera manager: " << seekcamera_error_get_str(status) << std::endl;
	}
	return status;
}

void SeekCameraLoopHandler::stop()
{
	if (mp_cameraManager)
	{
		std::cout << "seekcamera_capture prototype stopping" << std::endl;
		// Teardown the camera manager.
		seekcamera_manager_destroy(&mp_cameraManager);

		for (auto const& [id, camera] : m_cameraMap)
		{
			camera.disconnect();
		}
		mp_cameraManager = nullptr;
	}
}


void SeekCameraLoopHandler::cameraEventCallback(seekcamera_t* p_camera, seekcamera_manager_event_t event, seekcamera_error_t eventStatus, void* p_userData)
{
	seekcamera_chipid_t cid{};
	seekcamera_get_chipid(camera, &cid);
	std::cout << seekcamera_manager_get_event_str(event) << " (CID: " << cid << ")" << std::endl;
	seekcamera_chipid_t cid{};
	seekcamera_get_chipid(p_camera, &cid);
	SeekCameraLoopHandler* p_loopHandler = (SeekCameraLoopHandler*)p_userData;
	std::string chipId((char*)&cid);
	auto cameraItr = p_loopHandler->m_cameraMap.find(chipId);
	if (cameraItr == ::std::end(m_cameraMap))
	{
		std::cerr << "unkown camera for event: (CID: " << cid << ")" << seekcamera_error_get_str(eventStatus) << std::endl;

		SeekCamera seekCamera(m_defaultDeviceName, m_defaultFormat);
		cameraItr = m_cameraMap.insert(chipId, ::std::move(seekCamera));
	}
	{
		SeekCamera& seekCamera = cameraItr->second;
		switch (event)
		{
		case SEEKCAMERA_MANAGER_EVENT_CONNECT:
			seekCamera.connect(p_camera);
			break;
		case SEEKCAMERA_MANAGER_EVENT_DISCONNECT:
			seekCamera.disconnect();
			break;
		case SEEKCAMERA_MANAGER_EVENT_ERROR:
			std::cerr << "unhandled camera error: (CID: " << cid << ")" << seekcamera_error_get_str(eventStatus) << std::endl;
			break;
		case SEEKCAMERA_MANAGER_EVENT_READY_TO_PAIR:
			seekCamera.handleReadyToPair(p_camera);
			break;
		default:
			break;
		}
	}
}

SeekCameraLoopHandler g_loopHandler;

void sighandler(int signum)
{
	std::cout << std::endl;
	std::cout << "Caught termination signal";
	std::cout << std::endl;
	g_loopHandler.stop();
}

int main()
{
	//TODO read in a map of cameras and devices, probably from JSON
	::std::unordered_map<::std::string, SeekCamera> cameraMap{};

	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);
	auto status = g_loopHandler.start(::std::move(cameraMap));
	if (status == SEEKCAMERA_SUCCESS)
	{
		std::cout << "press enter to quit" << ::std::endl;
		for (;;)
		{
			int c = getchar();
			if (c == '\n')
			{
				break;
			}
		}
	}
	return 0;
}

