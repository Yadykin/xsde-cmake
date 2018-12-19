// file      : xsd-frontend/semantic-graph/enumeration.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_SEMANTIC_GRAPH_ENUMERATION_HXX
#define XSD_FRONTEND_SEMANTIC_GRAPH_ENUMERATION_HXX

#include <xsd-frontend/semantic-graph/elements.hxx>
#include <xsd-frontend/semantic-graph/complex.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    class Enumeration: public virtual Complex
    {
    public:
      Enumeration (Path const& file, unsigned long line, unsigned long column);
    };


    class Enumerator: public virtual Instance
    {
    public:
      Enumerator (Path const& file, unsigned long line, unsigned long column);
    };
  }
}

#endif  // XSD_FRONTEND_SEMANTIC_GRAPH_ENUMERATION_HXX
