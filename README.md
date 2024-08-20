Assuming everything is set up on the Raspberry Pi, you can install echotherm_v4l2loopback using this set of commands:
git clone https://github.com/EchoMAV/echotherm_v4l2loopback.git /tmp/echotherm_v4l2loopback && cd /tmp/echotherm_v4l2loopback && sudo ./install.sh

Connect the camera and reboot.

To change the camera settings, including the color palette, simply modify the /usr/local/bin/echotherm_v4l2loopback_config.json file 

To uninstall, run sudo ./uninstall.sh


The service runs in the background and waits for a Seek thermal camera to be connected.

When it is connected, it begins pulling frames from the camera and feeding them into a v4l2 loopback device.

This v4l2loopback device can then be used in a gstreamer pipeline like this


gst-launch-1.0 v4l2src device=/dev/video0 ! videoconvert ! x264enc tune=zerolatency speed-preset=ultrafast bitrate=4000 ! rtph264pay config-interval=1 pt=96 ! udpsink host=192.168.1.98 port=5600 sync=false


