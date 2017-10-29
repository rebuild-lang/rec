# simple invocation helper

build_path = /tmp/rec_cpp-build

build :
	qbs build --build-directory $(build_path)

check :
	qbs build --build-directory $(build_path) -p autotest-runner

install :
	qbs install --build-directory $(build_path)

all : build
