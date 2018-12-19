// file      : xsd-frontend/transformations/schema-per-type.cxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <strings.h> // strcasecmp

#include <map>
#include <set>
#include <vector>

#include <sstream>
#include <iostream>

#include <xsd-frontend/semantic-graph.hxx>
#include <xsd-frontend/traversal.hxx>

#include <xsd-frontend/transformations/schema-per-type.hxx>

using std::wcerr;
using std::endl;

namespace XSDFrontend
{
  typedef Transformations::SchemaPerType::Failed Failed;
  typedef std::vector<SemanticGraph::Schema*> Schemas;
  typedef std::map<SemanticGraph::Type*, SemanticGraph::Schema*> TypeSchemaMap;

  // Compare file paths case-insensitively.
  //
  struct FileComparator
  {
    bool
    operator() (NarrowString const& x, NarrowString const& y) const
    {
      return strcasecmp (x.c_str (), y.c_str ()) < 0;
    }
  };

  typedef std::set<NarrowString, FileComparator> FileSet;

  namespace
  {
    // Go into included and imported schemas while making sure
    // we don't process the same stuff more than once.
    //
    struct Uses: Traversal::Includes,
                 Traversal::Imports,
                 Traversal::Implies
    {
      Uses (Schemas& schemas, SemanticGraph::Schema*& xsd)
          : schemas_ (schemas), xsd_ (xsd)
      {
        xsd_ = 0;
      }

      virtual void
      traverse (SemanticGraph::Includes& i)
      {
        SemanticGraph::Schema& s (i.schema ());

        if (!s.context ().count ("xsd-frontend-schema-per-type-seen"))
        {
          schemas_.push_back (&s);
          s.context ().set ("xsd-frontend-schema-per-type-seen", true);
          Traversal::Includes::traverse (i);
        }
      }

      virtual void
      traverse (SemanticGraph::Imports& i)
      {
        SemanticGraph::Schema& s (i.schema ());

        if (!s.context ().count ("xsd-frontend-schema-per-type-seen"))
        {
          schemas_.push_back (&s);
          s.context ().set ("xsd-frontend-schema-per-type-seen", true);
          Traversal::Imports::traverse (i);
        }
      }

      virtual void
      traverse (SemanticGraph::Implies& i)
      {
        if (xsd_ == 0)
          xsd_ = &i.schema ();
      }

    private:
      Schemas& schemas_;
      SemanticGraph::Schema*& xsd_;
    };

    void
    process_schema (SemanticGraph::Schema& s,
                    SemanticGraph::Schema& root,
                    SemanticGraph::Schema& xsd,
                    TypeSchemaMap& tsm,
                    FileSet& file_set,
                    bool fat_type_file,
                    Transformations::SchemaPerTypeTranslator& trans)
    {
      using namespace SemanticGraph;

      Path xsd_path ("XMLSchema.xsd");
      Namespace& ns (dynamic_cast<Namespace&> (s.names_begin ()->named ()));

      // We should be careful with iterator caching since we are going to
      // remove some of the nodes.
      //
      for (Scope::NamesIterator i (ns.names_begin ()); i != ns.names_end ();)
      {
        Nameable& n (i->named ());

        if (n.is_a<Type> ())
        {
          String name (n.name ());

          // Remove from the namespace.
          //
          Scope::NamesIterator tmp (i++);
          root.delete_edge (ns, n, *tmp);

          // Add a new schema node.
          //
          Path path;
          String tn (trans.translate_type (ns.name (), name));
          String wbase (tn ? tn : name);

          try
          {
            NarrowString base (wbase.to_narrow ());

            // Escape directory separators unless they came from the
            // translator.
            //
            if (!tn)
            {
              for (NarrowString::iterator i (base.begin ()), e (base.end ());
                   i != e; ++i)
              {
                if (*i == '/' || *i == '\\')
                  *i = '_';
              }
            }

            // Make sure it is unique.
            //
            NarrowString file_name (base);

            for (unsigned long i (1);
                 file_set.find (file_name) != file_set.end ();
                 ++i)
            {
              std::ostringstream os;
              os << i;
              file_name = base + os.str ();
            }

            file_set.insert (file_name);
            file_name += ".xsd";

            try
            {
              path = Path (file_name);
            }
            catch (InvalidPath const&)
            {
              wcerr << "error: '" << file_name.c_str () << "' is not a valid "
                    << "filesystem path" << endl;

              wcerr << "info: use type to file name translation mechanism "
                    << "to resolve this" << endl;

              throw Failed ();
            }
          }
          catch (NonRepresentable const&)
          {
            wcerr << "error: '" << wbase << "' cannot be represented as a "
                  << "narrow string" << endl;

            wcerr << "info: use type to file name translation mechanism "
                  << "to resolve this" << endl;

            throw Failed ();
          }

          Type& t (dynamic_cast<Type&> (n));

          Schema& ts (root.new_node<Schema> (path, 1, 1));
          root.new_edge<Implies> (ts, xsd, xsd_path);

          Namespace& tns (root.new_node<Namespace> (path, 1, 1));
          root.new_edge<Names> (ts, tns, ns.name ());
          root.new_edge<Names> (tns, n, name);

          // If we are generating fat type files, then also move the global
          // elements this type classifies to the new schema.
          //
          if (fat_type_file)
          {
            for (Type::ClassifiesIterator j (t.classifies_begin ());
                 j != t.classifies_end (); ++j)
            {
              Instance& e (j->instance ());

              // We can only move a global element from the same namespace.
              //
              if (e.is_a<Element> () &&
                  e.scope ().is_a<Namespace> () &&
                  e.scope ().name () == ns.name ())
              {
                Names& n (e.named ());
                String name (n.name ());

                // Watch out for the iterator validity: the edge we are
                // about to remove can be from the same list we are
                // currently iterating.
                //
                if (i != ns.names_end () && &*i == &n)
                  ++i;

                root.delete_edge (n.scope (), e, n);
                root.new_edge<Names> (tns, e, name);
              }
            }
          }

          // Add include to the original schema and enter into the
          // type-schema map.
          //
          root.new_edge<Includes> (s, ts, path);
          tsm[&t] = &ts;

          // Also mark this schema as "type schema" in case someone
          // needs to distinguish between the two kinds.
          //
          ts.context ().set ("type-schema", true);
        }
        else
          ++i;
      }
    }

    struct Type: Traversal::List,
                 Traversal::Complex,
                 Traversal::Member
    {
      Type (SemanticGraph::Schema& schema,
            SemanticGraph::Schema& root,
            char const* by_value_key,
            TypeSchemaMap& tsm)
          : schema_ (schema),
            root_ (root),
            by_value_key_ (by_value_key),
            tsm_ (tsm)
      {
        *this >> names_ >> *this;
      }

      virtual void
      traverse (SemanticGraph::List& l)
      {
        // Treat item type as base type since it is impossible
        // to create recursive constructs using list.
        //
        SemanticGraph::Type& t (l.argumented ().type ());
        set_dep (t, false);
      }

      virtual void
      traverse (SemanticGraph::Complex& c)
      {
        if (c.inherits_p ())
          set_dep (c.inherits ().base (), false);

        Traversal::Complex::names (c);
      }

      virtual void
      traverse (SemanticGraph::Member& m)
      {
        SemanticGraph::Type& t (m.type ());

        bool weak (
          by_value_key_ == 0 ||
          !t.context ().count (by_value_key_) ||
          !t.context ().get<bool> (by_value_key_));

        set_dep (t, weak);
      }

    private:
      void
      set_dep (SemanticGraph::Type& t, bool weak)
      {
        using namespace SemanticGraph;

        TypeSchemaMap::iterator i (tsm_.find (&t));

        // If a type is not present in the map then it must be
        // a built-in type.
        //
        if (i == tsm_.end ())
          return;

        // Check if we already saw this type. Theoretically, it could
        // be that we need to upgrade the type of include from weak to
        // strong. But because inheritance is handled first, the type
        // in the set will already be with the right type.
        //
        if (type_set_.find (&t) != type_set_.end ())
          return;

        type_set_.insert (&t);

        Schema& s (*i->second);
        Path path (s.used_begin ()->path ());
        SemanticGraph::Uses* u;

        if (s.names_begin ()->name () == schema_.names_begin ()->name ())
          u = &root_.new_edge<Includes> (schema_, s, path);
        else
          u = &root_.new_edge<Imports> (schema_, s, path);

        if (weak)
          u->context().set ("weak", true);
      }

    private:
      SemanticGraph::Schema& schema_;
      SemanticGraph::Schema& root_;
      char const* by_value_key_;
      TypeSchemaMap& tsm_;
      std::set<SemanticGraph::Type*> type_set_;

      Traversal::Names names_;
    };
  }

  namespace Transformations
  {
    SchemaPerType::
    SchemaPerType (SchemaPerTypeTranslator& trans,
                   bool fat,
                   char const* key)
        : fat_type_file_ (fat), by_value_key_ (key), trans_ (trans)
    {
    }

    Schemas SchemaPerType::
    transform (SemanticGraph::Schema& root)
    {
      // Collect initial schema nodes.
      //
      Schemas schemas;
      SemanticGraph::Schema* xsd;

      {
        Traversal::Schema schema;
        Uses uses (schemas, xsd);

        schema >> uses >> schema;

        // Some twisted schemas do recusive inclusions.
        //
        root.context ().set ("xsd-frontend-schema-per-type-seen", true);

        schema.dispatch (root);
      }

      // wcerr << schemas.size () << " initial schema nodes" << endl;

      // Add the schema file names to the file set.
      //
      FileSet file_set;

      for (Schemas::iterator i (schemas.begin ()); i != schemas.end (); ++i)
      {
        // This path was already normalized by the parser.
        //
        SemanticGraph::Path const& path (
          (*i)->context ().get<SemanticGraph::Path> ("absolute-path"));

        // Translate the schema file name.
        //
        NarrowString abs_path;

        // Try to use the portable representation of the path. If that
        // fails, fall back to the native representation.
        //
        try
        {
          abs_path = path.posix_string ();
        }
        catch (SemanticGraph::InvalidPath const&)
        {
          abs_path = path.string ();
        }

        NarrowString tf (trans_.translate_schema (abs_path));
        NarrowString file (tf ? tf : path.leaf ().string ());

        size_t p (file.rfind ('.'));
        NarrowString ext (
          p != NarrowString::npos ? NarrowString (file, p) : "");

        NarrowString base (
          p != NarrowString::npos ? NarrowString (file, 0, p) : file);

        // Make sure it is unique.
        //
        NarrowString new_name (base);

        for (unsigned long n (1);
             file_set.find (new_name) != file_set.end ();
             ++n)
        {
          std::ostringstream os;
          os << n;
          new_name = base + os.str ();
        }

        file_set.insert (new_name);
        new_name += ext;

        try
        {
          (*i)->context ().set ("renamed", SemanticGraph::Path (new_name));
        }
        catch (SemanticGraph::InvalidPath const&)
        {
          wcerr << "error: '" << new_name.c_str () << "' is not a valid "
                << "filesystem path" << endl;

          wcerr << "info: use schema file name translation mechanism "
                << "to resolve this" << endl;

          throw Failed ();
        }
      }

      // Process each schema node.
      //
      TypeSchemaMap tsm;

      for (Schemas::iterator i (schemas.begin ()); i != schemas.end (); ++i)
      {
        process_schema (**i, root, *xsd, tsm, file_set, fat_type_file_, trans_);
      }

      // wcerr << tsm.size () << " type schema nodes" << endl;

      // Establish include/import dependencies. While at it add the
      // new schemas to the list which we will return.
      //
      for (TypeSchemaMap::iterator i (tsm.begin ()); i != tsm.end (); ++i)
      {
        SemanticGraph::Schema& s (*i->second);
        Type t (s, root, by_value_key_, tsm);
        t.dispatch (*i->first);
        schemas.push_back (&s);
      }

      return schemas;
    }

    SchemaPerTypeTranslator::
    ~SchemaPerTypeTranslator ()
    {
    }
  }
}
