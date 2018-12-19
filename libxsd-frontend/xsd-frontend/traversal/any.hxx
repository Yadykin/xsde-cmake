// file      : xsd-frontend/traversal/any.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_TRAVERSAL_ANY_HXX
#define XSD_FRONTEND_TRAVERSAL_ANY_HXX

#include <xsd-frontend/traversal/elements.hxx>
#include <xsd-frontend/semantic-graph/any.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    typedef
    Node<SemanticGraph::Any>
    Any;
  }
}

#endif  // XSD_FRONTEND_TRAVERSAL_ANY_HXX
