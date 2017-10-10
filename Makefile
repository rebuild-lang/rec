# simple invocation helper

build_path = /tmp/rec_cpp-build

build :
	qbs build --build-directory $(build_path) debug

check :
	qbs build --build-directory $(build_path) -p autotest-runner debug

install :
	qbs install --build-directory $(build_path) debug

all : build
