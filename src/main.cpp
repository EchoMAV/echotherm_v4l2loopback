#include "SeekCameraLoopHandler.h"
#include "SeekCamera.h"

#include <iostream>

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

