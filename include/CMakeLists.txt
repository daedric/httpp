# Part of HTTPP.
#
# Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
#


SET(VERSION_TO_GENERATE   "${CMAKE_CURRENT_SOURCE_DIR}/httpp/version.hpp.in")
SET(VERSION_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/httpp/" CACHE STRINGS "Generated version.h directory")
SET(VERSION_GENERATED     "${VERSION_GENERATED_DIR}/version.hpp" CACHE STRINGS "Generated version.h")

CONFIGURE_FILE(${VERSION_TO_GENERATE} ${VERSION_GENERATED})
INCLUDE_DIRECTORIES("${VERSION_GENERATED_DIR}")

SET(CONFIG_TO_GENERATE   "${CMAKE_CURRENT_SOURCE_DIR}/httpp/detail/generated/config.hpp.in")
SET(CONFIG_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/httpp/detail/generated/" CACHE STRINGS "Generated config.h directory")
SET(CONFIG_GENERATED     "${CONFIG_GENERATED_DIR}/config.hpp" CACHE STRINGS "Generated config.h")

CONFIGURE_FILE(${CONFIG_TO_GENERATE} ${CONFIG_GENERATED})

INSTALL(DIRECTORY httpp
        DESTINATION ${INSTALL_INCLUDE_DIR} COMPONENT dev
        FILES_MATCHING REGEX "hpp$"
       )

