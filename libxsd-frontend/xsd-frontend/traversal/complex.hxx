// file      : xsd-frontend/traversal/complex.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_TRAVERSAL_COMPLEX_HXX
#define XSD_FRONTEND_TRAVERSAL_COMPLEX_HXX

#include <xsd-frontend/traversal/elements.hxx>
#include <xsd-frontend/semantic-graph/complex.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    struct Complex : ScopeTemplate<SemanticGraph::Complex>
    {
      virtual void
      traverse (Type&);

      virtual void
      pre (Type&);

      virtual void
      name (Type&);

      virtual void
      inherits (Type&);

      void
      inherits (Type&, EdgeDispatcher&);

      virtual void
      contains_compositor (Type&);

      void
      contains_compositor (Type&, EdgeDispatcher&);

      virtual void
      post (Type&);
    };
  }
}

#endif  // XSD_FRONTEND_TRAVERSAL_COMPLEX_HXX
