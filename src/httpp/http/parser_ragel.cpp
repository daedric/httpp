
#line 1 "parser.rl"
#include <iostream>
#include <vector>
#include <string>

#include "httpp/http/Parser.hpp"
#include "httpp/http/Request.hpp"
#include "httpp/utils/URL.hpp"


#line 13 "parser_ragel.cpp"
static const int http_start = 1;
static const int http_first_final = 61;
static const int http_error = 0;

static const int http_en_main = 1;


#line 12 "parser.rl"



#line 81 "parser.rl"


namespace HTTPP { namespace HTTP {
bool Parser::parse(const char* start,
                   const char* end,
                   size_t& consumed,
                   Request& request)
{
    const char *p = start;
    int cs;

    const char *token_begin, *token_end;

    
#line 40 "parser_ragel.cpp"
	{
	cs = http_start;
	}

#line 45 "parser_ragel.cpp"
	{
	switch ( cs )
	{
case 1:
	switch( (*p) ) {
		case 67: goto tr0;
		case 68: goto tr2;
		case 71: goto tr3;
		case 72: goto tr4;
		case 79: goto tr5;
		case 80: goto tr6;
		case 84: goto tr7;
	}
	goto st0;
st0:
cs = 0;
	goto _out;
tr0:
#line 65 "parser.rl"
	{
    token_begin = p;
}
	goto st2;
st2:
	p += 1;
case 2:
#line 72 "parser_ragel.cpp"
	if ( (*p) == 79 )
		goto st3;
	goto st0;
st3:
	p += 1;
case 3:
	if ( (*p) == 78 )
		goto st4;
	goto st0;
st4:
	p += 1;
case 4:
	if ( (*p) == 78 )
		goto st5;
	goto st0;
st5:
	p += 1;
case 5:
	if ( (*p) == 69 )
		goto st6;
	goto st0;
st6:
	p += 1;
case 6:
	if ( (*p) == 67 )
		goto st7;
	goto st0;
st7:
	p += 1;
case 7:
	if ( (*p) == 84 )
		goto tr13;
	goto st0;
tr13:
#line 69 "parser.rl"
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
#line 123 "parser_ragel.cpp"
	if ( (*p) == 32 )
		goto st9;
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st9;
	goto st0;
st9:
	p += 1;
case 9:
	switch( (*p) ) {
		case 32: goto st0;
		case 63: goto st0;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st0;
	goto tr15;
tr15:
#line 15 "parser.rl"
	{
    token_begin = p;
}
	goto st10;
st10:
	p += 1;
case 10:
#line 148 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: goto tr17;
		case 63: goto tr18;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto tr17;
	goto st10;
tr17:
#line 19 "parser.rl"
	{
    token_end = p;
    request.uri.assign(token_begin, token_end);
    token_begin = token_end = nullptr;
}
	goto st11;
tr58:
#line 45 "parser.rl"
	{
    token_begin = p;
}
#line 49 "parser.rl"
	{
    token_end = p;
    request.query_params.emplace_back(std::make_pair<std::string>({UTILS::decode({token_begin, token_end})}, ""));
    token_begin = token_end = nullptr;
}
	goto st11;
tr62:
#line 49 "parser.rl"
	{
    token_end = p;
    request.query_params.emplace_back(std::make_pair<std::string>({UTILS::decode({token_begin, token_end})}, ""));
    token_begin = token_end = nullptr;
}
	goto st11;
tr66:
#line 55 "parser.rl"
	{
    token_begin = p;
}
#line 59 "parser.rl"
	{
    token_end = p;
    request.query_params.back().second = UTILS::decode({token_begin, token_end});
    token_begin = token_end = nullptr;
}
	goto st11;
tr69:
#line 59 "parser.rl"
	{
    token_end = p;
    request.query_params.back().second = UTILS::decode({token_begin, token_end});
    token_begin = token_end = nullptr;
}
	goto st11;
st11:
	p += 1;
case 11:
#line 207 "parser_ragel.cpp"
	if ( (*p) == 72 )
		goto st12;
	goto st0;
st12:
	p += 1;
case 12:
	if ( (*p) == 84 )
		goto st13;
	goto st0;
st13:
	p += 1;
case 13:
	if ( (*p) == 84 )
		goto st14;
	goto st0;
st14:
	p += 1;
case 14:
	if ( (*p) == 80 )
		goto st15;
	goto st0;
st15:
	p += 1;
case 15:
	if ( (*p) == 47 )
		goto st16;
	goto st0;
st16:
	p += 1;
case 16:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr24;
	goto st0;
tr24:
#line 107 "parser.rl"
	{ request.major = (*p) - '0';}
	goto st17;
st17:
	p += 1;
case 17:
#line 248 "parser_ragel.cpp"
	if ( (*p) == 46 )
		goto st18;
	goto st0;
st18:
	p += 1;
case 18:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr26;
	goto st0;
tr26:
#line 108 "parser.rl"
	{request.minor = (*p) - '0';}
	goto st19;
st19:
	p += 1;
case 19:
#line 265 "parser_ragel.cpp"
	if ( (*p) == 13 )
		goto st20;
	goto st0;
st20:
	p += 1;
case 20:
	if ( (*p) == 10 )
		goto st21;
	goto st0;
st21:
	p += 1;
case 21:
	switch( (*p) ) {
		case 13: goto st22;
		case 45: goto tr30;
	}
	if ( (*p) < 65 ) {
		if ( 48 <= (*p) && (*p) <= 57 )
			goto tr30;
	} else if ( (*p) > 90 ) {
		if ( 97 <= (*p) && (*p) <= 122 )
			goto tr30;
	} else
		goto tr30;
	goto st0;
st22:
	p += 1;
case 22:
	if ( (*p) == 10 )
		goto st61;
	goto st0;
st61:
#line 116 "parser.rl"
	{
                    {p++; cs = 61; goto _out;}
                }
	p += 1;
case 61:
#line 304 "parser_ragel.cpp"
	goto st0;
tr30:
#line 25 "parser.rl"
	{
    token_begin = p;
}
	goto st23;
st23:
	p += 1;
case 23:
#line 315 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: goto tr32;
		case 45: goto st23;
		case 58: goto tr34;
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 13 )
			goto tr32;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st23;
		} else if ( (*p) >= 65 )
			goto st23;
	} else
		goto st23;
	goto st0;
tr32:
#line 29 "parser.rl"
	{
    token_end = p;
    request.headers.emplace_back(std::make_pair<std::string>({token_begin, token_end}, ""));
    token_begin = token_end = nullptr;
}
	goto st24;
st24:
	p += 1;
case 24:
#line 344 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: goto st24;
		case 58: goto st25;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st24;
	goto st0;
tr34:
#line 29 "parser.rl"
	{
    token_end = p;
    request.headers.emplace_back(std::make_pair<std::string>({token_begin, token_end}, ""));
    token_begin = token_end = nullptr;
}
	goto st25;
st25:
	p += 1;
case 25:
#line 363 "parser_ragel.cpp"
	switch( (*p) ) {
		case 13: goto tr39;
		case 32: goto tr38;
	}
	if ( 9 <= (*p) && (*p) <= 12 )
		goto tr38;
	goto tr37;
tr37:
#line 35 "parser.rl"
	{
    token_begin = p;
}
	goto st26;
st26:
	p += 1;
case 26:
#line 380 "parser_ragel.cpp"
	if ( (*p) == 13 )
		goto tr41;
	goto st26;
tr41:
#line 39 "parser.rl"
	{
    token_end = p;
    request.headers.back().second = {token_begin, token_end};
    token_begin = token_end = nullptr;
}
	goto st27;
st27:
	p += 1;
case 27:
#line 395 "parser_ragel.cpp"
	switch( (*p) ) {
		case 10: goto st21;
		case 13: goto tr41;
	}
	goto st26;
tr50:
#line 29 "parser.rl"
	{
    token_end = p;
    request.headers.emplace_back(std::make_pair<std::string>({token_begin, token_end}, ""));
    token_begin = token_end = nullptr;
}
	goto st28;
tr38:
#line 35 "parser.rl"
	{
    token_begin = p;
}
	goto st28;
st28:
	p += 1;
case 28:
#line 418 "parser_ragel.cpp"
	switch( (*p) ) {
		case 13: goto tr42;
		case 32: goto tr38;
	}
	if ( 9 <= (*p) && (*p) <= 12 )
		goto tr38;
	goto tr37;
tr39:
#line 35 "parser.rl"
	{
    token_begin = p;
}
	goto st29;
tr42:
#line 35 "parser.rl"
	{
    token_begin = p;
}
#line 39 "parser.rl"
	{
    token_end = p;
    request.headers.back().second = {token_begin, token_end};
    token_begin = token_end = nullptr;
}
	goto st29;
st29:
	p += 1;
case 29:
#line 447 "parser_ragel.cpp"
	switch( (*p) ) {
		case 10: goto tr43;
		case 13: goto tr42;
		case 32: goto tr38;
	}
	if ( 9 <= (*p) && (*p) <= 12 )
		goto tr38;
	goto tr37;
tr43:
#line 35 "parser.rl"
	{
    token_begin = p;
}
	goto st30;
st30:
	p += 1;
case 30:
#line 465 "parser_ragel.cpp"
	switch( (*p) ) {
		case 13: goto tr44;
		case 32: goto tr38;
		case 45: goto tr45;
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 12 )
			goto tr38;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr45;
		} else if ( (*p) >= 65 )
			goto tr45;
	} else
		goto tr45;
	goto tr37;
tr44:
#line 35 "parser.rl"
	{
    token_begin = p;
}
#line 39 "parser.rl"
	{
    token_end = p;
    request.headers.back().second = {token_begin, token_end};
    token_begin = token_end = nullptr;
}
	goto st31;
st31:
	p += 1;
case 31:
#line 498 "parser_ragel.cpp"
	switch( (*p) ) {
		case 10: goto tr46;
		case 13: goto tr42;
		case 32: goto tr38;
	}
	if ( 9 <= (*p) && (*p) <= 12 )
		goto tr38;
	goto tr37;
tr46:
#line 35 "parser.rl"
	{
    token_begin = p;
}
	goto st62;
st62:
#line 116 "parser.rl"
	{
                    {p++; cs = 62; goto _out;}
                }
	p += 1;
case 62:
#line 520 "parser_ragel.cpp"
	switch( (*p) ) {
		case 13: goto tr44;
		case 32: goto tr38;
		case 45: goto tr45;
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 12 )
			goto tr38;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr45;
		} else if ( (*p) >= 65 )
			goto tr45;
	} else
		goto tr45;
	goto tr37;
tr45:
#line 35 "parser.rl"
	{
    token_begin = p;
}
#line 25 "parser.rl"
	{
    token_begin = p;
}
	goto st32;
st32:
	p += 1;
case 32:
#line 551 "parser_ragel.cpp"
	switch( (*p) ) {
		case 13: goto tr48;
		case 32: goto tr47;
		case 45: goto st32;
		case 58: goto tr50;
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 12 )
			goto tr47;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto st32;
		} else if ( (*p) >= 65 )
			goto st32;
	} else
		goto st32;
	goto st26;
tr47:
#line 29 "parser.rl"
	{
    token_end = p;
    request.headers.emplace_back(std::make_pair<std::string>({token_begin, token_end}, ""));
    token_begin = token_end = nullptr;
}
	goto st33;
st33:
	p += 1;
case 33:
#line 581 "parser_ragel.cpp"
	switch( (*p) ) {
		case 13: goto tr52;
		case 32: goto st33;
		case 58: goto st28;
	}
	if ( 9 <= (*p) && (*p) <= 12 )
		goto st33;
	goto st26;
tr52:
#line 39 "parser.rl"
	{
    token_end = p;
    request.headers.back().second = {token_begin, token_end};
    token_begin = token_end = nullptr;
}
	goto st34;
tr48:
#line 29 "parser.rl"
	{
    token_end = p;
    request.headers.emplace_back(std::make_pair<std::string>({token_begin, token_end}, ""));
    token_begin = token_end = nullptr;
}
#line 39 "parser.rl"
	{
    token_end = p;
    request.headers.back().second = {token_begin, token_end};
    token_begin = token_end = nullptr;
}
	goto st34;
st34:
	p += 1;
case 34:
#line 615 "parser_ragel.cpp"
	switch( (*p) ) {
		case 10: goto st35;
		case 13: goto tr52;
		case 32: goto st33;
		case 58: goto st28;
	}
	if ( 9 <= (*p) && (*p) <= 12 )
		goto st33;
	goto st26;
st35:
	p += 1;
case 35:
	switch( (*p) ) {
		case 13: goto st36;
		case 32: goto st24;
		case 45: goto tr30;
		case 58: goto st25;
	}
	if ( (*p) < 48 ) {
		if ( 9 <= (*p) && (*p) <= 12 )
			goto st24;
	} else if ( (*p) > 57 ) {
		if ( (*p) > 90 ) {
			if ( 97 <= (*p) && (*p) <= 122 )
				goto tr30;
		} else if ( (*p) >= 65 )
			goto tr30;
	} else
		goto tr30;
	goto st0;
st36:
	p += 1;
case 36:
	switch( (*p) ) {
		case 10: goto st63;
		case 32: goto st24;
		case 58: goto st25;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st24;
	goto st0;
st63:
#line 116 "parser.rl"
	{
                    {p++; cs = 63; goto _out;}
                }
	p += 1;
case 63:
#line 664 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: goto st24;
		case 58: goto st25;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto st24;
	goto st0;
tr18:
#line 19 "parser.rl"
	{
    token_end = p;
    request.uri.assign(token_begin, token_end);
    token_begin = token_end = nullptr;
}
	goto st37;
tr59:
#line 45 "parser.rl"
	{
    token_begin = p;
}
#line 49 "parser.rl"
	{
    token_end = p;
    request.query_params.emplace_back(std::make_pair<std::string>({UTILS::decode({token_begin, token_end})}, ""));
    token_begin = token_end = nullptr;
}
	goto st37;
tr63:
#line 49 "parser.rl"
	{
    token_end = p;
    request.query_params.emplace_back(std::make_pair<std::string>({UTILS::decode({token_begin, token_end})}, ""));
    token_begin = token_end = nullptr;
}
	goto st37;
tr67:
#line 55 "parser.rl"
	{
    token_begin = p;
}
#line 59 "parser.rl"
	{
    token_end = p;
    request.query_params.back().second = UTILS::decode({token_begin, token_end});
    token_begin = token_end = nullptr;
}
	goto st37;
tr70:
#line 59 "parser.rl"
	{
    token_end = p;
    request.query_params.back().second = UTILS::decode({token_begin, token_end});
    token_begin = token_end = nullptr;
}
	goto st37;
st37:
	p += 1;
case 37:
#line 723 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: goto tr58;
		case 38: goto tr59;
		case 61: goto tr60;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto tr58;
	goto tr57;
tr57:
#line 45 "parser.rl"
	{
    token_begin = p;
}
	goto st38;
st38:
	p += 1;
case 38:
#line 741 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: goto tr62;
		case 38: goto tr63;
		case 61: goto tr64;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto tr62;
	goto st38;
tr60:
#line 45 "parser.rl"
	{
    token_begin = p;
}
#line 49 "parser.rl"
	{
    token_end = p;
    request.query_params.emplace_back(std::make_pair<std::string>({UTILS::decode({token_begin, token_end})}, ""));
    token_begin = token_end = nullptr;
}
	goto st39;
tr64:
#line 49 "parser.rl"
	{
    token_end = p;
    request.query_params.emplace_back(std::make_pair<std::string>({UTILS::decode({token_begin, token_end})}, ""));
    token_begin = token_end = nullptr;
}
	goto st39;
st39:
	p += 1;
case 39:
#line 773 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: goto tr66;
		case 38: goto tr67;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto tr66;
	goto tr65;
tr65:
#line 55 "parser.rl"
	{
    token_begin = p;
}
	goto st40;
st40:
	p += 1;
case 40:
#line 790 "parser_ragel.cpp"
	switch( (*p) ) {
		case 32: goto tr69;
		case 38: goto tr70;
	}
	if ( 9 <= (*p) && (*p) <= 13 )
		goto tr69;
	goto st40;
tr2:
#line 65 "parser.rl"
	{
    token_begin = p;
}
	goto st41;
st41:
	p += 1;
case 41:
#line 807 "parser_ragel.cpp"
	if ( (*p) == 69 )
		goto st42;
	goto st0;
st42:
	p += 1;
case 42:
	if ( (*p) == 76 )
		goto st43;
	goto st0;
st43:
	p += 1;
case 43:
	if ( (*p) == 69 )
		goto st44;
	goto st0;
st44:
	p += 1;
case 44:
	if ( (*p) == 84 )
		goto st45;
	goto st0;
st45:
	p += 1;
case 45:
	if ( (*p) == 69 )
		goto tr13;
	goto st0;
tr3:
#line 65 "parser.rl"
	{
    token_begin = p;
}
	goto st46;
st46:
	p += 1;
case 46:
#line 844 "parser_ragel.cpp"
	if ( (*p) == 69 )
		goto st7;
	goto st0;
tr4:
#line 65 "parser.rl"
	{
    token_begin = p;
}
	goto st47;
st47:
	p += 1;
case 47:
#line 857 "parser_ragel.cpp"
	if ( (*p) == 69 )
		goto st48;
	goto st0;
st48:
	p += 1;
case 48:
	if ( (*p) == 65 )
		goto st49;
	goto st0;
st49:
	p += 1;
case 49:
	if ( (*p) == 68 )
		goto tr13;
	goto st0;
tr5:
#line 65 "parser.rl"
	{
    token_begin = p;
}
	goto st50;
st50:
	p += 1;
case 50:
#line 882 "parser_ragel.cpp"
	if ( (*p) == 80 )
		goto st51;
	goto st0;
st51:
	p += 1;
case 51:
	if ( (*p) == 84 )
		goto st52;
	goto st0;
st52:
	p += 1;
case 52:
	if ( (*p) == 73 )
		goto st53;
	goto st0;
st53:
	p += 1;
case 53:
	if ( (*p) == 79 )
		goto st54;
	goto st0;
st54:
	p += 1;
case 54:
	if ( (*p) == 78 )
		goto st55;
	goto st0;
st55:
	p += 1;
case 55:
	if ( (*p) == 83 )
		goto tr13;
	goto st0;
tr6:
#line 65 "parser.rl"
	{
    token_begin = p;
}
	goto st56;
st56:
	p += 1;
case 56:
#line 925 "parser_ragel.cpp"
	switch( (*p) ) {
		case 79: goto st57;
		case 85: goto st7;
	}
	goto st0;
st57:
	p += 1;
case 57:
	if ( (*p) == 83 )
		goto st7;
	goto st0;
tr7:
#line 65 "parser.rl"
	{
    token_begin = p;
}
	goto st58;
st58:
	p += 1;
case 58:
#line 946 "parser_ragel.cpp"
	if ( (*p) == 82 )
		goto st59;
	goto st0;
st59:
	p += 1;
case 59:
	if ( (*p) == 65 )
		goto st60;
	goto st0;
st60:
	p += 1;
case 60:
	if ( (*p) == 67 )
		goto st45;
	goto st0;
	}

	_out: {}
	}

#line 122 "parser.rl"


    if (cs < http_first_final)
    {
        BOOST_LOG_TRIVIAL(error)
            << "Invalid request read, cannot parse: " << std::string(p, end);
        return false;
    }

    consumed = p - start;
    return true;
}

} }

