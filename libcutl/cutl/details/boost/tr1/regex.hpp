//  (C) Copyright John Maddock 2005.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TR1_REGEX_HPP_INCLUDED
#  define BOOST_TR1_REGEX_HPP_INCLUDED
#  include <cutl/details/boost/tr1/detail/config.hpp>

#ifdef BOOST_HAS_TR1_REGEX

#  if defined(BOOST_HAS_INCLUDE_NEXT) && !defined(BOOST_TR1_DISABLE_INCLUDE_NEXT)
#     include_next BOOST_TR1_HEADER(regex)
#  else
#     include <cutl/details/boost/tr1/detail/config_all.hpp>
#     include BOOST_TR1_STD_HEADER(BOOST_TR1_PATH(regex))
#  endif

#else

#include <cutl/details/boost/regex.hpp>

namespace std{ namespace tr1{

// [7.5] Regex constants
namespace regex_constants {

using ::cutl_details_boost::regex_constants::syntax_option_type;
using ::cutl_details_boost::regex_constants::icase;
using ::cutl_details_boost::regex_constants::nosubs;
using ::cutl_details_boost::regex_constants::optimize;
using ::cutl_details_boost::regex_constants::collate;
using ::cutl_details_boost::regex_constants::ECMAScript;
using ::cutl_details_boost::regex_constants::basic;
using ::cutl_details_boost::regex_constants::extended;
using ::cutl_details_boost::regex_constants::awk;
using ::cutl_details_boost::regex_constants::grep;
using ::cutl_details_boost::regex_constants::egrep;

using ::cutl_details_boost::regex_constants::match_flag_type;
using ::cutl_details_boost::regex_constants::match_default;
using ::cutl_details_boost::regex_constants::match_not_bol;
using ::cutl_details_boost::regex_constants::match_not_eol;
using ::cutl_details_boost::regex_constants::match_not_bow;
using ::cutl_details_boost::regex_constants::match_not_eow;
using ::cutl_details_boost::regex_constants::match_any;
using ::cutl_details_boost::regex_constants::match_not_null;
using ::cutl_details_boost::regex_constants::match_continuous;
using ::cutl_details_boost::regex_constants::match_prev_avail;
using ::cutl_details_boost::regex_constants::format_default;
using ::cutl_details_boost::regex_constants::format_sed;
using ::cutl_details_boost::regex_constants::format_no_copy;
using ::cutl_details_boost::regex_constants::format_first_only;

using ::cutl_details_boost::regex_constants::error_type;
using ::cutl_details_boost::regex_constants::error_collate;
using ::cutl_details_boost::regex_constants::error_ctype;
using ::cutl_details_boost::regex_constants::error_escape;
using ::cutl_details_boost::regex_constants::error_backref;
using ::cutl_details_boost::regex_constants::error_brack;
using ::cutl_details_boost::regex_constants::error_paren;
using ::cutl_details_boost::regex_constants::error_brace;
using ::cutl_details_boost::regex_constants::error_badbrace;
using ::cutl_details_boost::regex_constants::error_range;
using ::cutl_details_boost::regex_constants::error_space;
using ::cutl_details_boost::regex_constants::error_badrepeat;
using ::cutl_details_boost::regex_constants::error_complexity;
using ::cutl_details_boost::regex_constants::error_stack;

} // namespace regex_constants

// [7.6] Class regex_error
using ::cutl_details_boost::regex_error;

// [7.7] Class template regex_traits
using ::cutl_details_boost::regex_traits;

// [7.8] Class template basic_regex
using ::cutl_details_boost::basic_regex;
using ::cutl_details_boost::regex;
#ifndef BOOST_NO_WREGEX
using ::cutl_details_boost::wregex;
#endif

#if !BOOST_WORKAROUND(__BORLANDC__, < 0x0582)
// [7.8.6] basic_regex swap
using ::cutl_details_boost::swap;
#endif

// [7.9] Class template sub_match
using ::cutl_details_boost::sub_match;

using ::cutl_details_boost::csub_match;
#ifndef BOOST_NO_WREGEX
using ::cutl_details_boost::wcsub_match;
#endif
using ::cutl_details_boost::ssub_match;
#ifndef BOOST_NO_WREGEX
using ::cutl_details_boost::wssub_match;
#endif

// [7.10] Class template match_results
using ::cutl_details_boost::match_results;
using ::cutl_details_boost::cmatch;
#ifndef BOOST_NO_WREGEX
using ::cutl_details_boost::wcmatch;
#endif
using ::cutl_details_boost::smatch;
#ifndef BOOST_NO_WREGEX
using ::cutl_details_boost::wsmatch;
#endif

using ::cutl_details_boost::regex_match;

// [7.11.3] Function template regex_search
using ::cutl_details_boost::regex_search;

// [7.11.4] Function template regex_replace
using ::cutl_details_boost::regex_replace;

// [7.12.1] Class template regex_iterator
using ::cutl_details_boost::regex_iterator;
using ::cutl_details_boost::cregex_iterator;
#ifndef BOOST_NO_WREGEX
using ::cutl_details_boost::wcregex_iterator;
#endif
using ::cutl_details_boost::sregex_iterator;
#ifndef BOOST_NO_WREGEX
using ::cutl_details_boost::wsregex_iterator;
#endif

// [7.12.2] Class template regex_token_iterator
using ::cutl_details_boost::regex_token_iterator;
using ::cutl_details_boost::cregex_token_iterator;
#ifndef BOOST_NO_WREGEX
using ::cutl_details_boost::wcregex_token_iterator;
#endif
using ::cutl_details_boost::sregex_token_iterator;
#ifndef BOOST_NO_WREGEX
using ::cutl_details_boost::wsregex_token_iterator;
#endif

} } // namespaces

#endif

#endif
