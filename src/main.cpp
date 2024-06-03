#include "SeekCameraLoopHandler.h"
#include "SeekCamera.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <boost/property_tree/json_parser.hpp>
#include <boost/program_options.hpp>

// C includes
#include <csignal>



SeekCameraLoopHandler g_loopHandler;

void sighandler(int signum)
{
	std::cout << std::endl;
	std::cout << "Caught termination signal";
	std::cout << std::endl;
	g_loopHandler.stop();
}



void writeConfig(::std::filesystem::path const& path, ::std::unordered_map<::std::string, SeekCamera> & cameraMap)
{
	boost::property_tree::ptree properties;
	boost::property_tree::ptree cameraArray;
	for(auto const& [cid, camera] : cameraMap)
	{
		boost::property_tree::ptree cameraProps;
		cameraProps.put("cid", cid);
		cameraProps.put("device_path", camera.getDevicePath());
		cameraProps.put("format", camera.getFormat());
		cameraProps.put("color_palette", camera.getColorPalette());
		cameraArray.push_back(::std::make_pair("", cameraProps));
	}
	properties.add_child("camera_array", cameraArray);

	::std::ofstream outputStream(path);
	boost::property_tree::write_json(outputStream, properties, true);

}



int readConfig(std::filesystem::path const& configFilePath, ::std::unordered_map<::std::string, SeekCamera> & cameraMap)
{
	int returnCode=1;
	if(std::filesystem::exists(configFilePath)
		&& std::filesystem::is_regular_file(configFilePath))
	{
		boost::property_tree::ptree properties;
		try
		{
			
			::std::ifstream inputStream(configFilePath);
			boost::property_tree::read_json(inputStream,properties);

			auto cameraArray = properties.get_child("camera_array");

			for(auto const& [key,cameraProps] : cameraArray)
			{
				auto cid = cameraProps.get<std::string>("cid");
				auto devicePath = cameraProps.get<std::string>("device_path");
				auto format = (seekcamera_frame_format_t)cameraProps.get<int>("format");
				auto colorPalette = (seekcamera_color_palette_t)cameraProps.get<int>("color_palette");

				SeekCamera camera(devicePath, format, colorPalette);
				cameraMap.emplace(cid, ::std::move(camera));
				

			}
			returnCode=0;
		}
		catch(...)
		{
			std::cerr<<"exception while reading configFile "<<configFilePath.string()<<std::endl;
		}
	}
	return returnCode;
}


int main(int argc, char* argv[])
{



	std::unordered_map<::std::string, SeekCamera> cameraMap{};
	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("configFile", boost::program_options::value<std::filesystem::path>(), "choose the config file path");
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	boost::program_options::notify(vm);

	if(vm.count("help"))
	{
		std::cout<<desc<<::std::endl;
	}
	else if(vm.count("configFile"))
	{
		//TODO read in a map of cameras and devices, probably from JSON
		std::filesystem::path configFilePath = vm["configFile"].as<std::filesystem::path>();
		std::cout<<"configFile = "<<configFilePath<<std::endl;
		if(readConfig(configFilePath, cameraMap))
		{
			cameraMap.clear();
			//SeekCamera seekCamera("/dev/video2", SEEKCAMERA_FRAME_FORMAT_COLOR_YUY2, SEEKCAMERA_COLOR_PALETTE_WHITE_HOT);
			//cameraMap.emplace("E452AFB41114", ::std::move(seekCamera)).first;
			//std::cout<<"writing to config file "<<std::filesystem::absolute(configFilePath)<<std::endl;
			//writeConfig(configFilePath, cameraMap);
		}
	}
	else
	{
		std::cout<<"no options found"<<std::endl;
	}

	
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

