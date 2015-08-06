# Part of HTTPP.
#
# Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
#

FIND_LIBRARY(TCMALLOC_LIBRARIES
  NAMES
    tcmalloc
    tcmalloc_minimal

  PATHS
    /usr/lib
    /usr/local/lib
    /opt/local/lib
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TCMALLOC DEFAULT_MSG TCMALLOC_LIBRARIES)
MARK_AS_ADVANCED(TCMALLOC_LIBRARIES)
