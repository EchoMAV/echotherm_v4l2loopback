apt update
apt install -y libboost-all-dev
apt install -y cmake
apt install -y dkms
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
