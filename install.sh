# install all the stuff you'll need (dkms, cmake, boost, gstreamer)
apt update
apt install -y dkms cmake libboost-all-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio
# download the v4l2loopback stuff and place it into /usr/src
version=0.12.5
curl -L https://github.com/umlaeute/v4l2loopback/archive/v${version}.tar.gz | tar xvz -C /usr/src
# copy seek-thermal libs and include files
cp lib/* /usr/local/lib
cp -r include/* /usr/local/include
# copy the device rules
cp driver/udev/10-seekthermal.rules /etc/udev/rules.d
# reload your library path
ldconfig
# reload your device rules
udevadm control --reload 
# clean up the build directory if it exists
rm -rf build
mkdir build
cd build
# build the application and install it
cmake ..
make
make install
# build and install the v4l2loopback module
dkms add -m v4l2loopback -v ${version}
dkms build -m v4l2loopback -v ${version}
dkms install -m v4l2loopback -v ${version}
# ensure the module loads on startup
echo "v4l2loopback" > /etc/modules-load.d/v4l2loopback.conf
# clean up the v4l2loopback source
rm -rf /usr/src/v4l2loopback-${version}
# Copy over the auto-start script 
#mkdir -p /usr/local/echopilot/scripts
#cp ../seekcamera_capture_autostart.sh /usr/local/echopilot/scripts
#chmod +x /usr/local/echopilot/scripts/seekcamera_capture_autostart.sh

cp ../config.json /usr/local/bin

cp ../seekcamera_capture.service /etc/systemd/system/
systemctl start seekcamera_capture
systemctl enable seekcamera_capture