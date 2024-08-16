#pragma once
#include "SeekCamera.h"

#include <unordered_map>
#include <string>
#include "seekcamera/seekcamera.h"
#include "seekcamera/seekcamera_manager.h"
#include <filesystem>



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
		, seekcamera_frame_format_t defaultFormat = SEEKCAMERA_FRAME_FORMAT_COLOR_YUY2);

	void stop();

	void setConfigFile(std::string const& configFile);


	void setDefaultColorPalette(seekcamera_color_palette_t const colorPalette);
	void setDefaultShutterMode(seekcamera_shutter_mode_t const shutterMode);


private:

	static void cameraEventCallback(seekcamera_t* p_camera, seekcamera_manager_event_t event, seekcamera_error_t eventStatus, void* p_userData);


	//chip ids to cameras
	std::unordered_map< std::string, SeekCamera> m_cameraMap;
	std::string m_defaultDeviceName;
	seekcamera_color_palette_t m_defaultColorPalette;
	seekcamera_shutter_mode_t m_defaultShutterMode;
	seekcamera_frame_format_t m_defaultFormat;
	seekcamera_manager_t* mp_cameraManager;
	std::filesystem::path m_configFile;
};
