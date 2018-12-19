// file      : xsd-frontend/traversal/list.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsd-frontend/traversal/list.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    void List::
    traverse (Type& l)
    {
      pre (l);
      argumented (l);
      name (l);
      post (l);
    }

    void List::
    pre (Type&)
    {
    }

    void List::
    argumented (Type& l)
    {
      argumented (l, *this);
    }

    void List::
    argumented (Type& l, EdgeDispatcher& d)
    {
      d.dispatch (l.argumented ());
    }

    void List::
    name (Type&)
    {
    }

    void List::
    post (Type&)
    {
    }
  }
}
