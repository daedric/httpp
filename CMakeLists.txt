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

# Require at least Windows 7
# See: https://msdn.microsoft.com/en-us/library/windows/desktop/aa383745(v=vs.85).aspx
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DNTDDI_VERSION=0x06010000)
endif()

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

set(CPP_VERSION "c++14" CACHE STRING "C++ version to use")
set_property(CACHE CPP_VERSION PROPERTY STRINGS "c++14" "c++1y")

OPTION(BUILD_TESTS "Build the tests" ON)
OPTION(BUILD_EXAMPLES "Build the examples")
OPTION(BUILD_SHARED_LIBS "Build shared lib instead of static ones")
OPTION(PROFILE_HTTPP "Enable profiling")
OPTION(LINK_OPTIMIZATION "Enable link time optimization")
OPTION(SEARCH_COMMONPP "Should commonpp be looked for" ON)
OPTION(USE_BOOST_PROMISE "Use boost promise instead of the stl one" OFF)

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

IF (${USE_BOOST_PROMISE})
    SET(HTTPP_USE_BOOST_PROMISE 1)
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
  CHECK_CXX_COMPILER_FLAG("-std=${CPP_VERSION}" HAVE_CPP_VERSION)
  CHECK_CXX_COMPILER_FLAG(-Wno-unused-local-typedefs HAVE_NOUNUSED_LOCAL_TYPEDEF)

  IF (HAVE_NOUNUSED_LOCAL_TYPEDEF)
        ADD_DEFINITIONS("-Wno-unused-local-typedefs")
  ENDIF()

  if (NOT HAVE_CPP_VERSION)
      message(FATAL_ERROR "No advanced standard C++ support (-std=${CPP_VERSION} not defined).")
  endif()

  add_definitions("-std=${CPP_VERSION}")
  add_definitions("-Wextra -Wall")
  IF (${CMAKE_BUILD_TYPE} MATCHES "DEBUG")
      ADD_DEFINITIONS("-ggdb -g3")
  ENDIF ()

  IF (${PROFILE_HTTPP})
      MESSAGE(STATUS "HTTPP Profile enabled")
      SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -pg)
      SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -pg)
      SET(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} -pg)
  ENDIF()

  IF (${LINK_OPTIMIZATION})
      IF (NOT ${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
          MESSAGE(FATAL "GCC is required")
      ENDIF()

      FIND_PROGRAM(AR gcc-ar)
      SET(CMAKE_AR ${AR})
      FIND_PROGRAM(RANLIB gcc-ranlib)
      SET(CMAKE_RANLIB ${RANLIB})

      MESSAGE(STATUS "HTTPP link time optimization enabled")
      SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-flto -flto-odr-type-merging")
      SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-flto -flto-odr-type-merging")
      SET(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-flto -flto-odr-type-merging")
  ENDIF()

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

# commonpp
# micro hack to allow commonpp and httpp to be configured separatly within one project
IF (${SEARCH_COMMONPP})
    FIND_PACKAGE(commonpp
        PATHS
            /usr/lib/CMake/
            /usr/local/lib/CMake/
            $ENV{COMMONPP_CMAKE_DIR}
        PATH_SUFFIXES
            commonpp
        QUIET
    )

    IF (NOT ${commonpp_FOUND})
        MESSAGE(STATUS "Use bundled commonpp")
        ADD_SUBDIRECTORY(third_party/commonpp ${CMAKE_CURRENT_BINARY_DIR}/third_party/commonpp)
        INCLUDE_DIRECTORIES(third_party/commonpp/include ${CMAKE_CURRENT_BINARY_DIR}/third_party/commonpp/include)
        SET(commonpp_LIBRARIES commonpp)
        SET(EXPORT_commonpp TRUE)
    ELSE ()
        INCLUDE_DIRECTORIES(${commonpp_INCLUDE_DIRS})
    ENDIF()
ENDIF()

# Actual Stuff ################################################################
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${HTTPP_SOURCE_DIR}/CMakeScripts)

#Thread
FIND_PACKAGE(Threads REQUIRED)

FIND_PACKAGE(OpenSSL REQUIRED)
INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})

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

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIR})

FIND_PACKAGE(CURL REQUIRED)
INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${HWLOC_INCLUDE_DIR})
include_directories(${TBB_INCLUDE_DIRS})
link_directories(${TBB_LIBRARY_DIRS})
link_directories(${TBB_DEBUG_LIBRARY_DIRS})

SET(HTTPP_DEPS
    ${commonpp_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CURL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_LIBRARIES}
    ${HWLOC_PROCESS_LIBS}
    ${TBB_LIBRARIES})

IF (UNIX AND NOT APPLE)
    SET (HTTPP_DEPS ${HTTPP_DEPS} rt)
ENDIF()

INCLUDE_DIRECTORIES(include/)
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/include/")
ADD_SUBDIRECTORY(include/)
ADD_SUBDIRECTORY(src/)

IF (${BUILD_EXAMPLES})
    ADD_SUBDIRECTORY(examples/)
ENDIF()

IF (${BUILD_TESTS})
    ENABLE_TESTING()
    ADD_SUBDIRECTORY(tests/)
ENDIF()


# CMake export targets

IF (${EXPORT_commonpp})
    EXPORT(TARGETS httpp ${commonpp_LIBRARIES}
           FILE "${PROJECT_BINARY_DIR}/httpp_target.cmake")
ELSE()
    EXPORT(TARGETS httpp
           FILE "${PROJECT_BINARY_DIR}/httpp_target.cmake")
ENDIF()

EXPORT(PACKAGE HTTPP)

if (${CMAKE_MAJOR_VERSION} EQUAL 2
        AND ${CMAKE_MINOR_VERSION} EQUAL 8
        AND ${CMAKE_PATCH_VERSION} LESS 8)

    CONFIGURE_FILE(
        "contrib/HTTPPConfigVersion.cmake.in"
        "${PROJECT_BINARY_DIR}/HTTPPConfigVersion.cmake"
        @ONLY
       )

ELSE()
    INCLUDE(CMakePackageConfigHelpers)
    WRITE_BASIC_PACKAGE_VERSION_FILE("${PROJECT_BINARY_DIR}/HTTPPConfigVersion.cmake"
                                     VERSION ${HTTPP_VERSION}
                                     COMPATIBILITY ExactVersion)
ENDIF()

CONFIGURE_FILE("HTTPPConfig.cmake.in"
               "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/HTTPPConfig.cmake" @ONLY)

INSTALL(FILES
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/HTTPPConfig.cmake"
        "${PROJECT_BINARY_DIR}/HTTPPConfigVersion.cmake"
        DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

INSTALL(EXPORT httpp_target
        DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# CPack stuff ##################################################################

set(CPACK_COMPONENTS_ALL binaries libraries includes configuration tools)
SET(CPACK_PACKAGE_VENDOR "Thomas Sanchez")
SET(CPACK_PACKAGE_CONTACT "Thomas Sanchez <thomas.sanchz@gmail.com>")
SET(CPACK_GENERATOR "TGZ")

INCLUDE(CPack)
