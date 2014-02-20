# Part of HTTPP.
#
# Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
#


SET(SOURCES
    HttpServer.cpp
    HttpClient.cpp

    http/client/curl_parser.cpp
    http/Connection.cpp
    http/Parser.cpp
    http/Protocol.cpp
    http/Request.cpp
    http/Response.cpp
    http/URL.cpp
    http/Utils.cpp

    http/client/Request.cpp
    http/client/Response.cpp

    utils/ThreadPool.cpp
)


ADD_LIBRARY(httpp ${SOURCES})

IF (${BUILD_SHARED_LIBS})

    SET(DEFAULT_LIBRARIES
        ${Boost_LIBRARIES}
        ${CURL_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        rt)

    IF (TCMALLOC_FOUND)
        SET(DEFAULT_LIBRARIES
            ${DEFAULT_LIBRARIES}
            ${TCMALLOC_LIBRARIES})
    ENDIF()

    TARGET_LINK_LIBRARIES(httpp ${DEFAULT_LIBRARIES})
    INSTALL(TARGETS httpp
        LIBRARY DESTINATION
        lib COMPONENT library)
ELSE()
    INSTALL(TARGETS httpp
        ARCHIVE DESTINATION
        lib COMPONENT library)
ENDIF()


