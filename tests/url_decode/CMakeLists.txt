# Part of HTTPP.
#
# Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
#

set(MODULE "url")
add_definitions("-DBOOST_TEST_MODULE=${MODULE}")
ADD_HTTPP_TEST(decode)
