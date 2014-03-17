# Part of HTTPP.
#
# Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
#


SET(VERSION_TO_GENERATE   "${CMAKE_CURRENT_SOURCE_DIR}/httpp/version.hpp.in")
SET(VERSION_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/httpp/" CACHE STRINGS "Generated version.h directory")
SET(VERSION_GENERATED     "${VERSION_GENERATED_DIR}/version.hpp" CACHE STRINGS "Generated version.h")

CONFIGURE_FILE(${VERSION_TO_GENERATE} ${VERSION_GENERATED})
INCLUDE_DIRECTORIES("${VERSION_GENERATED_DIR}")

INSTALL(DIRECTORY httpp
        DESTINATION ${INSTALL_INCLUDE_DIR} COMPONENT dev
        FILES_MATCHING REGEX "hpp$"
       )

