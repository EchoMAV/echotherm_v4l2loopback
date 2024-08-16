#! /bin/bash
systemctl stop seekcamera_capture&
systemctl disable seekcamera_capture
rm /etc/systemd/system/seekcamera_capture.service
rm /usr/local/bin/seekcamera_capture_config.json
rm /usr/local/bin/seekcamera_capture
rm /etc/udev/rules.d/10-seekthermal.rules
rm -rf /usr/local/include/seekframe
rm -rf /usr/local/include/seekcamera
rm /usr/local/lib/libseekcamera.so.4.4
rm /usr/local/lib/libseekcamera.so
