#pragma once
#include "EchoThermCamera.h"

#include <unordered_map>
#include <string>
#include "seekcamera/seekcamera.h"
#include "seekcamera/seekcamera_manager.h"
#include <filesystem>

class EchoThermCameraLoopHandler
{
public:
	EchoThermCameraLoopHandler();
	~EchoThermCameraLoopHandler();
    EchoThermCameraLoopHandler(EchoThermCameraLoopHandler const& that)=delete;
    EchoThermCameraLoopHandler(EchoThermCameraLoopHandler&& that);
    EchoThermCameraLoopHandler& operator=(EchoThermCameraLoopHandler const& that)=delete;
    EchoThermCameraLoopHandler& operator=(EchoThermCameraLoopHandler&& that)=delete;


	seekcamera_error_t start(
		std::unordered_map< std::string, EchoThermCamera> cameraMap);

	void stop();

	void setConfigFile(std::string const& configFile);


	void setDefaultColorPalette(seekcamera_color_palette_t const colorPalette);
	void setDefaultShutterMode(seekcamera_shutter_mode_t const shutterMode);
	void setDefaultDeviceName(std::string const& deviceName);
	void setDefaultFrameFormat(seekcamera_frame_format_t const frameFormat);

private:

	static void cameraEventCallback(seekcamera_t* p_camera, seekcamera_manager_event_t event, seekcamera_error_t eventStatus, void* p_userData);


	//chip ids to cameras
	std::unordered_map< std::string, EchoThermCamera> m_cameraMap;
	std::string m_defaultDeviceName;
	seekcamera_color_palette_t m_defaultColorPalette;
	seekcamera_shutter_mode_t m_defaultShutterMode;
	seekcamera_frame_format_t m_defaultFormat;
	seekcamera_manager_t* mp_cameraManager;
	std::filesystem::path m_configFile;
};
