# Part of HTTPP.
#
# Distributed under the 3-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
#

MACRO(CHECK_HAS_THREAD_LOCAL_SPECIFIER)
    UNSET(HAS_THREAD_LOCAL_SPECIFIER)
    INCLUDE (CheckCXXSourceCompiles)
    SET(SAVE ${CMAKE_REQUIRED_FLAGS})
    SET(CMAKE_REQUIRED_FLAGS "-std=c++11")
    CHECK_CXX_SOURCE_COMPILES("
#include <memory>

struct MyStruct
{
    void init()
    {
        if (!myptr)
        {
            myptr.reset(new MyStruct);
        }
    }

    static thread_local std::unique_ptr<MyStruct> myptr;
};

thread_local std::unique_ptr<MyStruct> MyStruct::myptr;

int main(int, char**)
{

    thread_local int i;
    MyStruct mstruct;
    mstruct.init();
    return 0;
}
"
    HAS_THREAD_LOCAL_SPECIFIER)
    SET(CMAKE_REQUIRED_FLAGS ${SAVE})
ENDMACRO()
