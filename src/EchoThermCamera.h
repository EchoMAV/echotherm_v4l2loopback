#pragma once
#include <string>
#include <mutex>
#include "seekcamera/seekcamera.h"

class EchoThermCamera
{
public:
	EchoThermCamera(std::string const& devicePath, seekcamera_frame_format_t format, seekcamera_color_palette_t colorPalette, seekcamera_shutter_mode_t shutterMode);
	~EchoThermCamera();
    EchoThermCamera(EchoThermCamera const& that)=delete;
    EchoThermCamera(EchoThermCamera&& that);
    EchoThermCamera& operator=(EchoThermCamera const& that)=delete;
    EchoThermCamera& operator=(EchoThermCamera&& that)=delete;


	seekcamera_error_t connect(seekcamera_t* p_camera);
	seekcamera_error_t disconnect();
	seekcamera_error_t handleReadyToPair(seekcamera_t* p_camera);
	//increment the timeout counter
	int recordTimeout();
	//reset the counter
	void resetTimeouts();

	std::string const& getDevicePath() const;
	seekcamera_frame_format_t getFormat() const;
	seekcamera_color_palette_t getColorPalette() const;
	seekcamera_shutter_mode_t getShutterMode() const;

	seekcamera_error_t triggerShutter();

private:
	seekcamera_error_t openSession(bool reconnect);
	seekcamera_error_t closeSession();
	int openDevice(int frameWidht, int frameHeight);
	void handleCameraFrameAvailable(seekcamera_frame_t* p_cameraframe);


	std::string m_devicePath;
	seekcamera_frame_format_t m_format;
	seekcamera_color_palette_t m_colorPalette;
	seekcamera_shutter_mode_t m_shutterMode;
	seekcamera_t* mp_camera;
	std::mutex m_mut;
	int m_device;
	int m_timeoutCount;
};