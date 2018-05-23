# Part of HTTPP.
#
# Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
#


set(sources
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
    http/RestDispatcher.cpp

    http/client/curl_parser.cpp
    http/client/Connection.cpp
    http/client/Manager.cpp
    http/client/Request.cpp

    utils/LazyDecodedValue.cpp

    ${VERSION_TO_GENERATE}
    ${CONFIG_TO_GENERATE}
)

set_source_files_properties(http/parser_ragel.cpp PROPERTIES COMPILE_FLAGS -Wno-implicit-fallthrough)

add_library(httpp ${sources})

target_link_libraries(httpp ${HTTPP_DEPS})

