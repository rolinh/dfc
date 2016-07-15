all: release

release: _mkdir_build
	cd build && cmake -DCMAKE_BUILD_TYPE=RELEASE .. && make

debug: _mkdir_build
	cd build && cmake -DCMAKE_BUILD_TYPE=DEBUG -DCMAKE_VERBOSE_MAKEFILE=YES .. && make

clean:
	rm -rf build

_mkdir_build:
	test -d build || mkdir build

.PHONY: all release debug _mkdir_build clean
