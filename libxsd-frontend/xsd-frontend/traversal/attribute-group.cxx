// file      : xsd-frontend/traversal/attribute-group.cxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsd-frontend/traversal/attribute-group.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    void AttributeGroup::
    traverse (Type& g)
    {
      pre (g);
      names (g);
      post (g);
    }

    void AttributeGroup::
    pre (Type&)
    {
    }

    void AttributeGroup::
    post (Type&)
    {
    }
  }
}
