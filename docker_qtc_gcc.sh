#!/bin/bash
set -e
IMAGE=arbmind/qtcreator-gcc:latest
DOCKER_DISPLAY=host.docker.internal:0

docker run --rm -it --mount src=`pwd`,target=/build,type=bind \
    -e DISPLAY=${DOCKER_DISPLAY} \
    ${IMAGE} \
    qtcreator rec_cpp.qbs
