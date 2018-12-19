// file      : cutl/exception.cxx
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <typeinfo>

#include <cutl/exception.hxx>

namespace cutl
{
  char const* exception::
  what () const LIBCUTL_NOTHROW_NOEXCEPT
  {
    return typeid (*this).name ();
  }
}
