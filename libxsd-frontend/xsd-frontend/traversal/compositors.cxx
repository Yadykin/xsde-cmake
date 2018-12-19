// file      : xsd-frontend/traversal/compositors.cxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsd-frontend/traversal/compositors.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    // ContainsParticle
    //
    void ContainsParticle::
    traverse (Type& c)
    {
      dispatch (c.particle ());
    }


    // ContainsCompositor
    //
    void ContainsCompositor::
    traverse (Type& c)
    {
      dispatch (c.compositor ());
    }


    // Compositor
    //
    void Compositor::
    traverse (Type& c)
    {
      pre (c);
      contains (c);
      post (c);
    }

    void Compositor::
    pre (Type&)
    {
    }

    void Compositor::
    contains (Type& c)
    {
      iterate_and_dispatch (
        c.contains_begin (), c.contains_end (), edge_traverser ());
    }

    void Compositor::
    contains (Type& c, EdgeDispatcher& d)
    {
      iterate_and_dispatch (c.contains_begin (), c.contains_end (), d);
    }

    void Compositor::
    post (Type&)
    {
    }


    // All
    //
    void All::
    traverse (Type& c)
    {
      pre (c);
      contains (c);
      post (c);
    }

    void All::
    pre (Type&)
    {
    }

    void All::
    contains (Type& c)
    {
      iterate_and_dispatch (
        c.contains_begin (), c.contains_end (), edge_traverser ());
    }

    void All::
    contains (Type& c, EdgeDispatcher& d)
    {
      iterate_and_dispatch (c.contains_begin (), c.contains_end (), d);
    }

    void All::
    post (Type&)
    {
    }


    // Choice
    //
    void Choice::
    traverse (Type& c)
    {
      pre (c);
      contains (c);
      post (c);
    }

    void Choice::
    pre (Type&)
    {
    }

    void Choice::
    contains (Type& c)
    {
      iterate_and_dispatch (
        c.contains_begin (), c.contains_end (), edge_traverser ());
    }

    void Choice::
    contains (Type& c, EdgeDispatcher& d)
    {
      iterate_and_dispatch (c.contains_begin (), c.contains_end (), d);
    }

    void Choice::
    post (Type&)
    {
    }


    // Sequence
    //
    void Sequence::
    traverse (Type& c)
    {
      pre (c);
      contains (c);
      post (c);
    }

    void Sequence::
    pre (Type&)
    {
    }

    void Sequence::
    contains (Type& c)
    {
      iterate_and_dispatch (
        c.contains_begin (), c.contains_end (), edge_traverser ());
    }

    void Sequence::
    contains (Type& c, EdgeDispatcher& d)
    {
      iterate_and_dispatch (c.contains_begin (), c.contains_end (), d);
    }

    void Sequence::
    post (Type&)
    {
    }
  }
}
