// file      : cutl/compiler/context.cxx
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <cutl/compiler/context.hxx>

using namespace std;

namespace cutl
{
  namespace compiler
  {
    void context::
    set (string const& key, container::any const& value)
    {
      using container::any;

      std::pair<map::iterator, bool> r (
        map_.insert (map::value_type (key, value)));

      any& x (r.first->second);

      if (!r.second)
      {
        if (value.type_info () != x.type_info ())
          throw typing ();

        x = value;
      }
    }

    void context::
    remove (string const& key)
    {
      map::iterator i (map_.find (key));

      if (i == map_.end ())
        throw no_entry ();

      map_.erase (i);
    }

    type_info const& context::
    type_info (string const& key) const
    {
      map::const_iterator i (map_.find (key));

      if (i == map_.end ())
        throw no_entry ();

      return i->second.type_info ();
    }
  }
}
