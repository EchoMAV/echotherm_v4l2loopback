#--------------------------------------------------------------------------------------------------------------------------#
#Project configuration
#--------------------------------------------------------------------------------------------------------------------------#
cmake_minimum_required(VERSION 3.16)
project(echotherm_v4l2loopback DESCRIPTION "EchoTherm V4L2Loopback Capture")


find_package(Boost REQUIRED COMPONENTS system program_options CONFIG) 

#--------------------------------------------------------------------------------------------------------------------------#
#Executable configuration
#--------------------------------------------------------------------------------------------------------------------------#
add_executable(${PROJECT_NAME}
	src/main.cpp
	src/EchoThermCamera.cpp
	src/EchoThermCameraLoopHandler.cpp
)

target_compile_features(${PROJECT_NAME} 
	PRIVATE cxx_std_17
)

target_link_libraries(${PROJECT_NAME}
	seekcamera
	Boost::system
	Boost::program_options
)

#--------------------------------------------------------------------------------------------------------------------------#
#Install
#--------------------------------------------------------------------------------------------------------------------------#
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
