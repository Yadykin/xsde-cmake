// file      : xsd-frontend/semantic-graph/element-group.hxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_SEMANTIC_GRAPH_ELEMENT_GROUP_HXX
#define XSD_FRONTEND_SEMANTIC_GRAPH_ELEMENT_GROUP_HXX

#include <xsd-frontend/semantic-graph/elements.hxx>
#include <xsd-frontend/semantic-graph/compositors.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    class ElementGroup: public virtual Scope
    {
    public:
      ContainsCompositor&
      contains_compositor ()
      {
        assert (contains_compositor_ != 0);
        return *contains_compositor_;
      }

    public:
      ElementGroup (Path const& file, unsigned long line, unsigned long column);

      void
      add_edge_left (ContainsCompositor& e)
      {
        contains_compositor_ = &e;
      }

      using Scope::add_edge_left;

    private:
      ContainsCompositor* contains_compositor_;
    };
  }
}

#endif  // XSD_FRONTEND_SEMANTIC_GRAPH_ELEMENT_GROUP_HXX
