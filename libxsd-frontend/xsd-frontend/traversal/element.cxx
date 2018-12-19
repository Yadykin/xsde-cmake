// file      : xsd-frontend/traversal/element.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsd-frontend/traversal/element.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    void Element::
    traverse (Type& m)
    {
      pre (m);
      belongs (m);
      name (m);
      post (m);
    }

    void Element::
    pre (Type&)
    {
    }

    void Element::
    belongs (Type& m, EdgeDispatcher& d)
    {
      d.dispatch (m.belongs ());
    }

    void Element::
    belongs (Type& m)
    {
      belongs (m, edge_traverser ());
    }

    void Element::
    name (Type&)
    {
    }

    void Element::
    post (Type&)
    {
    }
  }
}
