// file      : xsd-frontend/traversal/attribute-group.hxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_TRAVERSAL_ATTRIBUTE_GROUP_HXX
#define XSD_FRONTEND_TRAVERSAL_ATTRIBUTE_GROUP_HXX

#include <xsd-frontend/traversal/elements.hxx>
#include <xsd-frontend/semantic-graph/attribute-group.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    struct AttributeGroup: ScopeTemplate<SemanticGraph::AttributeGroup>
    {
      virtual void
      traverse (Type&);

      virtual void
      pre (Type&);

      virtual void
      post (Type&);
    };
  }
}

#endif  // XSD_FRONTEND_TRAVERSAL_ATTRIBUTE_GROUP_HXX
