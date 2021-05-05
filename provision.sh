apt-get update -y
apt-get install -y git
apt-get install -y cmake
apt-get install -y autoconf
apt-get install -y libtbb-dev
# RUN apt-get install -y libhdf3-dev
apt-get install -y libbz2-dev
apt-get install -y zlib1g-dev
apt-get install -y libxml2-dev
apt-get install -y libcurl4-openssl-dev
apt-get install -y libboost-all-dev
apt-get install -y clang
apt-get install -y htop

echo "disk=/var/tmp/thrilldisk,0,linuxaio" > /home/vagrant/.thrill
chmod 777 /home/vagrant/.thrill
