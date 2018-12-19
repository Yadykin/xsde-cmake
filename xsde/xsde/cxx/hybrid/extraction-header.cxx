// file      : xsd/cxx/hybrid/extraction-header.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cxx/hybrid/extraction-header.hxx>

#include <xsd-frontend/semantic-graph.hxx>
#include <xsd-frontend/traversal.hxx>

namespace CXX
{
  namespace Hybrid
  {
    namespace
    {
      struct Enumeration : Traversal::Enumeration, Context
      {
        Enumeration (Context& c, Traversal::Complex& complex)
            : Context (c), complex_ (complex)
        {
        }

        virtual void
        traverse (Type& e)
        {
          // First see if we should delegate this one to the Complex
          // generator.
          //
          if (!enum_ || !enum_mapping (e))
          {
            complex_.traverse (e);
            return;
          }

          String const& name (ename_custom (e));

          // We may not need to generate the class if this type is
          // being customized.
          //
          if (!name)
            return;

          for (NarrowStrings::const_iterator i (istreams.begin ());
               i != istreams.end (); ++i)
          {
            os << (exceptions ? "void" : "bool") << endl
               << "operator>> (" << istream (*i) << "&," << endl
               << name << "&);"
               << endl;
          }
        }

      private:
        Traversal::Complex& complex_;
      };

      struct List : Traversal::List, Context
      {
        List (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (Type& l)
        {
          String const& name (ename_custom (l));

          // We may not need to generate the class if this type is
          // being customized.
          //
          if (!name)
            return;

          for (NarrowStrings::const_iterator i (istreams.begin ());
               i != istreams.end (); ++i)
          {
            os << (exceptions ? "void" : "bool") << endl
               << "operator>> (" << istream (*i) << "&," << endl
               << name << "&);"
               << endl;
          }
        }
      };

      struct Union : Traversal::Union, Context
      {
        Union (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (Type& u)
        {
          String const& name (ename_custom (u));

          // We may not need to generate the class if this type is
          // being customized.
          //
          if (!name)
            return;

          for (NarrowStrings::const_iterator i (istreams.begin ());
               i != istreams.end (); ++i)
          {
            os << (exceptions ? "void" : "bool") << endl
               << "operator>> (" << istream (*i) << "&," << endl
               << name << "&);"
               << endl;
          }
        }
      };

      //
      // Nested classes.
      //

      struct All: Traversal::All, Context
      {
        All (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (SemanticGraph::All& a)
        {
          // For the all compositor, maxOccurs=1 and minOccurs={0,1}
          // and it can only contain particles.
          //
          if (a.min () == 0)
          {
            String const& type (etype (a));
            String const& scope (Context::scope (a));

            for (NarrowStrings::const_iterator i (istreams.begin ());
                 i != istreams.end (); ++i)
            {
              os << (exceptions ? "void" : "bool") << endl
                 << "operator>> (" << istream (*i) << "&," << endl
                 << scope << "::" << type << "&);"
                 << endl;
            }
          }
        }
      };

      struct Choice: Traversal::Choice, Context
      {
        Choice (Context& c, bool in_choice)
            : Context (c), in_choice_ (in_choice)
        {
        }

        virtual void
        traverse (SemanticGraph::Choice& c)
        {
          // When choice is in choice we generate nested class even
          // for min == max == 1.
          //
          if (in_choice_ || c.max () != 1 || c.min () == 0)
          {
            String const& type (etype (c));
            String const& scope (Context::scope (c));

            for (NarrowStrings::const_iterator i (istreams.begin ());
                 i != istreams.end (); ++i)
            {
              os << (exceptions ? "void" : "bool") << endl
                 << "operator>> (" << istream (*i) << "&," << endl
                 << scope << "::" << type << "&);"
                 << endl;
            }
          }

          Choice::contains (c);
        }

      private:
        bool in_choice_;
      };


      struct Sequence: Traversal::Sequence, Context
      {
        Sequence (Context& c, bool in_choice)
            : Context (c), in_choice_ (in_choice)
        {
        }

        virtual void
        traverse (SemanticGraph::Sequence& s)
        {
          // When sequence is in choice we generate nested class even
          // for min == max == 1.
          //
          if (in_choice_ || s.max () != 1 || s.min () == 0)
          {
            String const& type (etype (s));
            String const& scope (Context::scope (s));

            for (NarrowStrings::const_iterator i (istreams.begin ());
                 i != istreams.end (); ++i)
            {
              os << (exceptions ? "void" : "bool") << endl
                 << "operator>> (" << istream (*i) << "&," << endl
                 << scope << "::" << type << "&);"
                 << endl;
            }
          }

          Sequence::contains (s);
        }

      private:
        bool in_choice_;
      };

      struct Complex : Traversal::Complex, Context
      {
        Complex (Context& c)
            : Context (c),
              all_ (c),
              choice_in_choice_ (c, true),
              choice_in_sequence_ (c, false),
              sequence_in_choice_ (c, true),
              sequence_in_sequence_ (c, false)
        {
          all_ >> all_contains_;

          choice_in_choice_ >> choice_contains_;
          choice_in_sequence_ >> choice_contains_;
          choice_contains_ >> choice_in_choice_;
          choice_contains_ >> sequence_in_choice_;

          sequence_in_choice_ >> sequence_contains_;
          sequence_in_sequence_ >> sequence_contains_;
          sequence_contains_ >> choice_in_sequence_;
          sequence_contains_ >> sequence_in_sequence_;

          contains_compositor_ >> all_;
          contains_compositor_ >> choice_in_sequence_;
          contains_compositor_ >> sequence_in_sequence_;
        }

        virtual void
        traverse (Type& c)
        {
          String const& name (ename_custom (c));

          // We may not need to generate the class if this type is
          // being customized.
          //
          if (!name)
            return;

          for (NarrowStrings::const_iterator i (istreams.begin ());
               i != istreams.end (); ++i)
          {
            os << (exceptions ? "void" : "bool") << endl
               << "operator>> (" << istream (*i) << "&," << endl
               << name << "&);"
               << endl;
          }

          // Operators for nested classes.
          //
          if (!restriction_p (c) && c.contains_compositor_p ())
            Complex::contains_compositor (c, contains_compositor_);
        }

      private:
        All all_;
        Choice choice_in_choice_;
        Choice choice_in_sequence_;
        Sequence sequence_in_choice_;
        Sequence sequence_in_sequence_;
        Traversal::ContainsParticle all_contains_;
        Traversal::ContainsParticle choice_contains_;
        Traversal::ContainsParticle sequence_contains_;

        Traversal::ContainsCompositor contains_compositor_;
      };
    }

    void
    generate_extraction_header (Context& ctx)
    {
      Traversal::Schema schema;
      Sources sources;
      Traversal::Names names_ns, names;

      Namespace ns (ctx);

      List list (ctx);
      Union union_ (ctx);
      Complex complex (ctx);
      Enumeration enumeration (ctx, complex);

      schema >> sources >> schema;
      schema >> names_ns >> ns >> names;

      names >> list;
      names >> union_;
      names >> complex;
      names >> enumeration;

      schema.dispatch (ctx.schema_root);
    }
  }
}
