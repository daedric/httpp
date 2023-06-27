[![Build Status](http://travis-ci.org/daedric/httpp.png)](http://travis-ci.org/daedric/httpp)
[![Coverity Status](https://scan.coverity.com/projects/6818/badge.svg)](https://scan.coverity.com/projects/daedric-httpp)

**While the server works well, the client based on curl needs some migration to
the latest version.**

httpp
=====

Micro http server and client written in C++

The motivation behind this little piece of code is to provide a really simple,
yet efficient HTTP server to implement in-app webservice easily.

The HttpServer will take care of the connections, request parsing and response
generation. The handler on another hand is reponsible for implementing the
"logic" behind the service. For example, the server has a very limited
knowledge of the HTTP protocol…

The HttpClient has been implemented using curl and its multi interface.

See an exampe [here](examples/echo/simple_echo_server.cpp) or
[here](examples/ping/ping.cpp).

REQUIREMENTS
============

Boost is required, version >= 1.54 and libcurl. HTTPP has a dependency on
[CommonPP](https://github.com/daedric/commonpp) since the version 0.7. CommonPP
has a dependency on Boost and TBB. The TBB dependency will become optional.

A C++14 compliant compiler is also required.

BUILD
=====

    $> mkdir -p $HTTPP_PATH && cd $HTTPP_PATH
    $> git clone https://github.com/daedric/httpp.git .
    $> git submodule update --init
    $> mkdir -p $HTTPP_BUILD_PATH && cd $HTTPP_BUILD_PATH
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


The compilation and tests have been run on Mac OS X (curl and openssl from homebrew seems to be required to have all tests passing, I don't know why yet), and GNU/Linux distro (Ubuntu, ArchLinux).

Current stable
===========

The current stable is the v0.7.0.
There are still some things missing:

* Optional commonpp dependencies;
* Probes in the server.


Design choices
==============

This section is only about the server.

Since the 0.7 version, HTTPP is trying to allocate as few memory as possible.
Each connection is associated with some buffers that are never released to the
system.  This this might create a memory overhead, this allows to achieve a
better performance.

The HTTP parser has been ported to Ragel to generate a faster parser than the
one based on stream. I found out after having finished that
[Mongel](https://github.com/mongrel/mongrel/) did the same way before me, so I
might try to import it if it makes sense.

At the same time, the parser avoids as much as possible copies by using `std::string_view`.

One of the reason is that HTTPP is designed to be used in a REST server
exposing an API and developed for
[RTB](https://en.wikipedia.org/wiki/Real-time_bidding) platform, each request
is relatively similar to the previous one in size.  Second reason, it makes
everything simpler (no buffer management, no release, no cache needed, etc).

Performances
============

This section is only about the server.

On a Dell 3800 with an `Intel(R) Core(TM) i7-4702HQ CPU @ 2.20GHz`, I was able
to achieve between 160K and 180K request per second with the `ping` program
(against 70 to 90K with the 0.6.2) and
[ab](https://httpd.apache.org/docs/current/programs/ab.html) with `-k -c 8`
options on the same machine, this means that HTTPP overhead is about 5 to 6
micro seconds.

With a slightly more complex server that decoded the request, performed some string manipulation 
and returned a predefined json payload, we have seen approximately 220K requests per second on 
identical hardware, using [wrk](https://github.com/wg/wrk) to drive the load testing. 5 threads were allocated
to the httpp server, and 3 to wrk (with 500 concurrent connections).

This is scaling as threads are added but I don't have enough proper machine to
do a real benchmark.

