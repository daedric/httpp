FIND_PATH(HTTPP_INCLUDE_DIRS
    NAMES
        httpp/version.hpp
    PATH_SUFFIXES
        include
    PATHS
        /usr
        /usr/local
        /opt/local
        $ENV{HTTPP_DIR}
    )


FIND_LIBRARY(HTTPP_LIBRARIES
    NAMES
        httpp
    PATH_SUFFIXES
        lib
    PATHS
        /usr
        /usr/local
        /opt/local
        $ENV{HTTPP_DIR}
    )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HTTPP DEFAULT_MSG HTTPP_LIBRARIES HTTPP_INCLUDE_DIRS)

MARK_AS_ADVANCED(HTTPP_INCLUDE_DIRS HTTPP_LIBRARIES)
