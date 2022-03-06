#!/bin/bash
set -e
IMAGE=arbmind/qbs-clang13:qbs_v1.20.1

docker run --rm -t -v`pwd`:/build -e TERM=xterm-color -e GTEST_COLOR=yes \
    ${IMAGE} \
    build --file /build/rec_cpp.qbs --build-directory /tmp/rec_cpp-build -p autotest-runner \
    modules.cpp.commonCompilerFlags:-fcolor-diagnostics
