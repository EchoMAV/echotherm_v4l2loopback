#include "EchoThermCameraLoopHandler.h"

#include "EchoThermCamera.h"
#include <iostream>


EchoThermCameraLoopHandler::EchoThermCameraLoopHandler()
	: m_cameraMap{}
	, m_defaultDeviceName{}
    , m_defaultColorPalette{SEEKCAMERA_COLOR_PALETTE_WHITE_HOT}
	, m_defaultShutterMode{SEEKCAMERA_SHUTTER_MODE_AUTO}
	, m_defaultFormat{ SEEKCAMERA_FRAME_FORMAT_COLOR_YUY2}
	, mp_cameraManager{nullptr}
{
}

EchoThermCameraLoopHandler::~EchoThermCameraLoopHandler()
{
	stop();
}

EchoThermCameraLoopHandler::EchoThermCameraLoopHandler(EchoThermCameraLoopHandler&& that)
    : m_cameraMap{std::move(that.m_cameraMap)}
    , m_defaultDeviceName{std::move(that.m_defaultDeviceName)}
    , m_defaultColorPalette{std::move(that.m_defaultColorPalette)}
	, m_defaultShutterMode{std::move(that.m_defaultShutterMode)}
    , m_defaultFormat{std::move(that.m_defaultFormat)}
    , mp_cameraManager{that.mp_cameraManager}
{
    that.mp_cameraManager=nullptr;
}

void EchoThermCameraLoopHandler::setDefaultColorPalette(seekcamera_color_palette_t const colorPalette)
{
	m_defaultColorPalette=colorPalette;
}

void EchoThermCameraLoopHandler::setDefaultShutterMode(seekcamera_shutter_mode_t const shutterMode)
{
	m_defaultShutterMode=shutterMode;
}

void EchoThermCameraLoopHandler::setDefaultDeviceName(std::string const& deviceName)
{
	m_defaultDeviceName=deviceName;
}
void EchoThermCameraLoopHandler::setDefaultFrameFormat(seekcamera_frame_format_t const frameFormat)
{
	m_defaultFormat=frameFormat;
}

seekcamera_error_t EchoThermCameraLoopHandler::start(std::unordered_map< std::string, EchoThermCamera> cameraMap)
{
	m_cameraMap = ::std::move(cameraMap);
	stop();
	std::cout << "echotherm_v4l2loopback starting" << std::endl;
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

void EchoThermCameraLoopHandler::setConfigFile(std::string const& configFile)
{
	m_configFile=configFile;
}

void EchoThermCameraLoopHandler::stop()
{
	if (mp_cameraManager)
	{
		std::cout << "echotherm_v4l2loopback stopping" << std::endl;
		// Teardown the camera manager.
		seekcamera_manager_destroy(&mp_cameraManager);

		for (auto& [id, camera] : m_cameraMap)
		{
			camera.disconnect();
		}
		mp_cameraManager = nullptr;
	}
}


void EchoThermCameraLoopHandler::cameraEventCallback(seekcamera_t* p_camera, seekcamera_manager_event_t event, seekcamera_error_t eventStatus, void* p_userData)
{
	seekcamera_chipid_t cid{};
	seekcamera_get_chipid(p_camera, &cid);
	std::cout << seekcamera_manager_get_event_str(event) << " (CID: " << cid << ")" << std::endl;
	seekcamera_get_chipid(p_camera, &cid);
	EchoThermCameraLoopHandler* p_loopHandler = (EchoThermCameraLoopHandler*)p_userData;
	std::string chipId((char*)&cid);
	auto cameraItr = p_loopHandler->m_cameraMap.find(chipId);
	if (cameraItr == ::std::end(p_loopHandler->m_cameraMap))
	{
		std::cerr << "unknown camera for event: (CID: " << cid << "): " << seekcamera_error_get_str(eventStatus) << std::endl;

		std::string deviceName;
		if(p_loopHandler->m_defaultDeviceName.empty())
		{
			deviceName="/dev/video0";
		}
		else
		{
			deviceName=p_loopHandler->m_defaultDeviceName;
		}
		EchoThermCamera echoThermCamera(deviceName, p_loopHandler->m_defaultFormat, p_loopHandler->m_defaultColorPalette, p_loopHandler->m_defaultShutterMode);
		cameraItr = p_loopHandler->m_cameraMap.emplace(chipId, ::std::move(echoThermCamera)).first;
	}
	{
		EchoThermCamera& echoThermCamera = cameraItr->second;
		switch (event)
		{
		case SEEKCAMERA_MANAGER_EVENT_CONNECT:
			echoThermCamera.connect(p_camera);
			break;
		case SEEKCAMERA_MANAGER_EVENT_DISCONNECT:
			echoThermCamera.disconnect();
			break;
		case SEEKCAMERA_MANAGER_EVENT_ERROR:
			std::cerr << "unhandled camera error: (CID: " << cid << ")" << seekcamera_error_get_str(eventStatus) << std::endl;
			if(std::filesystem::exists(p_loopHandler->m_configFile))
			{
				//touch the config file to force the manager to re-load itself
				std::error_code ec;
				std::filesystem::last_write_time(p_loopHandler->m_configFile, std::filesystem::file_time_type::clock::now(), ec);
				if(ec)
				{
					std::cerr <<"Error while touching config file "<<p_loopHandler->m_configFile<<": "<<ec<<std::endl;
				}
			}
			break;
		case SEEKCAMERA_MANAGER_EVENT_READY_TO_PAIR:
			echoThermCamera.handleReadyToPair(p_camera);
			break;
		default:
			break;
		}
	}
}