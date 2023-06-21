# Part of HTTPP.
#
# Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
#

project(HTTPP C CXX)
cmake_minimum_required(VERSION 3.9)
cmake_policy(SET CMP0069 NEW)

# CMAKE_POLICY(SET CMP0005 NEW)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting cmake build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
                                               "RelWithDebInfo")
endif()

set(INSTALL_LIB_DIR
    lib
    CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR
    bin
    CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR
    include
    CACHE PATH "Installation directory for header files")

if(WIN32 AND NOT CYGWIN)
  set(_CMAKE_DIR "CMake")
else()
  set(_CMAKE_DIR "lib/CMake/HTTPP")
endif()

set(INSTALL_CMAKE_DIR
    "${_CMAKE_DIR}"
    CACHE PATH "Installation directory for CMake files")

set(CMAKE_CXX_STANDARD
    17
    CACHE STRING "C++ standard for all targets.")

option(BUILD_CLIENT "Build client" false)
option(BUILD_TESTS "Build the tests" ON)
option(BUILD_EXAMPLES "Build the examples")
option(BUILD_SHARED_LIBS "Build shared lib instead of static ones")

# Setting vars #################################################################
set(HTTPP_VERSION_MAJOR "0")
set(HTTPP_VERSION_MINOR "8")
set(HTTPP_VERSION_PATCH "0")

set(CPACK_PACKAGE_VERSION_MAJOR ${HTTPP_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${HTTPP_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${HTTPP_VERSION_PATCH})

set(HTTPP_VERSION
    "${HTTPP_VERSION_MAJOR}.${HTTPP_VERSION_MINOR}.${HTTPP_VERSION_PATCH}")

set(CPACK_PACKAGE_VERSION ${HTTPP_VERSION})

if(${HTTPP_PARSER_BACKEND} MATCHES "Stream")
  set(HTTPP_PARSER_BACKEND "HTTPP_STREAM_BACKEND")
  message(STATUS "HTTPP PARSER Backend : Stream")
elseif(${HTTPP_PARSER_BACKEND} MATCHES "Ragel")
  set(HTTPP_PARSER_BACKEND "HTTPP_RAGEL_BACKEND")
  message(STATUS "HTTPP PARSER Backend : Ragel")
else()
  set(HTTPP_PARSER_BACKEND "HTTPP_RAGEL_BACKEND")
  message(STATUS "HTTPP PARSER Backend : Ragel (Defaulted)")
endif()

message(
  STATUS
    "Cmake Version        : ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}"
)
message(STATUS "HTTPP Version        : ${HTTPP_VERSION}")
message(STATUS "Build Type           : ${CMAKE_BUILD_TYPE}")
message(STATUS "Build Tests          : ${BUILD_TESTS}")
message(
  STATUS "System               : ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
message(STATUS "Install Prefix       : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Source Directory     : ${HTTPP_SOURCE_DIR}")

if(${CMAKE_CXX_COMPILER_ID} MATCHES GNU OR ${CMAKE_CXX_COMPILER_ID} MATCHES
                                           Clang)
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-Wno-unused-local-typedefs
                          HAVE_NOUNUSED_LOCAL_TYPEDEF)

  if(HAVE_NOUNUSED_LOCAL_TYPEDEF)
    add_definitions("-Wno-unused-local-typedefs")
  endif()

  add_definitions("-Wextra -Wall")
  if(${CMAKE_BUILD_TYPE} MATCHES "DEBUG")
    add_definitions("-ggdb -g3")
  endif()
endif()

if(${BUILD_SHARED_LIBS})
  add_definitions("-DBOOST_LOG_DYN_LINK=1 -DBOOST_ALL_DYN_LINK=1")
endif()

if(WIN32 AND CMAKE_SYSTEM_VERSION)
  set(VER ${CMAKE_SYSTEM_VERSION})
  string(REPLACE "." "" VER ${VER})
  string(REGEX REPLACE "([0-9])" "0\\1" VER ${VER})

  set(VERSION "0x${VER}")
  add_definitions(-D_WIN32_WINNT=${VERSION})
endif()

if(TARGET commonpp)
  set(commonpp_LIBRARIES commonpp)
else()
  find_package(
    commonpp
    PATHS
    /usr/lib/CMake/
    /usr/local/lib/CMake/
    $ENV{COMMONPP_CMAKE_DIR}
    PATH_SUFFIXES
    commonpp)

  if(NOT ${commonpp_FOUND})
    message(STATUS "Use bundled commonpp")
    add_subdirectory(third_party/commonpp
                     ${CMAKE_CURRENT_BINARY_DIR}/third_party/commonpp)
    set(commonpp_LIBRARIES commonpp)
  endif()
endif()
# Actual Stuff ################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${HTTPP_SOURCE_DIR}/CMakeScripts)

# Thread
find_package(Threads REQUIRED)

find_package(OpenSSL REQUIRED)
include_directories(SYSTEM ${OPENSSL_INCLUDE_DIR})

# Boost
if(${BUILD_SHARED_LIBS})
  set(Boost_USE_STATIC_LIBS OFF)
else()
  set(Boost_USE_STATIC_LIBS ON)
endif()

set(Boost_USE_MULTITHREADED ON)

message(STATUS "Boost static library use     : ${Boost_USE_STATIC_LIBS}")

find_package(
  Boost 1.54.0
  COMPONENTS log log_setup filesystem unit_test_framework thread system
  REQUIRED)

include_directories(SYSTEM ${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

set(HTTPP_DEPS ${commonpp_LIBRARIES} ${Boost_LIBRARIES}
               ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES})

if(${BUILD_CLIENT})
  find_package(CURL REQUIRED)
  include_directories(SYSTEM ${CURL_INCLUDE_DIRS})
  set(HTTPP_DEPS ${CURL_LIBRARIES} ${HTTPP_DEPS})
endif()

if(UNIX AND NOT APPLE)
  set(HTTPP_DEPS ${HTTPP_DEPS} rt)
endif()

add_subdirectory(src/)
add_subdirectory(include/)

if(${BUILD_EXAMPLES})
  add_subdirectory(examples/)
endif()

if(${BUILD_TESTS})
  enable_testing()
  add_subdirectory(tests/)
endif()

find_program(
  CLANG_FORMAT
  NAMES clang-format
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
        clang-format-3.8)

if(CLANG_FORMAT)
  file(
    GLOB_RECURSE
    ALL_SOURCE_FILES
    *.cpp
    *.h
    *.hpp
    *.c
    *.cc)
  add_custom_target(format-srcs COMMAND ${CLANG_FORMAT} -style=file -i
                                        ${ALL_SOURCE_FILES})
endif()
