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

set(MODULE "Client")
add_definitions("-DBOOST_TEST_MODULE=${MODULE}")

ADD_HTTPP_TEST(post_content)
ADD_HTTPP_TEST(header)
ADD_HTTPP_TEST(error)
ADD_HTTPP_TEST(query)
ADD_HTTPP_TEST(post_data)
ADD_HTTPP_TEST(follow_redirect)
ADD_HTTPP_TEST(continue)
ADD_HTTPP_TEST(completion_handler)
ADD_HTTPP_TEST(google)
ADD_HTTPP_TEST(cancel_async_operation)
ADD_HTTPP_TEST(delete)
ADD_HTTPP_TEST(file)
