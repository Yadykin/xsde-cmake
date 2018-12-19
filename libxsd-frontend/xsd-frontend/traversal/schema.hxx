// file      : xsd-frontend/traversal/schema.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_TRAVERSAL_SCHEMA_HXX
#define XSD_FRONTEND_TRAVERSAL_SCHEMA_HXX

#include <xsd-frontend/traversal/elements.hxx>
#include <xsd-frontend/semantic-graph/schema.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    //
    //
    struct Uses: Edge<SemanticGraph::Uses>
    {
      Uses ()
      {
      }

      Uses (NodeBase& n)
      {
        node_traverser (n);
      }

      virtual void
      traverse (Type& e)
      {
        dispatch (e.schema ());
      }
    };

    //
    //
    struct Implies: Edge<SemanticGraph::Implies>
    {
      Implies ()
      {
      }

      Implies (NodeBase& n)
      {
        node_traverser (n);
      }

      virtual void
      traverse (Type& e)
      {
        dispatch (e.schema ());
      }
    };


    //
    //
    struct Sources: Edge<SemanticGraph::Sources>
    {
      Sources ()
      {
      }

      Sources (NodeBase& n)
      {
        node_traverser (n);
      }

      virtual void
      traverse (Type& e)
      {
        dispatch (e.schema ());
      }
    };


    //
    //
    struct Includes: Edge<SemanticGraph::Includes>
    {
      Includes ()
      {
      }

      Includes (NodeBase& n)
      {
        node_traverser (n);
      }

      virtual void
      traverse (Type& e)
      {
        dispatch (e.schema ());
      }
    };


    //
    //
    struct Imports: Edge<SemanticGraph::Imports>
    {
      Imports ()
      {
      }

      Imports (NodeBase& n)
      {
        node_traverser (n);
      }

      virtual void
      traverse (Type& e)
      {
        dispatch (e.schema ());
      }
    };


    //
    //
    struct Schema: ScopeTemplate<SemanticGraph::Schema>
    {
      virtual void
      traverse (Type& s)
      {
        pre (s);

        iterate_and_dispatch (
          s.uses_begin (), s.uses_end (), edge_traverser ());

        names (s);

        post (s);
      }

      virtual void
      pre (Type&)
      {
      }

      virtual void
      post (Type&)
      {
      }
    };
  }
}

#endif  // XSD_FRONTEND_TRAVERSAL_SCHEMA_HXX
