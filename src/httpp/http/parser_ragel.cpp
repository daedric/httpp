
#line 1 "parser.rl"
/*
 * Part of HTTPP.
 *
 * Distributed under the 2-clause BSD licence (See LICENCE.TXT file at the
 * project root).
 *
 * Copyright (c) 2015 Thomas Sanchez.  All rights reserved.
 *
 */

#include <vector>
#include <string>

#include <commonpp/core/LoggingInterface.hpp>

#include "httpp/http/Parser.hpp"
#include "httpp/http/Request.hpp"
#include "httpp/utils/URL.hpp"
#include "httpp/utils/LazyDecodedValue.hpp"

#if HTTPP_PARSER_BACKEND == HTTPP_RAGEL_BACKEND

#define TOKEN_LEN size_t(token_end - token_begin)
#define TOKEN_REF boost::string_ref(token_begin, TOKEN_LEN)


#line 30 "parser_ragel.cpp"
static const int http_start = 1;
static const int http_first_final = 61;
static const int http_error = 0;

static const int http_en_main = 1;


#line 29 "parser.rl"



#line 99 "parser.rl"


namespace HTTPP { namespace HTTP {
bool Parser::parse(const char* start,
                   const char* end,
                   size_t& consumed,
                   Request& request)
{
    const char *p = start;
    int cs;

    const char *token_begin, *token_end;

    
#line 57 "parser_ragel.cpp"
	{
	cs = http_start;
	}

#line 62 "parser_ragel.cpp"
	{
	switch ( cs )
	{
case 1:
	switch( (*p) ) {
		case 67: {
goto ctr0;
	}
		case 68: {
goto ctr2;
	}
		case 71: {
goto ctr3;
	}
		case 72: {
goto ctr4;
	}
		case 79: {
goto ctr5;
	}
		case 80: {
goto ctr6;
	}
		case 84: {
goto ctr7;
	}
	}
{
	goto st0;
}
st0:
cs = 0;
	goto _out;
ctr0:
#line 82 "parser.rl"
	{
    token_begin = p;
}
	goto st2;
st2:
	p += 1;
case 2:
#line 105 "parser_ragel.cpp"
	if ( (*p) == 79 ) {
		goto st3;
	}
{
	goto st0;
}
st3:
	p += 1;
case 3:
	if ( (*p) == 78 ) {
		goto st4;
	}
{
	goto st0;
}
st4:
	p += 1;
case 4:
	if ( (*p) == 78 ) {
		goto st5;
	}
{
	goto st0;
}
st5:
	p += 1;
case 5:
	if ( (*p) == 69 ) {
		goto st6;
	}
{
	goto st0;
}
st6:
	p += 1;
case 6:
	if ( (*p) == 67 ) {
		goto st7;
	}
{
	goto st0;
}
st7:
	p += 1;
case 7:
	if ( (*p) == 84 ) {
		goto ctr13;
	}
{
	goto st0;
}
ctr13:
#line 86 "parser.rl"
	{
    try
    {
        request.method = method_from(token_begin);
    }
    catch(...)
    {
        {p++; cs = 8; goto _out;}
    }

    token_begin = nullptr;
}
	goto st8;
st8:
	p += 1;
case 8:
#line 175 "parser_ragel.cpp"
	if ( (*p) == 32 ) {
		goto st9;
	}
	if ( 9 <= (*p) && (*p) <= 13 ) {
		goto st9;
	}
{
	goto st0;
}
st9:
	p += 1;
case 9:
	switch( (*p) ) {
		case 32: {
goto st0;
	}
		case 63: {
goto st0;
	}
	}
	if ( (*p) < 14 ) {
		if ( 9 <= (*p) ) {
			goto st0;
		}
	} else if ( (*p) > 31 ) {
		if ( (*p) > 62 ) {
			if ( 64 <= (*p) ) {
				goto ctr15;
			}
		} else if ( (*p) >= 33 ) {
			goto ctr15;
		}
	} else {
		goto ctr15;
	}
{
	goto ctr15;
}
ctr15:
#line 32 "parser.rl"
	{
    token_begin = p;
}
	goto st10;
st10:
	p += 1;
case 10:
#line 223 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: {
goto ctr17;
	}
		case 63: {
goto ctr18;
	}
	}
	if ( (*p) < 14 ) {
		if ( 9 <= (*p) ) {
			goto ctr17;
		}
	} else if ( (*p) > 31 ) {
		if ( (*p) > 62 ) {
			if ( 64 <= (*p) ) {
				goto st10;
			}
		} else if ( (*p) >= 33 ) {
			goto st10;
		}
	} else {
		goto st10;
	}
{
	goto st10;
}
ctr17:
#line 36 "parser.rl"
	{
    token_end = p;
    request.uri = TOKEN_REF;
    token_begin = token_end = nullptr;
}
	goto st11;
ctr58:
#line 62 "parser.rl"
	{
    token_begin = p;
}
#line 66 "parser.rl"
	{
    token_end = p;
    request.query_params.emplace_back(std::make_pair<std::string, boost::string_ref>({UTILS::url_decode(token_begin, token_end)}, ""));
    token_begin = token_end = nullptr;
}
	goto st11;
ctr62:
#line 66 "parser.rl"
	{
    token_end = p;
    request.query_params.emplace_back(std::make_pair<std::string, boost::string_ref>({UTILS::url_decode(token_begin, token_end)}, ""));
    token_begin = token_end = nullptr;
}
	goto st11;
ctr66:
#line 72 "parser.rl"
	{
    token_begin = p;
}
#line 76 "parser.rl"
	{
    token_end = p;
    request.query_params.back().second = TOKEN_REF;
    token_begin = token_end = nullptr;
}
	goto st11;
ctr69:
#line 76 "parser.rl"
	{
    token_end = p;
    request.query_params.back().second = TOKEN_REF;
    token_begin = token_end = nullptr;
}
	goto st11;
st11:
	p += 1;
case 11:
#line 301 "parser_ragel.cpp"
	if ( (*p) == 72 ) {
		goto st12;
	}
{
	goto st0;
}
st12:
	p += 1;
case 12:
	if ( (*p) == 84 ) {
		goto st13;
	}
{
	goto st0;
}
st13:
	p += 1;
case 13:
	if ( (*p) == 84 ) {
		goto st14;
	}
{
	goto st0;
}
st14:
	p += 1;
case 14:
	if ( (*p) == 80 ) {
		goto st15;
	}
{
	goto st0;
}
st15:
	p += 1;
case 15:
	if ( (*p) == 47 ) {
		goto st16;
	}
{
	goto st0;
}
st16:
	p += 1;
case 16:
	if ( 48 <= (*p) && (*p) <= 57 ) {
		goto ctr24;
	}
{
	goto st0;
}
ctr24:
#line 125 "parser.rl"
	{ request.major = (*p) - '0';}
	goto st17;
st17:
	p += 1;
case 17:
#line 360 "parser_ragel.cpp"
	if ( (*p) == 46 ) {
		goto st18;
	}
{
	goto st0;
}
st18:
	p += 1;
case 18:
	if ( 48 <= (*p) && (*p) <= 57 ) {
		goto ctr26;
	}
{
	goto st0;
}
ctr26:
#line 126 "parser.rl"
	{request.minor = (*p) - '0';}
	goto st19;
st19:
	p += 1;
case 19:
#line 383 "parser_ragel.cpp"
	if ( (*p) == 13 ) {
		goto st20;
	}
{
	goto st0;
}
st20:
	p += 1;
case 20:
	if ( (*p) == 10 ) {
		goto st21;
	}
{
	goto st0;
}
st21:
	p += 1;
case 21:
	switch( (*p) ) {
		case 13: {
goto st22;
	}
		case 45: {
goto ctr30;
	}
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 ) {
			goto ctr30;
		}
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 ) {
			goto ctr30;
		}
	} else {
		goto ctr30;
	}
{
	goto st0;
}
st22:
	p += 1;
case 22:
	if ( (*p) == 10 ) {
		goto st61;
	}
{
	goto st0;
}
st61:
#line 134 "parser.rl"
	{
                    {p++; cs = 61; goto _out;}
                }
	p += 1;
case 61:
#line 440 "parser_ragel.cpp"
{
	goto st0;
}
ctr30:
#line 42 "parser.rl"
	{
    token_begin = p;
}
	goto st23;
st23:
	p += 1;
case 23:
#line 453 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: {
goto ctr32;
	}
		case 45: {
goto st23;
	}
		case 58: {
goto ctr34;
	}
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 13 ) {
			goto ctr32;
		}
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 ) {
				goto st23;
			}
		} else if ( (*p) >= 65 ) {
			goto st23;
		}
	} else {
		goto st23;
	}
{
	goto st0;
}
ctr32:
#line 46 "parser.rl"
	{
    token_end = p;
    request.headers.emplace_back(std::make_pair<boost::string_ref>(TOKEN_REF, ""));
    token_begin = token_end = nullptr;
}
	goto st24;
st24:
	p += 1;
case 24:
#line 494 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: {
goto st24;
	}
		case 58: {
goto st25;
	}
	}
	if ( 9 <= (*p) && (*p) <= 13 ) {
		goto st24;
	}
{
	goto st0;
}
ctr34:
#line 46 "parser.rl"
	{
    token_end = p;
    request.headers.emplace_back(std::make_pair<boost::string_ref>(TOKEN_REF, ""));
    token_begin = token_end = nullptr;
}
	goto st25;
st25:
	p += 1;
case 25:
#line 520 "parser_ragel.cpp"
	switch( (*p) ) {
		case 13: {
goto ctr39;
	}
		case 32: {
goto ctr38;
	}
	}
	if ( (*p) < 14 ) {
		if ( 9 <= (*p) && (*p) <= 12 ) {
			goto ctr38;
		}
	} else if ( (*p) > 31 ) {
		if ( 33 <= (*p) ) {
			goto ctr37;
		}
	} else {
		goto ctr37;
	}
{
	goto ctr37;
}
ctr37:
#line 52 "parser.rl"
	{
    token_begin = p;
}
	goto st26;
st26:
	p += 1;
case 26:
#line 552 "parser_ragel.cpp"
	if ( (*p) == 13 ) {
		goto ctr41;
	}
	if ( 14 <= (*p) ) {
		goto st26;
	}
{
	goto st26;
}
ctr41:
#line 56 "parser.rl"
	{
    token_end = p;
    request.headers.back().second = TOKEN_REF;
    token_begin = token_end = nullptr;
}
	goto st27;
st27:
	p += 1;
case 27:
#line 573 "parser_ragel.cpp"
	switch( (*p) ) {
		case 10: {
goto st21;
	}
		case 13: {
goto ctr41;
	}
	}
	if ( (*p) > 12 ) {
		if ( 14 <= (*p) ) {
			goto st26;
		}
	} else if ( (*p) >= 11 ) {
		goto st26;
	}
{
	goto st26;
}
ctr50:
#line 46 "parser.rl"
	{
    token_end = p;
    request.headers.emplace_back(std::make_pair<boost::string_ref>(TOKEN_REF, ""));
    token_begin = token_end = nullptr;
}
	goto st28;
ctr38:
#line 52 "parser.rl"
	{
    token_begin = p;
}
	goto st28;
st28:
	p += 1;
case 28:
#line 609 "parser_ragel.cpp"
	switch( (*p) ) {
		case 13: {
goto ctr42;
	}
		case 32: {
goto ctr38;
	}
	}
	if ( (*p) < 14 ) {
		if ( 9 <= (*p) && (*p) <= 12 ) {
			goto ctr38;
		}
	} else if ( (*p) > 31 ) {
		if ( 33 <= (*p) ) {
			goto ctr37;
		}
	} else {
		goto ctr37;
	}
{
	goto ctr37;
}
ctr39:
#line 52 "parser.rl"
	{
    token_begin = p;
}
	goto st29;
ctr42:
#line 52 "parser.rl"
	{
    token_begin = p;
}
#line 56 "parser.rl"
	{
    token_end = p;
    request.headers.back().second = TOKEN_REF;
    token_begin = token_end = nullptr;
}
	goto st29;
st29:
	p += 1;
case 29:
#line 653 "parser_ragel.cpp"
	switch( (*p) ) {
		case 9: {
goto ctr38;
	}
		case 10: {
goto ctr43;
	}
		case 13: {
goto ctr42;
	}
		case 32: {
goto ctr38;
	}
	}
	if ( (*p) < 14 ) {
		if ( 11 <= (*p) && (*p) <= 12 ) {
			goto ctr38;
		}
	} else if ( (*p) > 31 ) {
		if ( 33 <= (*p) ) {
			goto ctr37;
		}
	} else {
		goto ctr37;
	}
{
	goto ctr37;
}
ctr43:
#line 52 "parser.rl"
	{
    token_begin = p;
}
	goto st30;
st30:
	p += 1;
case 30:
#line 691 "parser_ragel.cpp"
	switch( (*p) ) {
		case 13: {
goto ctr44;
	}
		case 32: {
goto ctr38;
	}
		case 45: {
goto ctr45;
	}
	}
	if ( (*p) < 48 ) {
		if ( (*p) < 14 ) {
			if ( 9 <= (*p) && (*p) <= 12 ) {
				goto ctr38;
			}
		} else if ( (*p) > 31 ) {
			if ( (*p) > 44 ) {
				if ( 46 <= (*p) ) {
					goto ctr37;
				}
			} else if ( (*p) >= 33 ) {
				goto ctr37;
			}
		} else {
			goto ctr37;
		}
	} else if ( (*p) > 57 ) {
		if ( (*p) < 91 ) {
			if ( (*p) > 64 ) {
				{
					goto ctr45;
				}
			} else {
				goto ctr37;
			}
		} else if ( (*p) > 96 ) {
			if ( (*p) > 122 ) {
				{
					goto ctr37;
				}
			} else {
				goto ctr45;
			}
		} else {
			goto ctr37;
		}
	} else {
		goto ctr45;
	}
{
	goto ctr37;
}
ctr44:
#line 52 "parser.rl"
	{
    token_begin = p;
}
#line 56 "parser.rl"
	{
    token_end = p;
    request.headers.back().second = TOKEN_REF;
    token_begin = token_end = nullptr;
}
	goto st31;
st31:
	p += 1;
case 31:
#line 760 "parser_ragel.cpp"
	switch( (*p) ) {
		case 9: {
goto ctr38;
	}
		case 10: {
goto ctr46;
	}
		case 13: {
goto ctr42;
	}
		case 32: {
goto ctr38;
	}
	}
	if ( (*p) < 14 ) {
		if ( 11 <= (*p) && (*p) <= 12 ) {
			goto ctr38;
		}
	} else if ( (*p) > 31 ) {
		if ( 33 <= (*p) ) {
			goto ctr37;
		}
	} else {
		goto ctr37;
	}
{
	goto ctr37;
}
ctr46:
#line 52 "parser.rl"
	{
    token_begin = p;
}
	goto st62;
st62:
#line 134 "parser.rl"
	{
                    {p++; cs = 62; goto _out;}
                }
	p += 1;
case 62:
#line 802 "parser_ragel.cpp"
	switch( (*p) ) {
		case 13: {
goto ctr44;
	}
		case 32: {
goto ctr38;
	}
		case 45: {
goto ctr45;
	}
	}
	if ( (*p) < 48 ) {
		if ( (*p) < 14 ) {
			if ( 9 <= (*p) && (*p) <= 12 ) {
				goto ctr38;
			}
		} else if ( (*p) > 31 ) {
			if ( (*p) > 44 ) {
				if ( 46 <= (*p) ) {
					goto ctr37;
				}
			} else if ( (*p) >= 33 ) {
				goto ctr37;
			}
		} else {
			goto ctr37;
		}
	} else if ( (*p) > 57 ) {
		if ( (*p) < 91 ) {
			if ( (*p) > 64 ) {
				{
					goto ctr45;
				}
			} else {
				goto ctr37;
			}
		} else if ( (*p) > 96 ) {
			if ( (*p) > 122 ) {
				{
					goto ctr37;
				}
			} else {
				goto ctr45;
			}
		} else {
			goto ctr37;
		}
	} else {
		goto ctr45;
	}
{
	goto ctr37;
}
ctr45:
#line 52 "parser.rl"
	{
    token_begin = p;
}
#line 42 "parser.rl"
	{
    token_begin = p;
}
	goto st32;
st32:
	p += 1;
case 32:
#line 869 "parser_ragel.cpp"
	switch( (*p) ) {
		case 13: {
goto ctr48;
	}
		case 32: {
goto ctr47;
	}
		case 45: {
goto st32;
	}
		case 58: {
goto ctr50;
	}
	}
	if ( (*p) < 48 ) {
		if ( (*p) < 14 ) {
			if ( 9 <= (*p) && (*p) <= 12 ) {
				goto ctr47;
			}
		} else if ( (*p) > 31 ) {
			if ( (*p) > 44 ) {
				if ( 46 <= (*p) ) {
					goto st26;
				}
			} else if ( (*p) >= 33 ) {
				goto st26;
			}
		} else {
			goto st26;
		}
	} else if ( (*p) > 57 ) {
		if ( (*p) < 91 ) {
			if ( (*p) > 64 ) {
				{
					goto st32;
				}
			} else if ( (*p) >= 59 ) {
				goto st26;
			}
		} else if ( (*p) > 96 ) {
			if ( (*p) > 122 ) {
				{
					goto st26;
				}
			} else {
				goto st32;
			}
		} else {
			goto st26;
		}
	} else {
		goto st32;
	}
{
	goto st26;
}
ctr47:
#line 46 "parser.rl"
	{
    token_end = p;
    request.headers.emplace_back(std::make_pair<boost::string_ref>(TOKEN_REF, ""));
    token_begin = token_end = nullptr;
}
	goto st33;
st33:
	p += 1;
case 33:
#line 937 "parser_ragel.cpp"
	switch( (*p) ) {
		case 13: {
goto ctr52;
	}
		case 32: {
goto st33;
	}
		case 58: {
goto st28;
	}
	}
	if ( (*p) < 14 ) {
		if ( 9 <= (*p) && (*p) <= 12 ) {
			goto st33;
		}
	} else if ( (*p) > 31 ) {
		if ( (*p) > 57 ) {
			if ( 59 <= (*p) ) {
				goto st26;
			}
		} else if ( (*p) >= 33 ) {
			goto st26;
		}
	} else {
		goto st26;
	}
{
	goto st26;
}
ctr52:
#line 56 "parser.rl"
	{
    token_end = p;
    request.headers.back().second = TOKEN_REF;
    token_begin = token_end = nullptr;
}
	goto st34;
ctr48:
#line 46 "parser.rl"
	{
    token_end = p;
    request.headers.emplace_back(std::make_pair<boost::string_ref>(TOKEN_REF, ""));
    token_begin = token_end = nullptr;
}
#line 56 "parser.rl"
	{
    token_end = p;
    request.headers.back().second = TOKEN_REF;
    token_begin = token_end = nullptr;
}
	goto st34;
st34:
	p += 1;
case 34:
#line 992 "parser_ragel.cpp"
	switch( (*p) ) {
		case 9: {
goto st33;
	}
		case 10: {
goto st35;
	}
		case 13: {
goto ctr52;
	}
		case 32: {
goto st33;
	}
		case 58: {
goto st28;
	}
	}
	if ( (*p) < 14 ) {
		if ( 11 <= (*p) && (*p) <= 12 ) {
			goto st33;
		}
	} else if ( (*p) > 31 ) {
		if ( (*p) > 57 ) {
			if ( 59 <= (*p) ) {
				goto st26;
			}
		} else if ( (*p) >= 33 ) {
			goto st26;
		}
	} else {
		goto st26;
	}
{
	goto st26;
}
st35:
	p += 1;
case 35:
	switch( (*p) ) {
		case 13: {
goto st36;
	}
		case 32: {
goto st24;
	}
		case 45: {
goto ctr30;
	}
		case 58: {
goto st25;
	}
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 12 ) {
			goto st24;
		}
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 ) {
				goto ctr30;
			}
		} else if ( (*p) >= 65 ) {
			goto ctr30;
		}
	} else {
		goto ctr30;
	}
{
	goto st0;
}
st36:
	p += 1;
case 36:
	switch( (*p) ) {
		case 9: {
goto st24;
	}
		case 10: {
goto st63;
	}
		case 32: {
goto st24;
	}
		case 58: {
goto st25;
	}
	}
	if ( 11 <= (*p) && (*p) <= 13 ) {
		goto st24;
	}
{
	goto st0;
}
st63:
#line 134 "parser.rl"
	{
                    {p++; cs = 63; goto _out;}
                }
	p += 1;
case 63:
#line 1093 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: {
goto st24;
	}
		case 58: {
goto st25;
	}
	}
	if ( 9 <= (*p) && (*p) <= 13 ) {
		goto st24;
	}
{
	goto st0;
}
ctr18:
#line 36 "parser.rl"
	{
    token_end = p;
    request.uri = TOKEN_REF;
    token_begin = token_end = nullptr;
}
	goto st37;
ctr59:
#line 62 "parser.rl"
	{
    token_begin = p;
}
#line 66 "parser.rl"
	{
    token_end = p;
    request.query_params.emplace_back(std::make_pair<std::string, boost::string_ref>({UTILS::url_decode(token_begin, token_end)}, ""));
    token_begin = token_end = nullptr;
}
	goto st37;
ctr63:
#line 66 "parser.rl"
	{
    token_end = p;
    request.query_params.emplace_back(std::make_pair<std::string, boost::string_ref>({UTILS::url_decode(token_begin, token_end)}, ""));
    token_begin = token_end = nullptr;
}
	goto st37;
ctr67:
#line 72 "parser.rl"
	{
    token_begin = p;
}
#line 76 "parser.rl"
	{
    token_end = p;
    request.query_params.back().second = TOKEN_REF;
    token_begin = token_end = nullptr;
}
	goto st37;
ctr70:
#line 76 "parser.rl"
	{
    token_end = p;
    request.query_params.back().second = TOKEN_REF;
    token_begin = token_end = nullptr;
}
	goto st37;
st37:
	p += 1;
case 37:
#line 1159 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: {
goto ctr58;
	}
		case 38: {
goto ctr59;
	}
		case 61: {
goto ctr60;
	}
	}
	if ( (*p) < 33 ) {
		if ( (*p) > 13 ) {
			if ( (*p) <= 31 ) {
				goto ctr57;
			}
		} else if ( (*p) >= 9 ) {
			goto ctr58;
		}
	} else if ( (*p) > 37 ) {
		if ( (*p) > 60 ) {
			if ( 62 <= (*p) ) {
				goto ctr57;
			}
		} else if ( (*p) >= 39 ) {
			goto ctr57;
		}
	} else {
		goto ctr57;
	}
{
	goto ctr57;
}
ctr57:
#line 62 "parser.rl"
	{
    token_begin = p;
}
	goto st38;
st38:
	p += 1;
case 38:
#line 1202 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: {
goto ctr62;
	}
		case 38: {
goto ctr63;
	}
		case 61: {
goto ctr64;
	}
	}
	if ( (*p) < 33 ) {
		if ( (*p) > 13 ) {
			if ( (*p) <= 31 ) {
				goto st38;
			}
		} else if ( (*p) >= 9 ) {
			goto ctr62;
		}
	} else if ( (*p) > 37 ) {
		if ( (*p) > 60 ) {
			if ( 62 <= (*p) ) {
				goto st38;
			}
		} else if ( (*p) >= 39 ) {
			goto st38;
		}
	} else {
		goto st38;
	}
{
	goto st38;
}
ctr60:
#line 62 "parser.rl"
	{
    token_begin = p;
}
#line 66 "parser.rl"
	{
    token_end = p;
    request.query_params.emplace_back(std::make_pair<std::string, boost::string_ref>({UTILS::url_decode(token_begin, token_end)}, ""));
    token_begin = token_end = nullptr;
}
	goto st39;
ctr64:
#line 66 "parser.rl"
	{
    token_end = p;
    request.query_params.emplace_back(std::make_pair<std::string, boost::string_ref>({UTILS::url_decode(token_begin, token_end)}, ""));
    token_begin = token_end = nullptr;
}
	goto st39;
st39:
	p += 1;
case 39:
#line 1259 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: {
goto ctr66;
	}
		case 38: {
goto ctr67;
	}
	}
	if ( (*p) < 14 ) {
		if ( 9 <= (*p) ) {
			goto ctr66;
		}
	} else if ( (*p) > 31 ) {
		if ( (*p) > 37 ) {
			if ( 39 <= (*p) ) {
				goto ctr65;
			}
		} else if ( (*p) >= 33 ) {
			goto ctr65;
		}
	} else {
		goto ctr65;
	}
{
	goto ctr65;
}
ctr65:
#line 72 "parser.rl"
	{
    token_begin = p;
}
	goto st40;
st40:
	p += 1;
case 40:
#line 1295 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: {
goto ctr69;
	}
		case 38: {
goto ctr70;
	}
	}
	if ( (*p) < 14 ) {
		if ( 9 <= (*p) ) {
			goto ctr69;
		}
	} else if ( (*p) > 31 ) {
		if ( (*p) > 37 ) {
			if ( 39 <= (*p) ) {
				goto st40;
			}
		} else if ( (*p) >= 33 ) {
			goto st40;
		}
	} else {
		goto st40;
	}
{
	goto st40;
}
ctr2:
#line 82 "parser.rl"
	{
    token_begin = p;
}
	goto st41;
st41:
	p += 1;
case 41:
#line 1331 "parser_ragel.cpp"
	if ( (*p) == 69 ) {
		goto st42;
	}
{
	goto st0;
}
st42:
	p += 1;
case 42:
	if ( (*p) == 76 ) {
		goto st43;
	}
{
	goto st0;
}
st43:
	p += 1;
case 43:
	if ( (*p) == 69 ) {
		goto st44;
	}
{
	goto st0;
}
st44:
	p += 1;
case 44:
	if ( (*p) == 84 ) {
		goto st45;
	}
{
	goto st0;
}
st45:
	p += 1;
case 45:
	if ( (*p) == 69 ) {
		goto ctr13;
	}
{
	goto st0;
}
ctr3:
#line 82 "parser.rl"
	{
    token_begin = p;
}
	goto st46;
st46:
	p += 1;
case 46:
#line 1383 "parser_ragel.cpp"
	if ( (*p) == 69 ) {
		goto st7;
	}
{
	goto st0;
}
ctr4:
#line 82 "parser.rl"
	{
    token_begin = p;
}
	goto st47;
st47:
	p += 1;
case 47:
#line 1399 "parser_ragel.cpp"
	if ( (*p) == 69 ) {
		goto st48;
	}
{
	goto st0;
}
st48:
	p += 1;
case 48:
	if ( (*p) == 65 ) {
		goto st49;
	}
{
	goto st0;
}
st49:
	p += 1;
case 49:
	if ( (*p) == 68 ) {
		goto ctr13;
	}
{
	goto st0;
}
ctr5:
#line 82 "parser.rl"
	{
    token_begin = p;
}
	goto st50;
st50:
	p += 1;
case 50:
#line 1433 "parser_ragel.cpp"
	if ( (*p) == 80 ) {
		goto st51;
	}
{
	goto st0;
}
st51:
	p += 1;
case 51:
	if ( (*p) == 84 ) {
		goto st52;
	}
{
	goto st0;
}
st52:
	p += 1;
case 52:
	if ( (*p) == 73 ) {
		goto st53;
	}
{
	goto st0;
}
st53:
	p += 1;
case 53:
	if ( (*p) == 79 ) {
		goto st54;
	}
{
	goto st0;
}
st54:
	p += 1;
case 54:
	if ( (*p) == 78 ) {
		goto st55;
	}
{
	goto st0;
}
st55:
	p += 1;
case 55:
	if ( (*p) == 83 ) {
		goto ctr13;
	}
{
	goto st0;
}
ctr6:
#line 82 "parser.rl"
	{
    token_begin = p;
}
	goto st56;
st56:
	p += 1;
case 56:
#line 1494 "parser_ragel.cpp"
	switch( (*p) ) {
		case 79: {
goto st57;
	}
		case 85: {
goto st7;
	}
	}
{
	goto st0;
}
st57:
	p += 1;
case 57:
	if ( (*p) == 83 ) {
		goto st7;
	}
{
	goto st0;
}
ctr7:
#line 82 "parser.rl"
	{
    token_begin = p;
}
	goto st58;
st58:
	p += 1;
case 58:
#line 1524 "parser_ragel.cpp"
	if ( (*p) == 82 ) {
		goto st59;
	}
{
	goto st0;
}
st59:
	p += 1;
case 59:
	if ( (*p) == 65 ) {
		goto st60;
	}
{
	goto st0;
}
st60:
	p += 1;
case 60:
	if ( (*p) == 67 ) {
		goto st45;
	}
{
	goto st0;
}
	}

	_out: {}
	}

#line 140 "parser.rl"


    if (cs < http_first_final)
    {
        GLOG(error) << "Invalid request read, cannot parse: " << std::string(p, end);
        return false;
    }

    consumed = p - start;
    return true;
}

} }
#endif
