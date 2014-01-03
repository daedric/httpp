# Part of HTTPP.
#
# Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
#


SET(SOURCES
    HttpServer.cpp

    http/Connection.cpp
    http/Parser.cpp
    http/Request.cpp
    http/HttpCode.cpp
    http/Response.cpp
    http/Utils.cpp

    utils/ThreadPool.cpp
)


ADD_LIBRARY(httpp ${SOURCES})

IF (${BUILD_SHARED_LIBS})
    TARGET_LINK_LIBRARIES(httpp ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
    INSTALL(TARGETS httpp
        LIBRARY DESTINATION
        lib COMPONENT library)
ELSE()
    INSTALL(TARGETS httpp
        ARCHIVE DESTINATION
        lib COMPONENT library)
ENDIF()


