# Part of HTTPP.
#
# Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
#

MACRO(ADD_HTTPP_EXAMPLE ex_name sources)

    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/httpp/include)
    ADD_EXECUTABLE(${ex_name} ${sources})
    TARGET_LINK_LIBRARIES(${ex_name} httpp ${DEFAULT_LIBRARIES})

ENDMACRO()

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
