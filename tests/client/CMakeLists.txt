# Part of HTTPP.
#
# Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
# project root).
#
# Copyright (c) 2014 Thomas Sanchez.  All rights reserved.
#

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})


ADD_HTTPP_TEST(Client post_content)
ADD_HTTPP_TEST(Client header)
ADD_HTTPP_TEST(Client error)
ADD_HTTPP_TEST(Client query)
ADD_HTTPP_TEST(Client post_data)
ADD_HTTPP_TEST(Client follow_redirect)
ADD_HTTPP_TEST(Client continue)
ADD_HTTPP_TEST(Client completion_handler)
ADD_HTTPP_TEST(Client google)
ADD_HTTPP_TEST(Client cancel_async_operation)
ADD_HTTPP_TEST(Client delete)
ADD_HTTPP_TEST(Client file)
