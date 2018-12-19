// file      : xsd-frontend/traversal/attribute.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsd-frontend/traversal/attribute.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    void Attribute::
    traverse (Type& a)
    {
      pre (a);
      belongs (a);
      name (a);
      post (a);
    }

    void Attribute::
    pre (Type&)
    {
    }

    void Attribute::
    belongs (Type& a, EdgeDispatcher& d)
    {
      d.dispatch (a.belongs ());
    }

    void Attribute::
    belongs (Type& a)
    {
      belongs (a, *this);
    }

    void Attribute::
    name (Type&)
    {
    }

    void Attribute::
    post (Type&)
    {
    }
  }
}
