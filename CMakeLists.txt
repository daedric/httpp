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
  SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
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

FOREACH(p LIB BIN INCLUDE CMAKE)
    SET(var INSTALL_${p}_DIR)
    IF(NOT IS_ABSOLUTE "${${var}}")
        SET(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
    ENDIF()
ENDFOREACH()


OPTION(BUILD_TESTS "Should the tests be built")
OPTION(BUILD_EXAMPLES "Should the examples be built")
OPTION(BUILD_SHARED_LIBS "Should the shared lib be built")
OPTION(PROFILE_HTTPP "Should we instrument the code to enable profiling")

# Setting vars #################################################################
SET(HTTPP_VERSION_MAJOR "0")
SET(HTTPP_VERSION_MINOR "7")
SET(HTTPP_VERSION_PATCH "0")

SET(CPACK_PACKAGE_VERSION_MAJOR ${HTTPP_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${HTTPP_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${HTTPP_VERSION_PATCH})

SET(HTTPP_VERSION "${HTTPP_VERSION_MAJOR}.${HTTPP_VERSION_MINOR}.${HTTPP_VERSION_PATCH}")

SET(CPACK_PACKAGE_VERSION ${HTTPP_VERSION})

MESSAGE(STATUS "Cmake Version        : ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
MESSAGE(STATUS "HTTPP Version        : ${HTTPP_VERSION}")
MESSAGE(STATUS "Build Type           : ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "Build Tests          : ${BUILD_TESTS}")
MESSAGE(STATUS "System               : ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
MESSAGE(STATUS "Install Prefix       : ${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "Source Directory     : ${HTTPP_SOURCE_DIR}")


IF (${CMAKE_CXX_COMPILER_ID} MATCHES GNU OR ${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  INCLUDE(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG(-std=c++11 HAVE_STD11)
  CHECK_CXX_COMPILER_FLAG(-Wno-unused-local-typedefs HAVE_NOUNUSED_LOCAL_TYPEDEF)

  IF (HAVE_NOUNUSED_LOCAL_TYPEDEF)
        ADD_DEFINITIONS("-Wno-unused-local-typedefs")
  ENDIF()

  IF (NOT HAVE_STD11)
      MESSAGE(FATAL_ERROR "No advanced standard C++ support (-std=c++11 not defined).")
  ENDIF()

  ADD_DEFINITIONS("-std=c++11 -Wextra -Wall")
  IF (${CMAKE_BUILD_TYPE} MATCHES "DEBUG")
      ADD_DEFINITIONS("-ggdb -g3")
  ENDIF ()

  IF (${PROFILE_HTTPP})
      MESSAGE(STATUS "HTTPP Profile enabled")
      SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -pg)
      SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -pg)
      SET(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} -pg)
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

# Actual Stuff ################################################################
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${HTTPP_SOURCE_DIR}/CMakeScripts)

#Thread
FIND_PACKAGE(Threads REQUIRED)

# TBB, commonpp dependency
FIND_PACKAGE(TBB REQUIRED)

#TCMALLOC
FIND_PACKAGE(TCMALLOC)

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

SET(DEFAULT_LIBRARIES
    ${commonpp_LIBRARIES}
    ${TBB_LIBRARIES}
    ${Boost_LIBRARIES}
    ${CURL_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_LIBRARIES})

IF (UNIX AND NOT APPLE)
    SET (DEFAULT_LIBRARIES ${DEFAULT_LIBRARIES} rt)
ENDIF()

IF (TCMALLOC_FOUND)
    SET(DEFAULT_LIBRARIES
        ${DEFAULT_LIBRARIES}
        ${TCMALLOC_LIBRARIES})
ENDIF()


INCLUDE_DIRECTORIES(include/)
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

FILE(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
SET(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include" "${VERSION_GENERATED_DIR}")
CONFIGURE_FILE("HTTPPConfig.cmake.in"
               "${PROJECT_BINARY_DIR}/HTTPConfig.cmake" @ONLY)


SET(CONF_INCLUDE_DIRS "\${HTTPP_CMAKE_DIR}/${REL_INCLUDE_DIR}")
CONFIGURE_FILE("HTTPPConfig.cmake.in"
               "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/HTTPPConfig.cmake" @ONLY)

INSTALL(FILES
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/HTTPPConfig.cmake"
        "${PROJECT_BINARY_DIR}/HTTPPConfigVersion.cmake"
        DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

INSTALL(EXPORT httpp_target
        DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)

# CPack stuff ##################################################################

SET(CPACK_SET_DESTDIR "ON")
SET(PACKAGE_NAME "HTTPP")
SET(PACKAGE_FILE_NAME "${PACKAGE_NAME}")

SET(CPACK_PACKAGE_NAME "${PACKAGE_FILE_NAME}")
SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE_FILE_NAME}")
SET(CPACK_DEBIAN_PACKAGE_NAME "${CPACK_PACKAGE_NAME}-dev")
SET(CPACK_COMPONENTS_ALL binaries examples dev)
SET(CPACK_PACKAGE_VENDOR "Thomas Sanchez")
SET(CPACK_PACKAGE_CONTACT "Thomas Sanchez <thomas.sanchz@gmail.com>")
SET(LIB_INSTALL_DIR, ${CMAKE_INSTALL_PREFIX})
SET(BIN_INSTALL_DIR, ${CMAKE_INSTALL_PREFIX})
SET(INCLUDE_INSTALL_DIR, ${CMAKE_INSTALL_PREFIX})

IF (UNIX)
    SET(CPACK_GENERATOR "DEB;TGZ")
ENDIF (UNIX)

INCLUDE(CPack)
