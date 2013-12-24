# Part of HTTPP.
#
# Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
#


SET(VERSION_TO_GENERATE  "${CMAKE_CURRENT_SOURCE_DIR}/httpp/version.hpp.in")
SET(VERSION_GENERATED    "${CMAKE_CURRENT_BINARY_DIR}/httpp/version.hpp" CACHE STRINGS "Generated version.h")

CONFIGURE_FILE(${VERSION_TO_GENERATE} ${VERSION_GENERATED})
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/httpp")
INSTALL(FILES ${VERSION_GENERATED}
        DESTINATION include/httpp
        COMPONENT includes)

INSTALL(DIRECTORY httpp
        DESTINATION include/
        COMPONENT includes
        FILES_MATCHING
        REGEX "hpp$"
       )
