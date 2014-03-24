#!/bin/sh

wget http://jaist.dl.sourceforge.net/boost/boost_1_54_0.tar.bz2
tar -xf boost_1_54_0.tar.bz2
cd boost_1_54_0
echo "using gcc : 4.8 : /usr/bin/g++-4.8 ; " >> tools/build/v2/user-config.jam
./bootstrap.sh --prefix=/usr
sudo ./b2 -j2 release architecture=x86 cxxflags="-std=c++11 -Wno-deprecated-declarations -Wno-unused-local-typedefs" address-model=64 --with-system --with-thread --with-test --with-log install > /dev/null
