#--------------------------------------------------------------------------------------------------------------------------#
#Project configuration
#--------------------------------------------------------------------------------------------------------------------------#
project(seekcamera_capture DESCRIPTION "Seek Thermal Capture Prototype")

#--------------------------------------------------------------------------------------------------------------------------#
#Executable configuration
#--------------------------------------------------------------------------------------------------------------------------#
add_executable(${PROJECT_NAME}
	src/main.cpp
	src/SeekCamera.cpp
	src/SeekCameraLoopHandler.cpp
)

target_compile_features(${PROJECT_NAME} 
	PRIVATE cxx_std_17
)


target_link_libraries(${PROJECT_NAME}
	seekcamera
)


if(UNIX)
	target_link_libraries(${PROJECT_NAME} m)
endif()

#--------------------------------------------------------------------------------------------------------------------------#
#Runtime dependencies
#--------------------------------------------------------------------------------------------------------------------------#
if(MSVC)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE_DIR:seekcamera>/seekcamera.dll
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/seekcamera.dll
	)
endif()

#--------------------------------------------------------------------------------------------------------------------------#
#Install
#--------------------------------------------------------------------------------------------------------------------------#
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
