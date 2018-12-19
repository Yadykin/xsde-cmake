// file      : xsd-frontend/traversal/namespace.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_TRAVERSAL_NAMESPACE_HXX
#define XSD_FRONTEND_TRAVERSAL_NAMESPACE_HXX

#include <xsd-frontend/traversal/elements.hxx>
#include <xsd-frontend/semantic-graph/namespace.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    struct Namespace: ScopeTemplate<SemanticGraph::Namespace>
    {
      virtual void
      traverse (Type& m)
      {
        pre (m);
        name (m);
        names (m);
        post (m);
      }

      virtual void
      pre (Type&)
      {
      }

      virtual void
      name (Type&)
      {
      }

      virtual void
      post (Type&)
      {
      }
    };
  }
}

#endif  // XSD_FRONTEND_TRAVERSAL_NAMESPACE_HXX
