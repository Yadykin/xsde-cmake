// file      : xsd-frontend/traversal/enumeration.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_TRAVERSAL_ENUMERATION_HXX
#define XSD_FRONTEND_TRAVERSAL_ENUMERATION_HXX

#include <xsd-frontend/traversal/elements.hxx>
#include <xsd-frontend/semantic-graph/enumeration.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    struct Enumeration : ScopeTemplate<SemanticGraph::Enumeration>
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
      post (Type&);
    };

    struct Enumerator : Node<SemanticGraph::Enumerator>
    {
      virtual void
      traverse (Type&);

      virtual void
      pre (Type&);

      virtual void
      belongs (Type&, EdgeDispatcher&);

      virtual void
      belongs (Type&);

      virtual void
      name (Type&);

      virtual void
      post (Type&);
    };
  }
}

#endif  // XSD_FRONTEND_TRAVERSAL_ENUMERATION_HXX
