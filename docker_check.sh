#!/bin/bash

docker run --rm -t -v`pwd`:/build -e TERM=xterm-color -e GTEST_COLOR=yes \
    arbmind/qbs-clang8:1 \
    build --file /build/rec_cpp.qbs --build-directory /tmp/rec_cpp-build -p autotest-runner \
    modules.cpp.commonCompilerFlags:-fcolor-diagnostics
