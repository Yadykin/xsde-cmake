// file      : processing/inheritance/processor.cxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <set>
#include <iostream>

#include <processing/inheritance/processor.hxx>

#include <xsd-frontend/semantic-graph.hxx>
#include <xsd-frontend/traversal.hxx>

#include <elements.hxx>

using namespace std;

namespace Processing
{
  namespace SemanticGraph = XSDFrontend::SemanticGraph;
  namespace Traversal = XSDFrontend::Traversal;

  namespace Inheritance
  {
    namespace
    {
      struct Dep
      {
        Dep (SemanticGraph::Type& t,
             SemanticGraph::Member* m = 0,
             String const& xpath = L"")
            : type (t), member (m), member_xpath (xpath)
        {
        }

        SemanticGraph::Type& type;
        SemanticGraph::Member* member; // Member if type is anonymous.
        String member_xpath;
      };

      inline bool
      operator< (Dep const& a, Dep const& b)
      {
        return &a.type < &b.type;
      }

      typedef set<Dep> DepSet;
      typedef set<SemanticGraph::Type*> TypeSet;


      String
      xpath (SemanticGraph::Nameable& n)
      {
        if (dynamic_cast<SemanticGraph::Namespace*> (&n) != 0)
          return L"<namespace-level>"; // There is a bug if you see this.

        if (n.named_p ())
        {
          SemanticGraph::Scope& scope (n.scope ());

          if (dynamic_cast<SemanticGraph::Namespace*> (&scope) != 0)
            return n.name ();

          return xpath (scope) + L"/" + n.name ();
        }
        else
        {
          return L"(anonymous type for " +
            n.context ().get<String> ("instance-name") + L")";
        }
      }


      // Calculate the list of dependencies for this type.
      //
      struct Type: Traversal::List,
                   Traversal::Complex,
                   Traversal::Member
      {
        Type (DepSet& dep_set, char const* by_value_key)
            : by_value_key_ (by_value_key), dep_set_ (dep_set), last_ (0)
        {
          *this >> names_ >> *this;
        }

        virtual void
        traverse (SemanticGraph::List& l)
        {
          SemanticGraph::Type& t (l.argumented ().type ());

          if (by_value_key_ != 0 &&
              t.context ().count (by_value_key_) &&
              t.context ().get<bool> (by_value_key_))
          {
            dep_set_.insert (Dep (t));
          }
        }

        virtual void
        traverse (SemanticGraph::Complex& c)
        {
          using SemanticGraph::Complex;

          if (c.inherits_p ())
            dep_set_.insert (Dep (c.inherits ().base (), last_, last_xpath_));

          types_seen_.insert (&c);

          // Go after anonymous types.
          //
          names (c);
        }

        virtual void
        traverse (SemanticGraph::Member& m)
        {
          SemanticGraph::Type& t (m.type ());

          if (!t.named_p ())
          {
            if (types_seen_.find (&t) == types_seen_.end ())
            {
              SemanticGraph::Context& ctx (t.context ());

              last_xpath_ = xpath (m);

              String prev_xpath;

              if (ctx.count ("instance-name"))
                prev_xpath = ctx.get<String> ("instance-name");

              ctx.set ("instance-name", last_xpath_);

              last_ = &m;
              dispatch (t);

              if (prev_xpath)
                ctx.set ("instance-name", prev_xpath);
              else
                ctx.remove ("instance-name");
            }
          }
          else
          {
            if (by_value_key_ != 0 &&
                t.context ().count (by_value_key_) &&
                t.context ().get<bool> (by_value_key_))
            {
              dep_set_.insert (Dep (t));
            }
          }
        }

      private:
        char const* by_value_key_;
        DepSet& dep_set_;
        TypeSet types_seen_;

        SemanticGraph::Member* last_;
        String last_xpath_;

        Traversal::Names names_;
      };


      //
      //
      template <typename N, typename A>
      struct NodeArgs
      {
        NodeArgs (N& node, A arg)
            : node_ (node), arg_ (arg)
        {
        }

        operator N& () const
        {
          return node_;
        }

        template <typename E>
        void
        add_edge_left (E& e)
        {
          node_.add_edge_left (e, arg_);
        }

        template <typename E>
        void
        add_edge_right (E& e)
        {
          node_.add_edge_right (e, arg_);
        }

      private:
        N& node_;
        A arg_;
      };


      //
      //
      struct Global: Traversal::Type,
                     Traversal::List,
                     Traversal::Complex,
                     Traversal::Element
      {
        Global (SemanticGraph::Schema& root,
                SemanticGraph::Schema& schema,
                char const* by_value_key,
                bool& failed)
            : root_ (root),
              schema_ (schema),
              by_value_key_ (by_value_key),
              failed_ (failed)
        {
        }

        virtual void
        traverse (SemanticGraph::Type& t)
        {
          if (t.named_p ())
            types_seen_.insert (&t);
        }

        virtual void
        traverse (SemanticGraph::List& l)
        {
          check_dep (l, l);
          types_seen_.insert (&l);
        };

        virtual void
        traverse (SemanticGraph::Complex& c)
        {
          check_dep (c, c);
          types_seen_.insert (&c);
        };

        virtual void
        traverse (SemanticGraph::Element& e)
        {
          SemanticGraph::Type& t (e.type ());

          if (!t.named_p ())
          {
            t.context ().set ("instance-name", xpath (e));
            check_dep (e, t);
            t.context ().remove ("instance-name");
          }
        };

      private:
        void
        check_dep (SemanticGraph::Nameable& global,
                   SemanticGraph::Type& type)
        {
          using SemanticGraph::Scope;
          using SemanticGraph::Names;
          using SemanticGraph::Schema;

          DepSet prereqs;

          // Calculate our prerequisistes.
          //
          {
            Inheritance::Type t (prereqs, by_value_key_);
            t.dispatch (type);
          }

          for (DepSet::const_iterator i (prereqs.begin ());
               i != prereqs.end (); ++i)
          {
            Dep const& dep (*i);
            SemanticGraph::Type& t (dep.type);

            // We won't be able to generate compilable code in case of a
            // dependency on ourselves (e.g., a member element with
            // anonymous type that inherits from us).
            //
            if (&t == &type)
            {
              assert (dep.member != 0);

              SemanticGraph::Member& m (*dep.member);

              wcerr << t.file () << ":" << t.line () << ":" << t.column ()
                    << " error: nested anonymous type for '"
                    << dep.member_xpath << "' cyclicly inherits from '"
                    << t.name () << "'" << endl;

              wcerr << t.file () << ":" << t.line () << ":" << t.column ()
                    << " error: unable to generate valid code for such "
                    << "cyclic inheritance" << endl;

              wcerr << m.file () << ":" << m.line () << ":" << m.column ()
                    << " info: '" << m.name () << "' element is declared here"
                    << endl;

              wcerr << t.file () << ":" << t.line () << ":" << t.column ()
                    << ": info: consider explicitly naming this type "
                    << "or remove the --preserve-anonymous option"
                    << endl;

              failed_ = true;
              continue;
            }

            if (types_seen_.find (&t) == types_seen_.end ())
            {
              Scope& scope (t.scope ());
              Schema& schema (dynamic_cast<Schema&> (scope.scope ()));

              // Don't worry about types that are in included/imported
              // schemas.
              //
              if (&schema != &schema_ && !sources_p (schema_, schema))
                continue;

              if (t.context ().count ("seen"))
              {
                wcerr << t.file () << ":" << t.line () << ":" << t.column ()
                      << " error: nested anonymous type in '" << t.name ()
                      << "' or '" << type.name () << "' inherits from one of "
                      << "these types and makes them mutually dependant"
                      << endl;

                wcerr << t.file () << ":" << t.line () << ":" << t.column ()
                      << " error: unable to generate valid code for such "
                      << "cyclic dependency" << endl;

                wcerr << type.file () << ":" << type.line () << ":"
                      << type.column () << " info: '" << type.name ()
                      << "' type is defined here"
                      << endl;

                wcerr << t.file () << ":" << t.line () << ":" << t.column ()
                      << ": info: consider explicitly naming the anonymous "
                      << "type or remove the --preserve-anonymous option"
                      << endl;

                failed_ = true;
                continue;
              }

              //wcerr << "type '" << t.name () << "' needs to be moved " <<
              //  "before " << (global.is_a<SG::Type> () ? "type" : "element")
              // << " '" << global.name () << "'" << endl;


              // Delete current Names edge.
              //
              String name (t.name ());
              {
                Names& n (t.named ());
                root_.delete_edge (scope, t, n);
              }

              // Insert a new Names edge before global.
              //
              {
                // t.scope () and global.scope () can be different in
                // case of the chameleon inclusion.
                //
                Scope& scope (global.scope ());

                // Convert to the insert-after call.
                //
                Scope::NamesIterator i (scope.find (global.named ()));

                if (i == scope.names_begin ())
                  i = scope.names_end ();
                else
                  --i;

                NodeArgs<Scope, Scope::NamesIterator> na (scope, i);
                root_.new_edge<Names> (na, t, name);
              }

              // Recursively process the moved type.
              //
              global.context ().set ("seen", true);
              dispatch (t);
              global.context ().remove ("seen");
            }
          }
        }

      private:
        // Return true if root sources s.
        //
        bool
        sources_p (SemanticGraph::Schema& root, SemanticGraph::Schema& s)
        {
          using SemanticGraph::Schema;
          using SemanticGraph::Sources;

          for (Schema::UsesIterator i (root.uses_begin ());
               i != root.uses_end (); ++i)
          {
            if (i->is_a<Sources> ())
            {
              if (&i->schema () == &s || sources_p (i->schema (), s))
                return true;
            }
          }

          return false;
        }

      private:
        SemanticGraph::Schema& root_;
        SemanticGraph::Schema& schema_;
        char const* by_value_key_;
        TypeSet types_seen_;
        bool& failed_;
      };


      // Sources traverser that goes into each schema only once.
      //
      struct Sources: Traversal::Sources
      {
        virtual void
        traverse (SemanticGraph::Sources& s)
        {
          if (schemas_.insert (&s.schema ()).second)
            Traversal::Sources::traverse (s);
        }

      private:
        set<SemanticGraph::Schema*> schemas_;
      };

      // Go into included/imported schemas while making sure we don't
      // process the same stuff more than once.
      //
      struct Uses: Traversal::Includes, Traversal::Imports
      {
        Uses (SemanticGraph::Schema& root,
              char const* by_value_key,
              bool& failed)
            : root_ (root), by_value_key_ (by_value_key), failed_ (failed)
        {
        }

        virtual void
        traverse (SemanticGraph::Includes& i)
        {
          traverse (i.schema ());
        }

        virtual void
        traverse (SemanticGraph::Imports& i)
        {
          traverse (i.schema ());
        }

      private:
        void
        traverse (SemanticGraph::Schema& s)
        {
          if (!s.context ().count ("processing-inheritance-seen"))
          {
            Traversal::Schema schema;
            Sources sources;

            schema >> sources >> schema;
            schema >> *this;

            Traversal::Names schema_names;
            Traversal::Namespace ns;
            Traversal::Names ns_names;

            schema >> schema_names >> ns >> ns_names;

            Global global (root_, s, by_value_key_, failed_);

            ns_names >> global;

            s.context ().set ("processing-inheritance-seen", true);
            schema.dispatch (s);
          }
        }

      private:
        SemanticGraph::Schema& root_;
        char const* by_value_key_;
        bool& failed_;
      };
    }

    void Processor::
    process (SemanticGraph::Schema& tu,
             SemanticGraph::Path const&,
             char const* by_value_key)
    {
      bool failed (false);

      // We need to process include/imported schemas since other
      // parts of the process, for example, name processors can
      // rely on the order of types in the schema.
      //
      Traversal::Schema schema;
      Sources sources;
      Uses uses (tu, by_value_key, failed);

      schema >> sources >> schema;
      schema >> uses;

      Traversal::Names schema_names;
      Traversal::Namespace ns;
      Traversal::Names ns_names;

      schema >> schema_names >> ns >> ns_names;

      Global global (tu, tu, by_value_key, failed);

      ns_names >> global;

      // Some twisted schemas do recusive self-inclusion.
      //
      tu.context ().set ("processing-inheritance-seen", true);

      schema.dispatch (tu);

      if (failed)
        throw Failed ();
    }
  }
}
