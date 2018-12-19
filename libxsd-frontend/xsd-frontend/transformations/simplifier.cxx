// file      : xsd-frontend/transformations/simplifier.cxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsd-frontend/semantic-graph.hxx>
#include <xsd-frontend/traversal.hxx>

#include <xsd-frontend/transformations/simplifier.hxx>

namespace XSDFrontend
{
  namespace
  {
    struct Compositor: Traversal::All,
                       Traversal::Choice,
                       Traversal::Sequence
    {
      Compositor (SemanticGraph::Schema& root)
          : root_ (root)
      {
      }

      virtual void
      traverse (SemanticGraph::All& a)
      {
        // The all compositor cannot contain compositors.
        //
        if (a.contains_begin () == a.contains_end ())
          remove (a);
      }

      virtual void
      traverse (SemanticGraph::Choice& c)
      {
        // Do the depth-first traversal so that we take into account
        // the potential removal of nested compositors.
        //
        using SemanticGraph::Compositor;

        for (Compositor::ContainsIterator i (c.contains_begin ());
             i != c.contains_end ();)
        {
          edge_traverser ().dispatch (*i++);
        }

        Choice::contains (c);

        if (c.contains_begin () == c.contains_end ())
          remove (c);
      }

      virtual void
      traverse (SemanticGraph::Sequence& s)
      {
        // Do the depth-first traversal so that we take into account
        // the potential removal of nested compositors.
        //
        using SemanticGraph::Compositor;

        for (Compositor::ContainsIterator i (s.contains_begin ());
             i != s.contains_end ();)
        {
          edge_traverser ().dispatch (*i++);
        }

        if (s.contains_begin () == s.contains_end ())
          remove (s);
      }

    private:
      virtual void
      remove (SemanticGraph::Compositor& c)
      {
        using SemanticGraph::Node;
        using SemanticGraph::Choice;
        using SemanticGraph::Complex;
        using SemanticGraph::Compositor;

        if (c.contained_particle_p ())
        {
          Compositor& com (c.contained_particle ().compositor ());

          // Empty compositors in choice are important.
          //
          if (!com.is_a<Choice> ())
            root_.delete_edge (com, c, c.contained_particle ());
        }
        else
        {
          Complex& con (
            dynamic_cast<Complex&> (c.contained_compositor ().container ()));
          root_.delete_edge (con, c, c.contained_compositor ());
        }
      }

    private:
      SemanticGraph::Schema& root_;
    };

    //
    //
    struct Type: Traversal::Complex
    {
      virtual void
      traverse (SemanticGraph::Complex& c)
      {
        if (c.contains_compositor_p ())
          Complex::contains_compositor (c);
      }
    };

    // Go into implied/included/imported schemas while making sure
    // we don't process the same stuff more than once.
    //
    struct Uses: Traversal::Uses
    {
      virtual void
      traverse (Type& u)
      {
        SemanticGraph::Schema& s (u.schema ());

        if (!s.context ().count ("xsd-frontend-simplifier-seen"))
        {
          s.context ().set ("xsd-frontend-simplifier-seen", true);
          Traversal::Uses::traverse (u);
        }
      }
    };
  }

  namespace Transformations
  {
    void Simplifier::
    transform (SemanticGraph::Schema& s, SemanticGraph::Path const&)
    {
      Traversal::Schema schema;
      Uses uses;

      schema >> uses >> schema;

      Traversal::Names schema_names;
      Traversal::Namespace ns;
      Traversal::Names ns_names;
      Type type;

      schema >> schema_names >> ns >> ns_names >> type;

      Compositor compositor (s);
      Traversal::ContainsCompositor contains_compositor;
      Traversal::ContainsParticle contains_particle;

      type >> contains_compositor >> compositor;
      compositor >> contains_particle >> compositor;

      // Some twisted schemas do recusive inclusions.
      //
      s.context ().set ("xsd-frontend-simplifier-seen", true);

      schema.dispatch (s);
    }
  }
}
