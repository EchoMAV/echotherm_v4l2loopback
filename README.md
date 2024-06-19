Assuming everything is set up on the Raspberry Pi, you can install seekcamera_capture using this set of commands:
git clone https://github.com/EchoMAV/seekcamera_capture.git /tmp/seekcamera_capture && cd /tmp/seekcamera_capture && sudo ./install.sh

Connect the camera and reboot.

A cronjob will be added to will check if the camera is plugged in once a minute. If it is, it will check if the process is running. If not, it will start it




