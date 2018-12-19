// file      : cutl/compiler/cxx-indenter.cxx
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <cutl/compiler/cxx-indenter.hxx>

namespace cutl
{
  namespace compiler
  {
    template<>
    LIBCUTL_EXPORT char const* cxx_indenter<char>::
    keyword (cxx_indenter<char>::keyword_type t)
    {
      static char const* keywords[] =
      {
        "if",
        "do",
        "for",
        "else",
        "case",
        "while",
        "catch",
        "default"
      };

      return keywords[t];
    }

    template<>
    LIBCUTL_EXPORT wchar_t const* cxx_indenter<wchar_t>::
    keyword (cxx_indenter<wchar_t>::keyword_type t)
    {
      static wchar_t const* keywords[] =
      {
        L"if",
        L"do",
        L"for",
        L"else",
        L"case",
        L"while",
        L"catch",
        L"default"
      };

      return keywords[t];
    }
  }
}
