# Part of HTTPP.
#
# Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
#

PROJECT(HTTPP C CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#CMAKE_POLICY(SET CMP0005 NEW)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    MESSAGE(STATUS "Setting cmake build type to 'Release' as none was specified.")
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()


set(INSTALL_LIB_DIR     lib     CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR     bin     CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")

IF(WIN32 AND NOT CYGWIN)
    SET(_CMAKE_DIR "CMake")
ELSE()
    SET(_CMAKE_DIR "lib/CMake/HTTPP")
ENDIF()

set(INSTALL_CMAKE_DIR "${_CMAKE_DIR}" CACHE PATH "Installation directory for CMake files")

set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard for all targets.")

OPTION(BUILD_TESTS "Build the tests" ON)
OPTION(BUILD_EXAMPLES "Build the examples")
OPTION(BUILD_SHARED_LIBS "Build shared lib instead of static ones")

# Setting vars #################################################################
SET(HTTPP_VERSION_MAJOR "0")
SET(HTTPP_VERSION_MINOR "8")
SET(HTTPP_VERSION_PATCH "0")

SET(CPACK_PACKAGE_VERSION_MAJOR ${HTTPP_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${HTTPP_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${HTTPP_VERSION_PATCH})

SET(HTTPP_VERSION "${HTTPP_VERSION_MAJOR}.${HTTPP_VERSION_MINOR}.${HTTPP_VERSION_PATCH}")

SET(CPACK_PACKAGE_VERSION ${HTTPP_VERSION})

IF (${HTTPP_PARSER_BACKEND} MATCHES "Stream")
    SET(HTTPP_PARSER_BACKEND "HTTPP_STREAM_BACKEND")
    MESSAGE(STATUS "HTTPP PARSER Backend : Stream")
ELSEIF (${HTTPP_PARSER_BACKEND} MATCHES "Ragel")
    SET(HTTPP_PARSER_BACKEND "HTTPP_RAGEL_BACKEND")
    MESSAGE(STATUS "HTTPP PARSER Backend : Ragel")
ELSE()
    SET(HTTPP_PARSER_BACKEND "HTTPP_RAGEL_BACKEND")
    MESSAGE(STATUS "HTTPP PARSER Backend : Ragel (Defaulted)")
ENDIF()

MESSAGE(STATUS "Cmake Version        : ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
MESSAGE(STATUS "HTTPP Version        : ${HTTPP_VERSION}")
MESSAGE(STATUS "Build Type           : ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "Build Tests          : ${BUILD_TESTS}")
MESSAGE(STATUS "System               : ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
MESSAGE(STATUS "Install Prefix       : ${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "Source Directory     : ${HTTPP_SOURCE_DIR}")


IF (${CMAKE_CXX_COMPILER_ID} MATCHES GNU OR ${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  INCLUDE(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG(-Wno-unused-local-typedefs HAVE_NOUNUSED_LOCAL_TYPEDEF)

  IF (HAVE_NOUNUSED_LOCAL_TYPEDEF)
        ADD_DEFINITIONS("-Wno-unused-local-typedefs")
  ENDIF()

  add_definitions("-Wextra -Wall")
  IF (${CMAKE_BUILD_TYPE} MATCHES "DEBUG")
      ADD_DEFINITIONS("-ggdb -g3")
  ENDIF ()
ENDIF()

IF (${BUILD_SHARED_LIBS})
    ADD_DEFINITIONS("-DBOOST_LOG_DYN_LINK=1 -DBOOST_ALL_DYN_LINK=1")
ENDIF()

IF (WIN32 AND CMAKE_SYSTEM_VERSION)
    SET(VER ${CMAKE_SYSTEM_VERSION})
    STRING(REPLACE "." "" VER ${VER})
    STRING(REGEX REPLACE "([0-9])" "0\\1" VER ${VER})

    SET(VERSION "0x${VER}")
    ADD_DEFINITIONS(-D_WIN32_WINNT=${VERSION})
endif()

if (TARGET commonpp)
    SET(commonpp_LIBRARIES commonpp)
else()
    FIND_PACKAGE(commonpp
        PATHS
            /usr/lib/CMake/
            /usr/local/lib/CMake/
            $ENV{COMMONPP_CMAKE_DIR}
        PATH_SUFFIXES
            commonpp
    )

    IF (NOT ${commonpp_FOUND})
        MESSAGE(STATUS "Use bundled commonpp")
        ADD_SUBDIRECTORY(third_party/commonpp ${CMAKE_CURRENT_BINARY_DIR}/third_party/commonpp)
        SET(commonpp_LIBRARIES commonpp)
    ENDIF()
endif()
# Actual Stuff ################################################################
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${HTTPP_SOURCE_DIR}/CMakeScripts)

#Thread
FIND_PACKAGE(Threads REQUIRED)

FIND_PACKAGE(OpenSSL REQUIRED)
INCLUDE_DIRECTORIES(SYSTEM ${OPENSSL_INCLUDE_DIR})

#Boost
IF (${BUILD_SHARED_LIBS})
    SET(Boost_USE_STATIC_LIBS    OFF)
ELSE()
    SET(Boost_USE_STATIC_LIBS    ON)
ENDIF()

SET(Boost_USE_MULTITHREADED  ON)

MESSAGE(STATUS "Boost static library use     : ${Boost_USE_STATIC_LIBS}")

FIND_PACKAGE(Boost 1.54.0
    COMPONENTS
        log
        log_setup
        filesystem
        unit_test_framework
        thread
        system
    REQUIRED
)

INCLUDE_DIRECTORIES(SYSTEM ${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIR})

FIND_PACKAGE(CURL REQUIRED)
INCLUDE_DIRECTORIES(SYSTEM ${CURL_INCLUDE_DIRS})

SET(HTTPP_DEPS
    ${commonpp_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CURL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_LIBRARIES})

IF (UNIX AND NOT APPLE)
    SET (HTTPP_DEPS ${HTTPP_DEPS} rt)
ENDIF()

add_subdirectory(src/)
add_subdirectory(include/)

IF (${BUILD_EXAMPLES})
    ADD_SUBDIRECTORY(examples/)
ENDIF()

IF (${BUILD_TESTS})
    ENABLE_TESTING()
    ADD_SUBDIRECTORY(tests/)
ENDIF()

find_program(CLANG_FORMAT
        NAMES
        clang-format
        clang-format-9.0
        clang-format-8.1
        clang-format-8.0
        clang-format-7.0
        clang-format-6.2
        clang-format-6.0
        clang-format-5.2
        clang-format-5.0
        clang-format-4.0
        clang-format-3.9
        clang-format-3.8
        )


if (CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.h *.hpp *.c *.cc)
    add_custom_target(
            format-srcs
            COMMAND
            ${CLANG_FORMAT}
            -style=file
            -i
            ${ALL_SOURCE_FILES})
endif()

