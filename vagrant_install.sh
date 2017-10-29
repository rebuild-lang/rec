#!/bin/bash

# host:
#
# vagrant up
# vagrant ssh

# prepare:
sudo -i
apt update
apt upgrade -y

## http://apt.llvm.org/
## https://stackoverflow.com/questions/39332406/install-libc-on-ubuntu

wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -

apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-5.0 main"
add-apt-repository ppa:beineri/opt-qt591-xenial -y

apt update
apt install clang-5.0 lldb-5.0 lld-5.0 libclang-5.0-dev qt59script subversion cmake -y

update-alternatives --install /usr/bin/cc cc /usr/bin/clang-5.0 100 \
&& update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-5.0 100 \
&& update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-5.0 100 \
&& update-alternatives --install /usr/bin/clang clang /usr/bin/clang-5.0 100 \
&& update-alternatives --install /usr/bin/ld ld /usr/bin/ld.lld-5.0 10 \
&& update-alternatives --install /usr/bin/ld ld /usr/bin/ld.gold 20 \
&& update-alternatives --install /usr/bin/ld ld /usr/bin/ld.bfd 30 \
&& ld --version && echo 3 | update-alternatives --config ld && ld --version

cd /tmp
svn co http://llvm.org/svn/llvm-project/libcxx/branches/release_50/ libcxx
svn co http://llvm.org/svn/llvm-project/libcxxabi/branches/release_50/ libcxxabi
mkdir -p libcxx/build libcxxabi/build

cd /tmp/libcxx/build
cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_CONFIG_PATH=/usr/bin/llvm-config-5.0 -DCMAKE_INSTALL_PREFIX=/usr .. && make install

cd /tmp/libcxxabi/build
CPP_INCLUDE_PATHS=echo | c++ -Wp,-v -x c++ - -fsyntax-only 2>&1 \
  |grep ' /usr'|tr '\n' ' '|tr -s ' ' |tr ' ' ';'
CPP_INCLUDE_PATHS="/usr/include/c++/v1/;$CPP_INCLUDE_PATHS"
cmake -G "Unix Makefiles" -DLIBCXX_CXX_ABI=libstdc++ \
      -DLIBCXX_LIBSUPCXX_INCLUDE_PATHS="$CPP_INCLUDE_PATHS" \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
      -DLLVM_CONFIG_PATH=/usr/bin/llvm-config-5.0 \
      -DLIBCXXABI_LIBCXX_INCLUDES=../../libcxx/include  ..
make install

cd /tmp/libcxx/build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
      -DLIBCXX_CXX_ABI=libcxxabi -DLLVM_CONFIG_PATH=/usr/bin/llvm-config-5.0\
      -DLIBCXX_CXX_ABI_INCLUDE_PATHS=../../libcxxabi/include .. \
&& make install

##
## build custom qbs
##
exit

source /opt/qt59/bin/qt59-env.sh

## build qbs
cd /tmp
git clone https://github.com/qbs/qbs.git

mkdir -p /tmp/qbs-build
cd /tmp/qbs-build
qmake -makefile -spec linux-clang-libc++ "LIBS+=-lc++abi" "QBS_INSTALL_PREFIX=/opt/qbs" /tmp/qbs
make -j5 install

export PATH=/tmp/qbs-build/bin:$PATH
qbs setup-toolchains --detect
qbs config defaultProfile clang

##
## use it
##
cd /vagrant
qbs build --build-directory /tmp/rec_cpp-build
