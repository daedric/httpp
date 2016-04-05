# Part of HTTPP.
#
# Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
#


SET(SOURCES
    HttpServer.cpp
    HttpClient.cpp

    http/helper/ReadWholeRequest.cpp
    http/Connection.cpp
    http/Parser.cpp
    http/parser_ragel.cpp
    http/Protocol.cpp
    http/Request.cpp
    http/Response.cpp
    http/Utils.cpp

    http/client/curl_parser.cpp
    http/client/Connection.cpp
    http/client/Manager.cpp
    http/client/Request.cpp

    utils/LazyDecodedValue.cpp

    ${VERSION_TO_GENERATE}
    ${CONFIG_TO_GENERATE}
)

ADD_LIBRARY(httpp ${SOURCES})

SET_TARGET_PROPERTIES(httpp PROPERTIES
                      PUBLIC_HEADER "${VERSION_GENERATED}")

TARGET_LINK_LIBRARIES(httpp ${HTTPP_DEPS})

INSTALL(TARGETS httpp
        EXPORT httpp_target
        PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/httpp" COMPONENT dev
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR} COMPONENT library
        LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT library)

INSTALL(FILES ${CONFIG_GENERATED}
        DESTINATION "${INSTALL_INCLUDE_DIR}/httpp/detail/generated/" COMPONENT dev)

