// file      : xsd-frontend/semantic-graph/union.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_SEMANTIC_GRAPH_UNION_HXX
#define XSD_FRONTEND_SEMANTIC_GRAPH_UNION_HXX

#include <xsd-frontend/semantic-graph/elements.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    class Union: public virtual Specialization
    {
    public:
      Union (Path const& file, unsigned long line, unsigned long column);
    };
  }
}

#endif  // XSD_FRONTEND_SEMANTIC_GRAPH_UNION_HXX
