#!/bin/bash
echo "x64 dependencies builder 1.0.0"

zlibArchive='https://github.com/madler/zlib/releases/download/v1.2.13/zlib-1.2.13.tar.xz'
caresArchive='https://github.com/c-ares/c-ares/releases/download/cares-1_19_0/c-ares-1.19.0.tar.gz'
wolfSSLArchive='https://github.com/wolfSSL/wolfssl/archive/refs/tags/v5.6.0-stable.tar.gz'
nghttp2Archive='https://github.com/nghttp2/nghttp2/releases/download/v1.52.0/nghttp2-1.52.0.tar.xz'
curlArchive='https://github.com/curl/curl/releases/download/curl-8_0_1/curl-8.0.1.tar.xz'

echo -n "Install tools... "
sudo apt update && sudo apt install lsb-release wget software-properties-common gnupg git tar xz-utils curl cmake make autoconf libtool -y

echo -n "Install latest Clang... "
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 16
sudo update-alternatives --remove-all clang
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-16 100
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-16 100 

echo -n "Set vars... "
export CC=/usr/bin/clang
export CXX=/usr/bin/clang++ 

#zlib
echo -n "Download and Extract zlib... "
curl -s -L $zlibArchive | tar --xz -x

echo -n "Configure zlib... "
cd zlib*
./configure --static

echo -n "Build zlib... "
make -j$(nproc)

echo -n "Install zlib... "
sudo make install 
cd ..


#c-ares
echo -n "Download and Extract c-ares... "
curl -s -L $caresArchive | tar zx

echo -n "Configure c-ares... "
cd c-ares*
./configure --disable-shared

echo -n "Build c-ares... "
make -j"$(nproc)" 

echo -n "Install c-ares... "
sudo make install 
cd ..


#wolfSSL
echo -n "Download and Extract wolfSSL... "
curl -s -L $wolfSSLArchive | tar xz

echo -n "Configure wolfSSL... "
cd wolfssl*
./autogen.sh 
./configure --enable-curl --enable-distro --enable-static --disable-shared --enable-all-crypto --with-libz

echo -n "Build wolfSSL... "
make

echo -n "Install wolfSSL... "
sudo make install 
cd ..


#nghttp2
echo -n "Download and Extract nghttp2... "
curl -s -L $nghttp2Archive | tar --xz -x

echo -n "Configure nghttp2... "
cd nghttp2*
./configure --enable-lib-only --disable-shared

echo -n "Build nghttp2... "
make -j"$(nproc)" 

echo -n "Install nghttp2... "
sudo make install
cd ..


#cURL
echo -n "Download and Extract curl... "
curl -s -L $curlArchive | tar --xz -x

echo -n "Configure curl... "
cd curl*
./configure --disable-shared --with-wolfssl --enable-ares --with-nghttp2

echo -n "Build cURL... "
make -j"$(nproc)" 

echo -n "Install cURL... "
sudo make install 

echo -e "\e[32mAll done\e[0m"


exit 0
