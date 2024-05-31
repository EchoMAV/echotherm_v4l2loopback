# seekcamera_capture

A prototype used to demonstrate the Seek Thermal camera. Currently hard-coded to /dev/video2.

You need to set up a v4l2loopback dummy device
https://github.com/umlaeute/v4l2loopback

$ sudo modprobe v4l2loopback

You then need to force the format
v4l2loopback-ctl set-caps "video/x-raw, format=YUY2, width=320, height=240" /dev/video2

Verify your dummy device
v4l2-ctl -d /dev/video2 --all

Ensure that the dimensions are 320x240 and the pixel format is 'YUYV' (YUYV 4:2:2)

Run the application.

In another terminal, open a gstreamer pipeline with

gst-launch-1.0 v4l2src device=/dev/video2 ! "video/x-raw,format=(string)YUY2,width=(int)320,height=(int)240,framerate=(fraction)30/1" ! queue ! videoconvert ! x264enc tune=zerolatency speed-preset=ultrafast bitrate=4000 ! rtph264pay config-interval=1 pt=96 ! udpsink host=127.0.0.1 port=5600 sync=false


This will set the dummy device as the source. convert to I420, and then encode to x264.
Then it will pass it to rtph264pay to encode it into RTP packets.
Then it will pass it into a udpsink on Port 5600.

QGroundControl will be able to listen on this port and capture the video


## Building

Please refer to the SDK C Programming Guide for details.

## Usage

To run the application, call the application from the command-line.

## User controls

### Color palette
The color palette can be switched by pressing the mouse keys down in any area of the graphical user interface.

### Quit
The application can be exited by pressing the `q` key on any active window.
