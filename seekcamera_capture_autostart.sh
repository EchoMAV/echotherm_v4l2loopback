#! /bin/bash

# check if a seek thermal camera is plugged in. Their vendor ID is 289d.
if [ "$(lsusb | grep -c "289d:")" -ge 1 ]; then
	# check if the process is running
	case "$(pidof seekcamera_capture | wc -w)" in
		0) # restart the process
			seekcamera_capture &
			;;
		1) # all ok
			;;
		*) # kill duplicate processes
			kill -9 $(pidof seekcamera_capture | awk '{print $1}')
	    		;;
	esac
fi
