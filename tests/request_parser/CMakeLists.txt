# Part of HTTPP.
#
# Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2013 Thomas Sanchez.  All rights reserved.
#

set(MODULE "Parser")
add_definitions("-DBOOST_TEST_MODULE=${MODULE}")

ADD_HTTPP_TEST(query)
ADD_HTTPP_TEST(parser_streambuf)
ADD_HTTPP_TEST(headers)
