// file      : xsd-frontend/semantic-graph/annotation.hxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_SEMANTIC_GRAPH_ANNOTATION_HXX
#define XSD_FRONTEND_SEMANTIC_GRAPH_ANNOTATION_HXX

#include <xsd-frontend/semantic-graph/elements.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    //
    //
    class Annotation;

    class Annotates: public virtual Edge
    {
    public:
      Annotation&
      annotation ()
      {
        return *annotation_;
      }

    public:
      Annotates (): annotation_ (0) {}

      void
      set_left_node (Annotation& a)
      {
        annotation_ = &a;
      }

      void
      set_right_node (Node&) {}

      void
      set_right_node (Edge&) {}

    private:
      Annotation* annotation_;
    };

    //
    //
    class Annotation: public virtual Node
    {
    public:
      String const&
      documentation () const
      {
        return documentation_;
      }

    public:
      Annotation (Path const& file,
                  unsigned long line,
                  unsigned long column,
                  String const& documentation)
          : Node (file, line, column), documentation_ (documentation)
      {
      }

      void
      add_edge_left (Annotates&) {}

    private:
      String documentation_;
    };
  }
}

#endif  // XSD_FRONTEND_SEMANTIC_GRAPH_ANNOTATION_HXX
