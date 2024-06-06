# seekcamera_capture

A prototype used to demonstrate the Seek Thermal camera. Currently hard-coded to /dev/video2.

You need to set up a v4l2loopback dummy device
https://github.com/umlaeute/v4l2loopback

$ sudo modprobe v4l2loopback

The following assumes that your dummy device is named video0. It might be named video1, or video2. 

Verify your dummy device
$ v4l2-ctl -d /dev/video0 --all

Ensure that the dimensions are 320x240 and the pixel format is 'YUYV' (YUYV 4:2:2)

Run the application.

In another terminal, open a gstreamer pipeline with

$ gst-launch-1.0 v4l2src device=/dev/video0 ! "video/x-raw,format=(string)YUY2,width=(int)320,height=(int)240,framerate=(fraction)30/1" ! queue ! videoconvert !  x264enc tune=zerolatency speed-preset=ultrafast bitrate=4000 ! rtph264pay config-interval=1 pt=96 ! udpsink host=127.0.0.1 port=5600 sync=false

This will set the dummy device as the source. convert to I420, and then encode to x264.
Then it will pass it to rtph264pay to encode it into RTP packets.
Then it will pass it into a udpsink on Port 5600.

QGroundControl will be able to listen on this port and capture the video


## Building

Please refer to the SDK C Programming Guide for details.

## Usage

To run the application, call the application from the command-line.

You can give it the command-line option "--configFile <FILENAME>" to point it to a configuration

An example configuation JSON file:

{
    "camera_array": [
        {
            "cid": "E452AFB41114",
            "device_path": "\/dev\/video0",
            "format": "1024",
            "color_palette": "2"
        }
    ]
}


This will open the camera with the chip-ID E452AFB41114 and set it's format to YUY2. The color palette will be set to "spectra"


Any cameras not picked up on the configuration will be defaulted to /dev/video0, YUY2 format, and color palette 'white-hot'

