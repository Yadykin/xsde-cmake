// file      : xsd-frontend/semantic-graph/attribute.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_SEMANTIC_GRAPH_ATTRIBUTE_HXX
#define XSD_FRONTEND_SEMANTIC_GRAPH_ATTRIBUTE_HXX

#include <xsd-frontend/semantic-graph/elements.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    class Attribute: public virtual Member
    {
    public:
      bool
      optional_p () const
      {
        return optional_;
      }

    public:
      Attribute (Path const& file,
                 unsigned long line,
                 unsigned long column,
                 bool optional,
                 bool global,
                 bool qualified);
    private:
      bool optional_;
    };
  }
}

#endif  // XSD_FRONTEND_SEMANTIC_GRAPH_ATTRIBUTE_HXX
