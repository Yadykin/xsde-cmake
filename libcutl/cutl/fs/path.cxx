// file      : cutl/fs/path.cxx
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#ifdef _WIN32
#  include <stdlib.h> // _MAX_PATH
#  include <direct.h> // _[w]getcwd, _[w]chdir
#else
#  include <stdlib.h> // mbstowcs, wcstombs
#  include <limits.h> // PATH_MAX
#  include <unistd.h> // getcwd, chdir
#endif

#ifndef _WIN32
#  ifndef PATH_MAX
#    define PATH_MAX 4096
#  endif
#endif

#include <cutl/fs/path.hxx>

namespace cutl
{
  namespace fs
  {
    char const* invalid_path_base::
    what () const LIBCUTL_NOTHROW_NOEXCEPT
    {
      return "invalid filesystem path";
    }

    //
    // char
    //

    template <>
    LIBCUTL_EXPORT basic_path<char> basic_path<char>::
    current ()
    {
#ifdef _WIN32
      char cwd[_MAX_PATH];
      if(_getcwd(cwd, _MAX_PATH) == 0)
        throw invalid_basic_path<char> (".");
#else
      char cwd[PATH_MAX];
      if (getcwd (cwd, PATH_MAX) == 0)
        throw invalid_basic_path<char> (".");
#endif

      return basic_path<char> (cwd);
    }

    template <>
    LIBCUTL_EXPORT void basic_path<char>::
    current (basic_path const& p)
    {
      string_type const& s (p.string ());

      if (p.empty ())
        throw invalid_basic_path<char> (s);

#ifdef _WIN32
      if(_chdir(s.c_str ()) != 0)
        throw invalid_basic_path<char> (s);
#else
      if (chdir (s.c_str ()) != 0)
        throw invalid_basic_path<char> (s);
#endif
    }

    //
    // wchar_t
    //

    template <>
    LIBCUTL_EXPORT basic_path<wchar_t> basic_path<wchar_t>::
    current ()
    {
#ifdef _WIN32
      wchar_t wcwd[_MAX_PATH];
      if(_wgetcwd(wcwd, _MAX_PATH) == 0)
        throw invalid_basic_path<wchar_t> (L".");
#else
      char cwd[PATH_MAX];
      if (getcwd (cwd, PATH_MAX) == 0)
        throw invalid_basic_path<wchar_t> (L".");

      wchar_t wcwd[PATH_MAX];
      if (mbstowcs (wcwd, cwd, PATH_MAX) == size_type (-1))
        throw invalid_basic_path<wchar_t> (L".");
#endif

      return basic_path<wchar_t> (wcwd);
    }

    template <>
    LIBCUTL_EXPORT void basic_path<wchar_t>::
    current (basic_path const& p)
    {
      string_type const& s (p.string ());

      if (p.empty ())
        throw invalid_basic_path<wchar_t> (s);

#ifdef _WIN32
      if(_wchdir(s.c_str ()) != 0)
        throw invalid_basic_path<wchar_t> (s);
#else
      char ns[PATH_MAX + 1];

      if (wcstombs (ns, s.c_str (), PATH_MAX) == size_type (-1))
        throw invalid_basic_path<wchar_t> (s);

      ns[PATH_MAX] = '\0';

      if (chdir (ns) != 0)
        throw invalid_basic_path<wchar_t> (s);
#endif
    }
  }
}
