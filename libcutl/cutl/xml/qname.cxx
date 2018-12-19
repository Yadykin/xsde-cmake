// file      : cutl/xml/qname.cxx
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <ostream>

#include <cutl/xml/qname.hxx>

using namespace std;

namespace cutl
{
  namespace xml
  {
    string qname::
    string () const
    {
      std::string r;
      if (!ns_.empty ())
      {
        r += ns_;
        r += '#';
      }

      r += name_;
      return r;
    }

    ostream&
    operator<< (ostream& os, const qname& qn)
    {
      return os << qn.string ();
    }
  }
}
