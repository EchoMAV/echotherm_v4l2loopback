#pragma once
#include "SeekCamera.h"

#include <unordered_map>
#include <string>
#include "seekcamera/seekcamera.h"
#include "seekcamera/seekcamera_manager.h"



class SeekCameraLoopHandler
{
public:
	SeekCameraLoopHandler();
	~SeekCameraLoopHandler();
    SeekCameraLoopHandler(SeekCameraLoopHandler const& that)=delete;
    SeekCameraLoopHandler(SeekCameraLoopHandler&& that);
    SeekCameraLoopHandler& operator=(SeekCameraLoopHandler const& that)=delete;
    SeekCameraLoopHandler& operator=(SeekCameraLoopHandler&& that)=delete;


	seekcamera_error_t start(
		std::unordered_map< std::string, SeekCamera> cameraMap
		, std::string defaultDeviceName = "/dev/video0"
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
