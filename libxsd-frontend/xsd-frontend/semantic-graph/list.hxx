// file      : xsd-frontend/semantic-graph/list.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_SEMANTIC_GRAPH_LIST_HXX
#define XSD_FRONTEND_SEMANTIC_GRAPH_LIST_HXX

#include <xsd-frontend/semantic-graph/elements.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    class List: public virtual Specialization
    {
    public:
      List (Path const& file, unsigned long line, unsigned long column);
    };
  }
}

#endif  // XSD_FRONTEND_SEMANTIC_GRAPH_LIST_HXX
