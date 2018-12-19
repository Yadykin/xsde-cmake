// file      : xsd-frontend/transformations/enum-synthesis.cxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <set>

#include <xsd-frontend/semantic-graph.hxx>
#include <xsd-frontend/traversal.hxx>

#include <xsd-frontend/transformations/enum-synthesis.hxx>

namespace XSDFrontend
{
  namespace
  {
    typedef std::set<String> Enumerators;

    struct Enumerator: Traversal::Enumerator
    {
      Enumerator (SemanticGraph::Schema& s,
                  SemanticGraph::Enumeration& e,
                  Enumerators& enumerators)
          : schema_ (s), enum_ (e), enumerators_ (enumerators)
      {
      }

      virtual void
      traverse (Type& e)
      {
        String const& name (e.name ());

        if (enumerators_.find (name) == enumerators_.end ())
        {
          enumerators_.insert (name);

          // Clone the enumerator and add it to enum_.
          //
          Type& c (schema_.new_node<Type> (e.file (), e.line (), e.column ()));

          schema_.new_edge<SemanticGraph::Names> (enum_, c, name);
          schema_.new_edge<SemanticGraph::Belongs> (c, enum_);

          if (e.annotated_p ())
            schema_.new_edge<SemanticGraph::Annotates> (e.annotation (), c);
        }
      }

    private:
      SemanticGraph::Schema& schema_;
      SemanticGraph::Enumeration& enum_;
      Enumerators& enumerators_;
    };

    //
    //
    struct Union: Traversal::Union
    {
      Union (SemanticGraph::Schema& schema)
          : schema_ (schema)
      {
      }

      virtual void
      traverse (Type& u)
      {
        using SemanticGraph::Enumeration;

        SemanticGraph::Context& uc (u.context ());

        if (uc.count ("xsd-frontend-enum-synthesis-processed"))
          return;

        uc.set ("xsd-frontend-enum-synthesis-processed", true);

        // First see if this union is suitable for synthesis.
        //
        SemanticGraph::Type* base (0);

        for (Type::ArgumentedIterator i (u.argumented_begin ());
             i != u.argumented_end (); ++i)
        {
          if (i->type ().is_a<SemanticGraph::Union> ())
          {
            // See if we can synthesize an enum for this union. This
            // call can change the value i->type() returns.
            //
            dispatch (i->type ());
          }

          SemanticGraph::Type& t (i->type ());

          if (!t.is_a<Enumeration> ())
            return;

          // Make sure all the enums have a common base.
          //
          if (base == 0)
            base = &t;
          else
          {
            // Traverse the inheritance hierarchy until we fine a
            // common base.
            //
            while (base != 0)
            {
              SemanticGraph::Type* b (&t);

              for (; b != base && b->inherits_p ();
                   b = &b->inherits ().base ()) ;

              if (base == b)
                break;

              // Could not find any match on this level. Go one step
              // lower and try again.
              //
              base = base->inherits_p () ? &base->inherits ().base () : 0;
            }

            if (base == 0)
              return; // No common base.
          }
        }

        if (base == 0)
          return; // Empty union.

        // So this union is suitable for synthesis. Base variable points
        // to the "most-derived" common base type.
        //
        Enumeration& e (schema_.new_node<Enumeration> (
                          u.file (), u.line (), u.column ()));

        schema_.new_edge<SemanticGraph::Restricts> (e, *base);

        // Copy enumerators from the member enums.
        //
        {
          Enumerators set;
          Traversal::Enumeration en;
          Traversal::Names names;
          Enumerator er (schema_, e, set);
          en >> names >> er;

          for (Type::ArgumentedIterator i (u.argumented_begin ());
               i != u.argumented_end (); ++i)
          {
            en.dispatch (i->type ());
          }
        }

        // Reset edges pointing to union to point to enum.
        //
        if (u.annotated_p ())
        {
          schema_.reset_right_node (u.annotated (), e);
          schema_.add_edge_right (e, u.annotated ());
        }

        schema_.reset_right_node (u.named (), e);
        schema_.add_edge_right (e, u.named ());

        for (Type::ClassifiesIterator i (u.classifies_begin ()),
               end (u.classifies_end ()); i != end; ++i)
        {
          schema_.reset_right_node (*i, e);
          schema_.add_edge_right (e, *i);
        }

        for (Type::BegetsIterator i (u.begets_begin ()),
               end (u.begets_end ()); i != end; ++i)
        {
          schema_.reset_right_node (*i, e);
          schema_.add_edge_right (e, *i);
        }

        for (Type::ArgumentsIterator i (u.arguments_begin ()),
               end (u.arguments_end ()); i != end; ++i)
        {
          schema_.reset_left_node (*i, e);
          schema_.add_edge_left (e, *i);
        }

        // Remove Arguments edges pointing to the union.
        //
        while (u.argumented_begin () != u.argumented_end ())
        {
          SemanticGraph::Arguments& a (*u.argumented_begin ());
          schema_.delete_edge (a.type (), a.specialization (), a);
        }

        // Copy context and delete the union node.
        //
        e.context ().swap (uc);
        schema_.delete_node (u);
      }

    private:
      SemanticGraph::Schema& schema_;
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

        if (!s.context ().count ("xsd-frontend-enum-synthesis-seen"))
        {
          s.context ().set ("xsd-frontend-enum-synthesis-seen", true);
          Traversal::Uses::traverse (u);
        }
      }
    };
  }

  namespace Transformations
  {
    void EnumSynthesis::
    transform (SemanticGraph::Schema& s, SemanticGraph::Path const&)
    {
      Traversal::Schema schema;
      Uses uses;

      schema >> uses >> schema;

      Traversal::Names schema_names;
      Traversal::Namespace ns;
      Traversal::Names ns_names;
      Union u (s);

      schema >> schema_names >> ns >> ns_names >> u;

      // Some twisted schemas do recusive inclusions.
      //
      s.context ().set ("xsd-frontend-enum-synthesis-seen", true);

      schema.dispatch (s);
    }
  }
}
