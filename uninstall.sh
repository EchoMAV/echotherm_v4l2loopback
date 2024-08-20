#! /bin/bash
systemctl stop echotherm_v4l2loopback.service&
systemctl disable echotherm_v4l2loopback.service
rm /etc/systemd/system/echotherm_v4l2loopback.service
rm /usr/local/bin/echotherm_v4l2loopback_config.json
rm /usr/local/bin/echotherm_v4l2loopback
rm /etc/udev/rules.d/10-seekthermal.rules
rm -rf /usr/local/include/seekframe
rm -rf /usr/local/include/seekcamera
rm /usr/local/lib/libseekcamera.so.4.4
rm /usr/local/lib/libseekcamera.so
