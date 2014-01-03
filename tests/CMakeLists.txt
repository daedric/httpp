# Part of HTTPP.
#
# Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
#

MACRO(ADD_HTTPP_TEST MODULE TEST_NAME)
    SET(EXE_NAME Test_${TEST_NAME})
    # current project
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/httpp/include)
    ADD_DEFINITIONS("-DBOOST_TEST_MAIN -DBOOST_TEST_MODULE=${MODULE}")
    ADD_EXECUTABLE(${EXE_NAME} ${TEST_NAME}.cpp)

    IF (${BUILD_SHARED_LIBS})
        ADD_DEFINITIONS("-DBOOST_TEST_DYN_LINK=1")
    ENDIF()

    TARGET_LINK_LIBRARIES(${EXE_NAME} httpp ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${ARGN})
    ADD_TEST(${MODULE}_${TEST_NAME} ${EXE_NAME})
    SET_TESTS_PROPERTIES(${MODULE}_${TEST_NAME} PROPERTIES
                         FAIL_REGULAR_EXPRESSION "FAILED")
ENDMACRO(ADD_HTTPP_TEST TEST_NAME)

# from: http://stackoverflow.com/questions/7787823/cmake-how-to-get-the-name-of-all-subdirectories-of-a-directory
MACRO(SUBDIRLIST result curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(dirlist "")
    FOREACH(child ${children})
        IF(IS_DIRECTORY ${curdir}/${child})
            SET(dirlist ${dirlist} ${child})
        ENDIF()
    ENDFOREACH()
    SET(${result} ${dirlist})
ENDMACRO()

SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})

FOREACH(subdir ${SUBDIRS})
        ADD_SUBDIRECTORY(${subdir})
ENDFOREACH()

