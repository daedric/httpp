[![Build Status](http://travis-ci.org/daedric/httpp.png)](http://travis-ci.org/daedric/httpp)

httpp
=====

Micro http server written in C++

The motivation behind this little piece of code is to provide a really simple, yet efficient HTTP server to implement in-app webservice easily.

The HttpServer will take care of the connections, request parsing and response generation. The handler on another hand is reponsible for implementing the "logic" behind the service. For example, the server has a very limited knowledge of the HTTP protocol…

See an exampe [here](examples/echo/simple_echo_server.cpp)

REQUIREMENTS
============

Only boost is required, version >= 1.54.
A C++11 compliant compiler is also required.

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

Examples:

    $> cmake $HTTPP_PATH -DBUILD_EXAMPLES=ON

To build the shared library version:

    $> cmake $HTTPP_PATH -DBUILD_SHARED_LIBS=ON

By default the build is in release mode, to change that you can:

    $> cmake $HTTPP_PATH -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=DEBUG


The compilation and tests have been run on Windows (7, 8.1) , Mac OS X (Maverick) and GNU/Linux distro (Ubuntu, ArchLinux).
