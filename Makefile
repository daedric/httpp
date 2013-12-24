PREFIX ?= /usr/local

all:
	make -C build all

clean:
	make -C build clean

cmake:
	rm -rf build
	mkdir build
	cd build && cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..

package:
	make -C build package

test:
	make -C build test

re : cmake all
