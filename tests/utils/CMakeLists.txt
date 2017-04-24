# Part of HTTPP.
#
# Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2017 Thomas Sanchez.  All rights reserved.
#

set(MODULE "utils")
add_definitions("-DBOOST_TEST_MODULE=${MODULE}")
ADD_HTTPP_TEST(sorted_vector)
