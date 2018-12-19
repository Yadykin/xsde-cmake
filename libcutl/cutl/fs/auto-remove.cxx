// file      : cutl/fs/auto-remove.cxx
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <cstdio> // std::remove
#include <cerrno>

#include <cutl/fs/auto-remove.hxx>

namespace cutl
{
  namespace fs
  {
    auto_remove::
    ~auto_remove ()
    {
      if (!canceled_)
        std::remove (path_.string ().c_str ()); // Ignore error.
    }

    auto_removes::
    ~auto_removes ()
    {
      if (!canceled_)
      {
        for (paths::iterator i (paths_.begin ()); i != paths_.end (); ++i)
          std::remove (i->string ().c_str ()); // Ignore error.
      }
    }
  }
}
