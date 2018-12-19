// file      : xsd-frontend/traversal/compositors.hxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_TRAVERSAL_COMPOSITORS_HXX
#define XSD_FRONTEND_TRAVERSAL_COMPOSITORS_HXX

#include <xsd-frontend/traversal/elements.hxx>
#include <xsd-frontend/semantic-graph/compositors.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    //
    //
    struct ContainsParticle: Edge<SemanticGraph::ContainsParticle>
    {
      ContainsParticle ()
      {
      }

      ContainsParticle (NodeBase& n)
      {
        node_traverser (n);
      }

      virtual void
      traverse (Type&);
    };


    //
    //
    struct ContainsCompositor: Edge<SemanticGraph::ContainsCompositor>
    {
      ContainsCompositor ()
      {
      }

      ContainsCompositor (NodeBase& n)
      {
        node_traverser (n);
      }

      virtual void
      traverse (Type&);
    };

    //
    //
    struct Compositor : Node<SemanticGraph::Compositor>
    {
      virtual void
      traverse (Type&);

      virtual void
      pre (Type&);

      virtual void
      contains (Type&);

      virtual void
      contains (Type&, EdgeDispatcher&);

      virtual void
      post (Type&);
    };


    //
    //
    struct All : Node<SemanticGraph::All>
    {
      virtual void
      traverse (Type&);

      virtual void
      pre (Type&);

      virtual void
      contains (Type&);

      virtual void
      contains (Type&, EdgeDispatcher&);

      virtual void
      post (Type&);
    };


    //
    //
    struct Choice : Node<SemanticGraph::Choice>
    {
      virtual void
      traverse (Type&);

      virtual void
      pre (Type&);

      virtual void
      contains (Type&);

      virtual void
      contains (Type&, EdgeDispatcher&);

      virtual void
      post (Type&);
    };


    //
    //
    struct Sequence : Node<SemanticGraph::Sequence>
    {
      virtual void
      traverse (Type&);

      virtual void
      pre (Type&);

      virtual void
      contains (Type&);

      virtual void
      contains (Type&, EdgeDispatcher&);

      virtual void
      post (Type&);
    };
  }
}

#endif  // XSD_FRONTEND_TRAVERSAL_COMPOSITORS_HXX
