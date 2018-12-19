// file      : xsd-frontend/semantic-graph/namespace.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_SEMANTIC_GRAPH_NAMESPACE_HXX
#define XSD_FRONTEND_SEMANTIC_GRAPH_NAMESPACE_HXX

#include <xsd-frontend/semantic-graph/elements.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    class Namespace : public virtual Scope
    {
    public:
      Namespace (Path const& file, unsigned long line, unsigned long column);

      void
      add_edge_right (BelongsToNamespace&) {}

      using Scope::add_edge_right;
    };
  }
}

#endif  // XSD_FRONTEND_SEMANTIC_GRAPH_NAMESPACE_HXX
