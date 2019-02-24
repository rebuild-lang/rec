#!/bin/bash

## host:

## vagrant up
## vagrant ssh

## launch:

## sudo /vagrant/vagrant_install.sh
apt update
apt upgrade -y

## https://apt.llvm.org/
## https://stackoverflow.com/questions/39332406/install-libc-on-ubuntu

wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -

apt-add-repository "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-7 main"
add-apt-repository ppa:beineri/opt-qt-5.11.2-bionic -y

apt update
apt install clang-7 lldb-7 lld-7 libclang-7-dev qt511script subversion cmake -y

update-alternatives --install /usr/bin/cc cc /usr/bin/clang-7 100 \
&& update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-7 100 \
&& update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-7 100 \
&& update-alternatives --install /usr/bin/clang clang /usr/bin/clang-7 100 \
&& update-alternatives --install /usr/bin/ld ld /usr/bin/ld.lld-7 10 \
&& update-alternatives --install /usr/bin/ld ld /usr/bin/ld.gold 20 \
&& update-alternatives --install /usr/bin/ld ld /usr/bin/ld.bfd 30 \
&& ld --version && echo 3 | update-alternatives --config ld && ld --version

cd /tmp
svn co -q https://llvm.org/svn/llvm-project/libcxx/branches/release_70/ libcxx
svn co -q https://llvm.org/svn/llvm-project/libcxxabi/branches/release_70/ libcxxabi
mkdir -p libcxx/build libcxxabi/build

cd /tmp/libcxx/build
cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_CONFIG_PATH=/usr/bin/llvm-config-7 -DCMAKE_INSTALL_PREFIX=/usr .. && make install

cd /tmp/libcxxabi/build
CPP_INCLUDE_PATHS=echo | c++ -Wp,-v -x c++ - -fsyntax-only 2>&1 \
  |grep ' /usr'|tr '\n' ' '|tr -s ' ' |tr ' ' ';'
CPP_INCLUDE_PATHS="/usr/include/c++/v1/;$CPP_INCLUDE_PATHS"
cmake -G "Unix Makefiles" -DLIBCXX_CXX_ABI=libstdc++ \
      -DLIBCXX_LIBSUPCXX_INCLUDE_PATHS="$CPP_INCLUDE_PATHS" \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
      -DLLVM_CONFIG_PATH=/usr/bin/llvm-config-7 \
      -DLIBCXXABI_LIBCXX_INCLUDES=../../libcxx/include  ..
make install

cd /tmp/libcxx/build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
      -DLIBCXX_CXX_ABI=libcxxabi -DLLVM_CONFIG_PATH=/usr/bin/llvm-config-7\
      -DLIBCXX_CXX_ABI_INCLUDE_PATHS=../../libcxxabi/include .. \
&& make install

##
## build custom qbs
##
source /opt/qt511/bin/qt511-env.sh

## build qbs
cd /tmp
git clone https://github.com/qbs/qbs.git

mkdir -p /tmp/qbs-build
cd /tmp/qbs-build
qmake -makefile -spec linux-clang-libc++ "LIBS+=-lc++abi" "QBS_INSTALL_PREFIX=/opt/qbs" /tmp/qbs
make -j5 install

exit
## run without root from here on!

export PATH=/opt/qbs/bin:$PATH
qbs setup-toolchains --detect
qbs config defaultProfile clang

##
## use it
##
cd /vagrant
qbs build --build-directory /tmp/rec_cpp-build
