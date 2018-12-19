// file      : cutl/re/re.cxx
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <cutl/re.hxx>

#include <cutl/details/config.hxx> // LIBCUTL_*

// For build2 build it is either C++11 regex or external Boost.
//
// Note that some compiler/runtime combinations don't have usable C++11
// regex. For example Clang 3.5 with libstdc++ from GCC 4.9. In this case
// you can fall back to using Boost regex by passing -DLIBCUTL_BOOST_REGEX
// preprocessor option when building libcutl.
//
// @@ Should this rather be a (custom) config.* variable?
//
#ifdef LIBCUTL_BUILD2
#  if defined(LIBCUTL_CXX11) && !defined(LIBCUTL_BOOST_REGEX)
#    include <regex>
#    include <locale>
#    include <cstddef> // size_t
#  else
#    ifndef LIBCUTL_BOOST_REGEX
#      define LIBCUTL_BOOST_REGEX
#    endif
#    include <boost/tr1/regex.hpp>
#  endif
#else
#  ifndef LIBCUTL_BOOST_REGEX
#    define LIBCUTL_BOOST_REGEX
#  endif
#  ifndef LIBCUTL_EXTERNAL_BOOST
#    include <cutl/details/boost/tr1/regex.hpp>
#  else
#    include <boost/tr1/regex.hpp>
#  endif
#endif

using namespace std;

namespace cutl
{
  namespace re
  {
#ifdef LIBCUTL_BOOST_REGEX
    namespace ire = std::tr1;
#else
    namespace ire = std;
#endif

    //
    // format_base
    //

    char const* format_base::
    what () const LIBCUTL_NOTHROW_NOEXCEPT
    {
      return description_.c_str ();
    }

    //
    // basic_regex
    //
    template <typename C>
    struct basic_regex<C>::impl
    {
      typedef basic_string<C> string_type;
      typedef ire::basic_regex<C> regex_type;
      typedef typename regex_type::flag_type flag_type;

      impl () {}
      impl (regex_type const& r): r (r) {}
      impl (string_type const& s, bool icase)
      {
        flag_type f (ire::regex_constants::ECMAScript);

        if (icase)
          f |= ire::regex_constants::icase;

        r.assign (s, f);
      }

      regex_type r;
    };

    template <>
    LIBCUTL_EXPORT basic_regex<char>::
    ~basic_regex ()
    {
      delete impl_;
    }

    template <>
    LIBCUTL_EXPORT basic_regex<wchar_t>::
    ~basic_regex ()
    {
      delete impl_;
    }

    template <>
    LIBCUTL_EXPORT basic_regex<char>::
    basic_regex (basic_regex const& r)
        : str_ (r.str_), impl_ (new impl (r.impl_->r))
    {
    }

    template <>
    LIBCUTL_EXPORT basic_regex<wchar_t>::
    basic_regex (basic_regex const& r)
        : str_ (r.str_), impl_ (new impl (r.impl_->r))
    {
    }

    template <>
    LIBCUTL_EXPORT basic_regex<char>& basic_regex<char>::
    operator= (basic_regex const& r)
    {
      string_type tmp (r.str_);
      impl_->r = r.impl_->r;
      str_.swap (tmp);
      return *this;
    }

    template <>
    LIBCUTL_EXPORT basic_regex<wchar_t>& basic_regex<wchar_t>::
    operator= (basic_regex const& r)
    {
      string_type tmp (r.str_);
      impl_->r = r.impl_->r;
      str_.swap (tmp);
      return *this;
    }

    template <>
    LIBCUTL_EXPORT void basic_regex<char>::
    init (string_type const* s, bool icase)
    {
      string_type tmp (s == 0 ? string_type () : *s);

      try
      {
        if (impl_ == 0)
          impl_ = s == 0 ? new impl : new impl (*s, icase);
        else
        {
          impl::flag_type f (ire::regex_constants::ECMAScript);

          if (icase)
            f |= ire::regex_constants::icase;

          impl_->r.assign (*s, f);
        }
      }
      catch (ire::regex_error const& e)
      {
        throw basic_format<char> (s == 0 ? "" : *s, e.what ());
      }

      str_.swap (tmp);
    }

    template <>
    LIBCUTL_EXPORT void basic_regex<wchar_t>::
    init (string_type const* s, bool icase)
    {
      string_type tmp (s == 0 ? string_type () : *s);

      try
      {
        if (impl_ == 0)
          impl_ = s == 0 ? new impl : new impl (*s, icase);
        else
        {
          impl::flag_type f (ire::regex_constants::ECMAScript);

          if (icase)
            f |= ire::regex_constants::icase;

          impl_->r.assign (*s, f);
        }
      }
      catch (ire::regex_error const& e)
      {
        throw basic_format<wchar_t> (s == 0 ? L"" : *s, e.what ());
      }

      str_.swap (tmp);
    }

    template <>
    LIBCUTL_EXPORT bool basic_regex<char>::
    match (string_type const& s) const
    {
      return ire::regex_match (s, impl_->r);
    }

    template <>
    LIBCUTL_EXPORT bool basic_regex<wchar_t>::
    match (string_type const& s) const
    {
      return ire::regex_match (s, impl_->r);
    }

    template <>
    LIBCUTL_EXPORT bool basic_regex<char>::
    search (string_type const& s) const
    {
      return ire::regex_search (s, impl_->r);
    }

    template <>
    LIBCUTL_EXPORT bool basic_regex<wchar_t>::
    search (string_type const& s) const
    {
      return ire::regex_search (s, impl_->r);
    }

    // If we are using C++11 regex then extend the standard ECMA-262
    // substitution escape sequences with a subset of Perl sequences:
    //
    // \\, \u, \l, \U, \L, \E, \1, ..., \9
    //
    // Notes and limitations:
    //
    // - The only valid regex_constants flags are match_default,
    //   format_first_only (format_no_copy can easily be supported).
    //
    // - If backslash doesn't start any of the listed sequences then it is
    //   silently dropped and the following character is copied as is.
    //
    // - The character case conversion is performed according to the global
    //   C++ locale (which is, unless changed, is the same as C locale and
    //   both default to the POSIX locale aka "C").
    //
    template <typename C>
    static basic_string<C>
    regex_replace_ex (const basic_string<C>& s,
                      const ire::basic_regex<C>& re,
                      const basic_string<C>& fmt,
                      ire::regex_constants::match_flag_type flags)
    {
#ifdef LIBCUTL_BOOST_REGEX
      // Boost regex already does what we need.
      //
      return ire::regex_replace (s, re, fmt, flags);
#else
      using string_type = basic_string<C>;
      using str_it      = typename string_type::const_iterator;
      using regex_it    = regex_iterator<str_it>;

      bool first_only ((flags & regex_constants::format_first_only) ==
                       regex_constants::format_first_only);

      locale cl; // Copy of the global C++ locale.
      string_type r;

      // Beginning of the last unmatched substring.
      //
      str_it ub (s.begin ());

      for (regex_it b (s.begin (), s.end (), re, flags), i (b), e; i != e; ++i)
      {
        const match_results<str_it>& m (*i);

        // Copy the preceeding unmatched substring, save the beginning of the
        // one that follows.
        //
        r.append (ub, m.prefix ().second);
        ub = m.suffix ().first;

        if (first_only && i != b)
          r.append (m[0].first, m[0].second); // Append matched substring.
        else
        {
          // The standard implementation calls m.format() here. We perform our
          // own formatting.
          //
          // Note that we are using char type literals with the assumption
          // that being ASCII characters they will be properly "widened" to
          // the corresponding literals of the C template parameter type.
          //
          auto digit = [] (C c) -> int
          {
            return c >= '0' && c <= '9' ? c - '0' : -1;
          };

          enum class case_conv {none, upper, lower, upper_once, lower_once}
          mode (case_conv::none);

          auto conv_chr = [&mode, &cl] (C c) -> C
          {
            switch (mode)
            {
            case case_conv::upper_once: mode = case_conv::none; // Fall through.
            case case_conv::upper:      c = toupper (c, cl); break;
            case case_conv::lower_once: mode = case_conv::none; // Fall through.
            case case_conv::lower:      c = tolower (c, cl); break;
            case case_conv::none:       break;
            }
            return c;
          };

          auto append_chr = [&r, &conv_chr] (C c)
          {
            r.push_back (conv_chr (c));
          };

          auto append_str = [&r, &mode, &conv_chr] (str_it b, str_it e)
          {
            // Optimize for the common case.
            //
            if (mode == case_conv::none)
              r.append (b, e);
            else
            {
              for (str_it i (b); i != e; ++i)
                r.push_back (conv_chr (*i));
            }
          };

          size_t n (fmt.size ());
          for (size_t i (0); i < n; ++i)
          {
            C c (fmt[i]);

            switch (c)
            {
            case '$':
              {
                // Check if this is a $-based escape sequence. Interpret it
                // accordingly if that's the case, treat '$' as a regular
                // character otherwise.
                //
                c = fmt[++i]; // '\0' if last.

                switch (c)
                {
                case '$': append_chr (c); break;
                case '&': append_str (m[0].first, m[0].second); break;
                case '`':
                  {
                    append_str (m.prefix ().first, m.prefix ().second);
                    break;
                  }
                case '\'':
                  {
                    append_str (m.suffix ().first, m.suffix ().second);
                    break;
                  }
                default:
                  {
                    // Check if this is a sub-expression 1-based index ($n or
                    // $nn). Append the matching substring if that's the case.
                    // Treat '$' as a regular character otherwise. Index
                    // greater than the sub-expression count is silently
                    // ignored.
                    //
                    int si (digit (c));
                    if (si >= 0)
                    {
                      int d;
                      if ((d = digit (fmt[i + 1])) >= 0) // '\0' if last.
                      {
                        si = si * 10 + d;
                        ++i;
                      }
                    }

                    if (si > 0)
                    {
                      // m[0] refers to the matched substring.
                      //
                      if (static_cast<size_t> (si) < m.size ())
                        append_str (m[si].first, m[si].second);
                    }
                    else
                    {
                      // Not a $-based escape sequence so treat '$' as a
                      // regular character.
                      //
                      --i;
                      append_chr ('$');
                    }

                    break;
                  }
                }

                break;
              }
            case '\\':
              {
                c = fmt[++i]; // '\0' if last.

                switch (c)
                {
                case '\\': append_chr (c); break;

                case 'u': mode = case_conv::upper_once; break;
                case 'l': mode = case_conv::lower_once; break;
                case 'U': mode = case_conv::upper;      break;
                case 'L': mode = case_conv::lower;      break;
                case 'E': mode = case_conv::none;       break;
                default:
                  {
                    // Check if this is a sub-expression 1-based index. Append
                    // the matching substring if that's the case, Skip '\\'
                    // otherwise. Index greater than the sub-expression count
                    // is silently ignored.
                    //
                    int si (digit (c));
                    if (si > 0)
                    {
                      // m[0] refers to the matched substring.
                      //
                      if (static_cast<size_t> (si) < m.size ())
                        append_str (m[si].first, m[si].second);
                    }
                    else
                      --i;

                    break;
                  }
                }

                break;
              }
            default:
              {
                // Append a regular character.
                //
                append_chr (c);
                break;
              }
            }
          }
        }
      }

      r.append (ub, s.end ()); // Append the rightmost non-matched substring.
      return r;
#endif
    }

    template <>
    LIBCUTL_EXPORT string basic_regex<char>::
    replace (string_type const& s,
             string_type const& sub,
             bool first_only) const
    {
      ire::regex_constants::match_flag_type f (
        ire::regex_constants::format_default);

      if (first_only)
        f |= ire::regex_constants::format_first_only;

      return regex_replace_ex (s, impl_->r, sub, f);
    }

    template <>
    LIBCUTL_EXPORT wstring basic_regex<wchar_t>::
    replace (string_type const& s,
             string_type const& sub,
             bool first_only) const
    {
      ire::regex_constants::match_flag_type f (
        ire::regex_constants::format_default);

      if (first_only)
        f |= ire::regex_constants::format_first_only;

      return regex_replace_ex (s, impl_->r, sub, f);
    }
  }
}
