// file      : xsd-frontend/transformations/restriction.cxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <vector>
#include <iostream>

#include <xsd-frontend/semantic-graph.hxx>
#include <xsd-frontend/traversal.hxx>

#include <xsd-frontend/transformations/restriction.hxx>

using namespace std;

namespace XSDFrontend
{
  typedef Transformations::Restriction::Failed Failed;
  typedef std::vector<SemanticGraph::Complex*> BaseList;

  namespace
  {
    //
    //
    struct Complex: Traversal::Complex
    {
      Complex (SemanticGraph::Schema& schema)
          : schema_ (schema)
      {
      }

      virtual void
      traverse (Type& c)
      {
        using namespace SemanticGraph;
        using SemanticGraph::Complex;

        if (c.context ().count ("xsd-frontend-restriction-seen"))
          return;

        c.context ().set ("xsd-frontend-restriction-seen", true);

        // The base content model can be spread over several types
        // in the inheritance-by-extension hierarchy.
        //
        BaseList base_model;

        // Since attribute wildcards don't have names, we will have
        // to rely on their relative position to find association.
        //
        BaseList base_list;

        // Current implementation of semantic graph uses the same Restricts
        // edge for both simple type/content restriction and complex content
        // restriction. Here we are interested in the complex content only.
        //
        //
        if (c.inherits_p () &&
            c.inherits ().is_a<Restricts> () &&
            !c.inherits ().base ().is_a<AnyType> ())
        {
          // Go down our inheritance hierarchy until the end or the previous
          // restriction.
          //
          Complex* base (&c);

          while ((base = dynamic_cast<Complex*> (&base->inherits ().base ())))
          {
            traverse (*base); // Make sure our base is processed.

            // Handle attributes.
            //
            merge_attributes (c, *base);

            base_list.push_back (base);

            // Collect types that have complex content.
            //
            if (base->contains_compositor_p ())
              base_model.push_back (base);

            if (!base->inherits_p () || base->inherits ().is_a<Restricts> ())
              break;
          }

          // Handle attribute wildcards.
          //
          handle_any_attributes (c, base_list);

          // Handle complex content (not for the faint of heart).
          //
          if (c.contains_compositor_p ())
          {
            // Traverse both restricted content model and base content
            // model (in base_model) while looking for matches.
            //
            Compositor& root (c.contains_compositor ().compositor ());

            if (base_model.size () == 1)
              handle (root,
                      base_model[0]->contains_compositor ().compositor ());
            else
            {
              Compositor::ContainsIterator i (root.contains_begin ());
              BaseList::reverse_iterator j (base_model.rbegin ());

              for (; i != root.contains_end (); ++i, ++j)
              {
                Particle& p (i->particle ());

                if (!p.is_a<Compositor> ())
                {
                  wcerr << p.file () << ":" << p.line () << ":" << p.column ()
                        << ": error: expected compositor instead of particle"
                        << endl;
                  throw Failed ();
                }

                for (; j != base_model.rend (); ++j)
                {
                  if (match (p, (*j)->contains_compositor ().compositor ()))
                  {
                    handle (p, (*j)->contains_compositor ().compositor ());
                    break;
                  }
                }

                if (j == base_model.rend ())
                  break;
              }

              if (i != root.contains_end ())
              {
                Particle& p (i->particle ());

                wcerr << p.file () << ":" << p.line () << ":" << p.column ()
                      << ": error: unable to match restricted compositor"
                      << endl;
                throw Failed ();
              }
            }
          }
        }

        // Traverse anonymous types (via elements & attributes).
        //
        Traversal::Complex::names (c);
      }

    private:
      void
      handle (SemanticGraph::Particle& r, SemanticGraph::Particle& b)
      {
        using namespace SemanticGraph;

        if (r.is_a<Compositor> ())
        {
          Compositor& rc (dynamic_cast<Compositor&> (r));
          Compositor& bc (dynamic_cast<Compositor&> (b));

          Compositor::ContainsIterator i (rc.contains_begin ());
          Compositor::ContainsIterator j (bc.contains_begin ());

          for (; i != rc.contains_end (); ++i, ++j)
          {
            for (; j != bc.contains_end (); ++j)
            {
              Particle& rp (i->particle ());
              Particle& bp (j->particle ());

              if (typeid (rp) != typeid (bp))
                continue;

              if (match (rp, bp))
              {
                handle (rp, bp);
                break;
              }
            }

            if (j == bc.contains_end ())
              break;
          }

          if (i != rc.contains_end ())
          {
            Particle& p (i->particle ());

            wcerr << p.file () << ":" << p.line () << ":" << p.column ()
                  << ": error: unable to match restricted particle"
                  << endl;
            throw Failed ();
          }

          rc.context ().set ("xsd-frontend-restriction-correspondence", &bc);
        }
        else if (r.is_a<Element> ())
        {
          // Element
          //
          r.context ().set ("xsd-frontend-restriction-correspondence",
                            dynamic_cast<Element*> (&b));
        }
        else
        {
          // Wildcard
          //
          r.context ().set ("xsd-frontend-restriction-correspondence",
                            dynamic_cast<Any*> (&b));
        }
      }

      bool
      match (SemanticGraph::Particle& r, SemanticGraph::Particle& b)
      {
        using namespace SemanticGraph;

        if (typeid (r) != typeid (b))
          return false;

        if (r.is_a<Compositor> ())
        {
          Compositor& rc (dynamic_cast<Compositor&> (r));
          Compositor& bc (dynamic_cast<Compositor&> (b));

          Compositor::ContainsIterator i (rc.contains_begin ());

          if (i == rc.contains_end ())
            return true;

          Particle& rp (i->particle ());

          for (Compositor::ContainsIterator j (bc.contains_begin ());
               j != bc.contains_end (); ++j)
          {
            Particle& bp (j->particle ());

            if (typeid (rp) != typeid (bp))
              continue;

            if (match (rp, bp))
              return true;
          }
        }
        else if (r.is_a<Element> ())
        {
          Element& re (dynamic_cast<Element&> (r));
          Element& be (dynamic_cast<Element&> (b));

          if (re.qualified_p ())
          {
            if (be.qualified_p () &&
                re.name () == be.name () &&
                re.namespace_ ().name () == be.namespace_ ().name ())
              return true;
          }
          else
          {
            if (!be.qualified_p () && re.name () == be.name ())
              return true;
          }

          // @@ Need to take into account substitution groups.
          //
        }
        else
        {
          // Wildcard.
          //

          // @@ To handle this properly we will need to analyze
          //    namespaces.
          //
          return true;
        }

        return false;
      }

      void
      merge_attributes (SemanticGraph::Complex& c,
                        SemanticGraph::Complex& base)
      {
        using namespace SemanticGraph;

        for (Scope::NamesIterator i (base.names_begin ()),
               e (base.names_end ()); i != e; ++i)
        {
          Attribute* prot (dynamic_cast<Attribute*> (&i->named ()));

          if (prot == 0)
            continue;

          Name name (prot->name ());
          Scope::NamesIteratorPair r (c.find (name));

          Attribute* a (0);

          for (; r.first != r.second; ++r.first)
          {
            a = dynamic_cast<Attribute*> (&r.first->named ());

            if (a == 0)
              continue;

            if (prot->qualified_p ())
            {
              if (a->qualified_p () &&
                  prot->namespace_ ().name () == a->namespace_ ().name ())
              {
                break;
              }
            }
            else
            {
              if (!a->qualified_p ())
                break;
            }

            a = 0;
          }

          if (a == 0)
          {
            a = &schema_.new_node<Attribute> (prot->file (),
                                              prot->line (),
                                              prot->column (),
                                              prot->optional_p (),
                                              prot->global_p (),
                                              prot->qualified_p ());

            schema_.new_edge<Names> (c, *a, name);

            // Transfer namespace.
            //
            if (prot->qualified_p ())
            {
              schema_.new_edge<BelongsToNamespace> (*a, prot->namespace_ ());
            }

            // Default and fixed values if any.
            //
            if (prot->fixed_p ())
              a->fixed (prot->value ());
            else if (prot->default_p ())
              a->default_ (prot->value ());

            // Belongs edge.
            //
            schema_.new_edge<Belongs> (*a, prot->type ());

            // Transfer annotation.
            //
            if (prot->annotated_p ())
              schema_.new_edge<Annotates> (prot->annotation (), *a);
          }

          a->context ().set ("xsd-frontend-restriction-correspondence", prot);
        }
      }

      void
      handle_any_attributes (SemanticGraph::Complex& c, BaseList& bl)
      {
        using namespace SemanticGraph;

        BaseList::reverse_iterator bi (bl.rbegin ()), be (bl.rend ());
        Scope::NamesIterator si;

        if (bi != be)
          si = (*bi)->names_begin ();

        for (Scope::NamesIterator i (c.names_begin ()),
               e (c.names_end ()); i != e; ++i)
        {
          AnyAttribute* a (dynamic_cast<AnyAttribute*> (&i->named ()));

          if (a == 0)
            continue;

          AnyAttribute* p (0);

          while (bi != be)
          {
            for (; si != (*bi)->names_end (); ++si)
            {
              p = dynamic_cast<AnyAttribute*> (&si->named ());

              if (p != 0)
              {
                ++si;
                break;
              }
            }

            if (p != 0)
              break;

            // Didn't find anything in this base. Move on to the next.
            //
            ++bi;

            if (bi != be)
              si = (*bi)->names_begin ();
          }

          if (p != 0)
          {
            a->context ().set ("xsd-frontend-restriction-correspondence", p);
          }
          else
          {
            wcerr << a->file () << ":" << a->line () << ":" << a->column ()
                  << ": error: unable to find matching wildcard in base type"
                  << endl;
            throw Failed ();
          }
        }
      }

    private:
      SemanticGraph::Schema& schema_;
    };

    //
    //
    struct Anonymous : Traversal::Element,
                       Traversal::Attribute
    {
      Anonymous (Traversal::NodeDispatcher& d1)
          : complex_ (&d1, 0)
      {
        *this >> belongs_ >> complex_;
      }

      Anonymous (Traversal::NodeDispatcher& d1,
                 Traversal::NodeDispatcher& d2)
          : complex_ (&d1, &d2)
      {
        *this >> belongs_ >> complex_;
      }

      // Hooks.
      //
    public:
      virtual void
      member_pre (SemanticGraph::Member&)
      {
      }

      virtual void
      member_post (SemanticGraph::Member&)
      {
      }

    public:

      virtual void
      traverse (SemanticGraph::Element& e)
      {
        SemanticGraph::Type& t (e.type ());

        if (!t.named_p () && !t.context ().count ("seen"))
        {
          t.context ().set ("seen", true);

          member_pre (e);

          Element::belongs (e, belongs_);

          member_post (e);

          t.context ().remove ("seen");
        }
      }

      virtual void
      traverse (SemanticGraph::Attribute& a)
      {
        SemanticGraph::Type& t (a.type ());

        if (!t.named_p () && !t.context ().count ("seen"))
        {
          t.context ().set ("seen", true);

          member_pre (a);

          Attribute::belongs (a, belongs_);

          member_post (a);

          t.context ().remove ("seen");
        }
      }

    private:
      struct Complex : Traversal::Complex
      {
        Complex (Traversal::NodeDispatcher* d1,
                 Traversal::NodeDispatcher* d2)
            : d1_ (d1), d2_ (d2)
        {
        }

        virtual void
        traverse (SemanticGraph::Complex& c)
        {
          if (d1_)
            d1_->dispatch (c);

          if (d2_)
            d2_->dispatch (c);
        }

      private:
        Traversal::NodeDispatcher* d1_;
        Traversal::NodeDispatcher* d2_;

      } complex_;

      Traversal::Belongs belongs_;
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

        if (!s.context ().count ("xsd-frontend-restriction-seen"))
        {
          s.context ().set ("xsd-frontend-restriction-seen", true);
          Traversal::Uses::traverse (u);
        }
      }
    };
  }

  namespace Transformations
  {
    void Restriction::
    transform (SemanticGraph::Schema& s, SemanticGraph::Path const&)
    {
      Traversal::Schema schema;
      Uses uses;

      schema >> uses >> schema;

      Traversal::Names schema_names;
      Traversal::Namespace ns;
      Traversal::Names ns_names;

      schema >> schema_names >> ns >> ns_names;

      Complex complex_type (s);
      Anonymous anonymous (complex_type);

      ns_names >> complex_type;
      ns_names >> anonymous;

      Traversal::Names names;

      complex_type >> names >> anonymous;

      // Some twisted schemas do recusive inclusions.
      //
      s.context ().set ("xsd-frontend-restriction-seen", true);

      schema.dispatch (s);
    }
  }
}
