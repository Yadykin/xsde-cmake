// file      : xsd-frontend/traversal/element-group.hxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_TRAVERSAL_ELEMENT_GROUP_HXX
#define XSD_FRONTEND_TRAVERSAL_ELEMENT_GROUP_HXX

#include <xsd-frontend/traversal/elements.hxx>
#include <xsd-frontend/semantic-graph/element-group.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    struct ElementGroup: ScopeTemplate<SemanticGraph::ElementGroup>
    {
      virtual void
      traverse (Type&);

      virtual void
      pre (Type&);

      virtual void
      contains_compositor (Type&);

      virtual void
      contains_compositor (Type&, EdgeDispatcher&);

      virtual void
      post (Type&);
    };
  }
}

#endif  // XSD_FRONTEND_TRAVERSAL_ELEMENT_GROUP_HXX
