// file      : xsd-frontend/traversal/list.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_TRAVERSAL_LIST_HXX
#define XSD_FRONTEND_TRAVERSAL_LIST_HXX

#include <xsd-frontend/traversal/elements.hxx>
#include <xsd-frontend/semantic-graph/list.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    struct List: Node<SemanticGraph::List>
    {
      virtual void
      traverse (Type&);

      virtual void
      pre (Type&);

      virtual void
      argumented (Type&);

      virtual void
      argumented (Type&, EdgeDispatcher& d);

      virtual void
      name (Type&);

      virtual void
      post (Type&);
    };
  }
}

#endif  // XSD_FRONTEND_TRAVERSAL_LIST_HXX
