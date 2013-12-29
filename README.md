httpp
=====

Micro http server written in C++

The motivation behind this little piece of code is to provide a really simple, yet efficient HTTP server.

REQUIREMENTS
============

Only boost is required, version >= 1.54.
It may work with a version < 1.54 but it has not been tested.

BUILD
=====

    $> mkdir build
    $> cd build
    $> cmake $HTTPP_PATH
    $> make

A debian package can be generated running:

    $> make package

To build tests:

    $> cmake $HTTPP_PATH -DBUILD_TESTS=ON

By default the build is in release mode, to change that you can:

    $> cmake $HTTPP_PATH -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=DEBUG


The compilation has been tested on Mac OS X and GNU/Linux distros.
