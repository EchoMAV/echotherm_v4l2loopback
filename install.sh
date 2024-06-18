apt update
apt install -y dkms cmake libboost-all-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio
version=0.12.5
curl -L https://github.com/umlaeute/v4l2loopback/archive/v${version}.tar.gz | tar xvz -C /usr/src
cp lib/* /usr/local/lib
cp -r include/* /usr/local/include
cp driver/udev/10-seekthermal.rules /etc/udev/rules.d
ldconfig
udevadm control --reload 
rm -rf build
mkdir build
cd build
cmake ..
make
make install
dkms add -m v4l2loopback -v ${version}
dkms build -m v4l2loopback -v ${version}
dkms install -m v4l2loopback -v ${version}
echo "v4l2loopback" > /etc/modules-load.d/v4l2loopback.conf
modprobe vl42loopback
rm -rf /usr/src/v4l2loopback-${version}
