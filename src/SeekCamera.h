#pragma once
#include <string>
#include <mutex>
#include "seekcamera/seekcamera.h"

class SeekCamera
{
public:
	SeekCamera(std::string const& devicePath, seekcamera_frame_format_t format, seekcamera_color_palette_t colorPalette);
	~SeekCamera();
    SeekCamera(SeekCamera const& that)=delete;
    SeekCamera(SeekCamera&& that);
    SeekCamera& operator=(SeekCamera const& that)=delete;
    SeekCamera& operator=(SeekCamera&& that)=delete;


	seekcamera_error_t connect(seekcamera_t* p_camera);
	seekcamera_error_t disconnect();
	seekcamera_error_t handleReadyToPair(seekcamera_t* p_camera);
	//5 timeouts in a row, reconnect the camera
	//increment the counter. When it reaches 5, disconnect and reconnect
	void recordTimeout();
	//reset the counter
	void resetTimeouts();

	std::string const& getDevicePath() const;
	seekcamera_frame_format_t getFormat() const;
	seekcamera_color_palette_t getColorPalette() const;

private:
	seekcamera_error_t openSession();
	seekcamera_error_t closeSession();
	int openDevice(int frameWidht, int frameHeight);
	void handleCameraFrameAvailable(seekcamera_frame_t* p_cameraframe);


	std::string m_devicePath;
	seekcamera_frame_format_t m_format;
	seekcamera_color_palette_t m_colorPalette;
	seekcamera_t* mp_camera;
	std::mutex m_mut;
	int m_device;
	int m_timeoutCount;
};