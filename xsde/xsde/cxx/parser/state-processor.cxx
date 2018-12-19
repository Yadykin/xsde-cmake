// file      : xsde/cxx/parser/state-processor.cxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <vector>
#include <iostream>

#include <cxx/parser/state-processor.hxx>

#include <cxx/parser/elements.hxx>

#include <xsd-frontend/semantic-graph.hxx>
#include <xsd-frontend/traversal.hxx>

using namespace std;

namespace CXX
{
  namespace Parser
  {
    namespace
    {
      typedef vector<SemanticGraph::Particle*> Particles;

      void
      print (Particles const& p)
      {
        wcerr << "prefixes: " << endl;

        for (Particles::const_iterator i (p.begin ()); i != p.end (); ++i)
        {
          if (SemanticGraph::Element* e =
              dynamic_cast<SemanticGraph::Element*> (*i))
          {
            wcerr << e->name () << endl;
          }
          else
          {
            wcerr << "<any>" << endl;
          }
        }

        wcerr << endl;
      }

      //
      //
      struct Particle: Traversal::All,
                       Traversal::Choice,
                       Traversal::Sequence
      {
        Particle (size_t& all,
                  size_t& choice,
                  size_t& sequence,
                  size_t& depth)
            : all_ (all),
              choice_ (choice),
              sequence_ (sequence),
              depth_ (depth)
        {
        }

        virtual void
        traverse (SemanticGraph::All& a)
        {
          using SemanticGraph::Compositor;

          // Go over particles, collecting "prefix" particles in prefixes_,
          // assigning state numbers and calculating effective minOccurs.
          // If all prefixes of this compositor have minOccurs = 0, then
          // the compositor itself effectively has minOccurs = 0 regardless
          // of the actual value specified in the schema.
          //
          // Note that we don't need to care about depth since the 'all'
          // compositor cannot contain any nested compositors.
          //

          size_t state (0);
          size_t min (0);

          for (Compositor::ContainsIterator ci (a.contains_begin ());
               ci != a.contains_end (); ++ci)
          {
            SemanticGraph::Particle& p (ci->particle ());

            // The 'all' compositor can only include elements.
            //
            prefixes_.push_back (&p);

            if (min == 0 && ci->min () != 0)
              min = 1;

            p.context ().set ("p:prefix", true);
            p.context ().set ("p:state", state++);
          }

          if (!prefixes_.empty ())
          {
            a.context ().set ("p:comp-number", choice_++);
            a.context ().set ("p:prefixes", prefixes_);
            a.context ().set ("p:state-count", size_t (prefixes_.size ()));

            // effective-min = min * actual-min
            //
            if (min == 1)
              min = a.min ();

            a.context ().set ("p:effective-min", min);

            // print (prefixes_);
          }
        }

        virtual void
        traverse (SemanticGraph::Choice& c)
        {
          using SemanticGraph::Compositor;

          // Go over particles, collecting "prefix" particles in prefixes_,
          // assigning state numbers and calculating effective minOccurs.
          // If any prefix of this compositor have minOccurs = 0, then the
          // compositor itself effectively has minOccurs = 0 regardless of
          // the actual value specified in the schema.
          //

          size_t state (0);
          size_t min (1);

          for (Compositor::ContainsIterator ci (c.contains_begin ());
               ci != c.contains_end (); ++ci)
          {
            SemanticGraph::Particle& p (ci->particle ());

            if (p.is_a<SemanticGraph::Element> () ||
                p.is_a<SemanticGraph::Any> ())
            {
              prefixes_.push_back (&p);

              if (min == 1 && ci->min () == 0)
                min = 0;
            }
            else
            {
              size_t depth (0);
              Particle t (all_, choice_, sequence_, depth);
              t.dispatch (p);

              if (t.prefixes_.empty ())
                continue; // Skip empty compositors.

              if (++depth > depth_) // One for this compositor.
                depth_ = depth;

              prefixes_.insert (prefixes_.end (),
                                t.prefixes_.begin ().base (),
                                t.prefixes_.end ().base ());

              if (min == 1 &&
                  p.context ().get<size_t> ("p:effective-min") == 0)
                min = 0;
            }

            p.context ().set ("p:prefix", true);
            p.context ().set ("p:state", state++);
          }

          if (!prefixes_.empty ())
          {
            c.context ().set ("p:comp-number", choice_++);
            c.context ().set ("p:prefixes", prefixes_);

            // effective-min = min * actual-min
            //
            if (min == 1)
              min = c.min ();

            c.context ().set ("p:effective-min", min);

            // print (prefixes_);
          }
        }

        virtual void
        traverse (SemanticGraph::Sequence& s)
        {
          using SemanticGraph::Compositor;

          // Go over particles, collecting "prefix" particles in prefixes_,
          // assigning state numbers and calculating effective minOccurs.
          // If all prefixes of this compositor have minOccurs = 0, then
          // the compositor itself effectively has minOccurs = 0 regardless
          // of the actual value specified in the schema.
          //

          bool prefix (true);
          size_t state (0);
          size_t min (0);

          for (Compositor::ContainsIterator ci (s.contains_begin ());
               ci != s.contains_end (); ++ci)
          {
            SemanticGraph::Particle& p (ci->particle ());

            if (p.is_a<SemanticGraph::Element> () ||
                p.is_a<SemanticGraph::Any> ())
            {
              if (prefix)
              {
                prefixes_.push_back (&p);

                if (ci->min () != 0)
                  min = 1;
              }
            }
            else
            {
              size_t depth (0);
              Particle t (all_, choice_, sequence_, depth);
              t.dispatch (p);

              if (t.prefixes_.empty ())
                continue; // Skip empty compositors.

              if (++depth > depth_) // One for this compositor.
                depth_ = depth;

              if (prefix)
              {
                prefixes_.insert (prefixes_.end (),
                                  t.prefixes_.begin ().base (),
                                  t.prefixes_.end ().base ());

                if (p.context ().get<size_t> ("p:effective-min") != 0)
                  min = 1;
              }
            }

            p.context ().set ("p:state", state++);

            if (prefix)
              p.context ().set ("p:prefix", true);

            if (prefix && min != 0)
              prefix = false;
          }

          if (!prefixes_.empty ())
          {
            s.context ().set ("p:comp-number", sequence_++);
            s.context ().set ("p:prefixes", prefixes_);

            // effective-min = min * actual-min
            //
            if (min == 1)
              min = s.min ();

            s.context ().set ("p:effective-min", min);

            // print (prefixes_);
          }
        }

      private:
        Particles prefixes_;

        size_t& all_;
        size_t& choice_;
        size_t& sequence_;

        size_t& depth_;
      };


      //
      //
      struct Complex: Traversal::Complex
      {
        virtual void
        traverse (Type& c)
        {
          if (c.contains_compositor_p ())
          {
            size_t all (0), choice (0), sequence (0), depth (0);
            Particle t (all, choice, sequence, depth);
            t.dispatch (c.contains_compositor ().compositor ());

            // Set the maximum stack depth for this type. Used to
            // allocate fixed-size state stack.
            //
            c.context ().set ("p:depth", depth + 1);
          }
        }
      };
    }

    void StateProcessor::
    process (SemanticGraph::Schema& tu, SemanticGraph::Path const&)
    {
      Traversal::Schema schema;
      Sources sources;
      Traversal::Names schema_names;
      Traversal::Namespace ns;
      Traversal::Names ns_names;

      schema >> sources >> schema;
      schema >> schema_names >> ns >> ns_names;

      Complex complex_type;

      ns_names >> complex_type;

      schema.dispatch (tu);
    }
  }
}
