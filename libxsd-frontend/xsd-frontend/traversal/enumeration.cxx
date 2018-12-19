// file      : xsd-frontend/traversal/enumeration.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsd-frontend/traversal/enumeration.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    // Enumeration
    //
    void Enumeration::
    traverse (Type& e)
    {
      pre (e);
      name (e);
      inherits (e);
      names (e);
      post (e);
    }

    void Enumeration::
    pre (Type&)
    {
    }

    void Enumeration::
    name (Type&)
    {
    }

    void Enumeration::
    inherits (Type& e)
    {
      inherits (e, *this);
    }

    void Enumeration::
    inherits (Type& e, EdgeDispatcher& d)
    {
      if (e.inherits_p ())
        d.dispatch (e.inherits ());
    }

    void Enumeration::
    post (Type&)
    {
    }


    // Enumerator
    //
    void Enumerator::
    traverse (Type& e)
    {
      pre (e);
      belongs (e);
      name (e);
      post (e);
    }

    void Enumerator::
    pre (Type&)
    {
    }

    void Enumerator::
    belongs (Type& e, EdgeDispatcher& d)
    {
      d.dispatch (e.belongs ());
    }

    void Enumerator::
    belongs (Type& e)
    {
      belongs (e, edge_traverser ());
    }

    void Enumerator::
    name (Type&)
    {
    }

    void Enumerator::
    post (Type&)
    {
    }
  }
}
