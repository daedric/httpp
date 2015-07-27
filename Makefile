PREFIX ?= /usr/local

all:
	$(MAKE) -C build all

clean:
	$(MAKE) -C build clean

cmake:
	rm -rf build
	mkdir build
	cd build && cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..

package:
	$(MAKE) -C build package

test:
	$(MAKE) -C build test

re : cmake all
