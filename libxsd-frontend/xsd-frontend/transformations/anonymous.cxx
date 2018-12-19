// file      : xsd-frontend/transformations/anonymous.cxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsd-frontend/transformations/anonymous.hxx>

#include <xsd-frontend/semantic-graph.hxx>
#include <xsd-frontend/traversal.hxx>

#include <iostream>
#include <sstream>
#include <typeinfo>

using std::wcerr;
using std::endl;

namespace XSDFrontend
{
  namespace
  {
    using Transformations::AnonymousNameTranslator;

    //
    //
    struct CompareMembers: Traversal::Element,
                           Traversal::Attribute,
                           Traversal::Any,
                           Traversal::AnyAttribute
    {
      CompareMembers (SemanticGraph::Nameable& m, bool& r)
          : member_ (m), result_ (r)
      {
      }

      virtual void
      traverse (SemanticGraph::Element& x)
      {
        using SemanticGraph::Element;

        Element& y (dynamic_cast<Element&> (member_));

        // Check cardinalities.
        //
        if (x.min () != y.min () || x.max () != y.max ())
          return;

        traverse_member (x);
      }

      virtual void
      traverse (SemanticGraph::Attribute& x)
      {
        using SemanticGraph::Attribute;

        Attribute& y (dynamic_cast<Attribute&> (member_));

        // Check cardinalities.
        //
        if (x.optional_p () != y.optional_p ())
          return;

        traverse_member (x);
      }

      virtual void
      traverse_member (SemanticGraph::Member& x)
      {
        using SemanticGraph::Member;

        Member& y (dynamic_cast<Member&> (member_));

        // Check name.
        //
        if (x.name () != y.name ())
          return;

        // Check namespace.
        //
        if (x.qualified_p () || y.qualified_p ())
        {
          if (!x.qualified_p () || !y.qualified_p ())
            return;

          if (x.namespace_ ().name () != y.namespace_ ().name ())
            return;
        }

        // Check type.
        //
        // @@ What if types are anonymous and structurally equal?
        //
        if (&x.type () != &y.type ())
          return;

        // Check default/fixed values.
        //
        if (x.default_p () != y.default_p () || x.fixed_p () != y.fixed_p ())
          return;

        if (x.default_p () && x.value () != y.value ())
          return;

        result_ = true;
      }

      virtual void
      traverse (SemanticGraph::Any&)
      {
        //@@ TODO
      }

      virtual void
      traverse (SemanticGraph::AnyAttribute&)
      {
        //@@ TODO
      }

    private:
      SemanticGraph::Nameable& member_;
      bool& result_;
    };

    // Compare two types for structural equality.
    //
    struct CompareTypes: Traversal::List,
                         Traversal::Union,
                         Traversal::Enumeration,
                         Traversal::Complex
    {
      CompareTypes (SemanticGraph::Type& t, bool& r)
          : type_ (t), result_ (r)
      {
      }


      virtual void
      traverse (SemanticGraph::List&)
      {
        using SemanticGraph::List;

        //List& y (dynamic_cast<List&> (type_));
      }

      virtual void
      traverse (SemanticGraph::Union& x)
      {
        using SemanticGraph::Union;

        Union& y (dynamic_cast<Union&> (type_));

        Union::ArgumentedIterator ix (x.argumented_begin ()),
          iy (y.argumented_begin ());

        for (; ix != x.argumented_end () && iy != y.argumented_end ();
             ++ix, ++iy)
        {
          // @@ Anon structurally equivalent.
          //
          if (&iy->type () != &ix->type ())
            return;
        }

        result_ = true;
      }

      virtual void
      traverse (SemanticGraph::Enumeration& x)
      {
        using SemanticGraph::Enumeration;

        Enumeration& y (dynamic_cast<Enumeration&> (type_));

        // Bases should be the same.
        //
        if (&x.inherits ().base () != &y.inherits ().base ())
          return;

        // Make sure facets match.
        //
        using SemanticGraph::Restricts;

        Restricts& rx (dynamic_cast<Restricts&> (x.inherits ()));
        Restricts& ry (dynamic_cast<Restricts&> (y.inherits ()));

        if (rx.facets () != ry.facets ())
          return;

        // Compare enumerators.
        //
        using SemanticGraph::Scope;

        Scope::NamesIterator ix (x.names_begin ()), iy (y.names_begin ());
        for (; ix != x.names_end () && iy != y.names_end (); ++ix, ++iy)
        {
          if (ix->name () != iy->name ())
            return;
        }

        if (ix != x.names_end () || iy != y.names_end ())
          return;

        result_ = true;
      }

      virtual void
      traverse (SemanticGraph::Complex& x)
      {
        using SemanticGraph::Complex;

        Complex& y (dynamic_cast<Complex&> (type_));

        // Check inheritance.
        //
        if (x.inherits_p () || y.inherits_p ())
        {
          // They both must inherits.
          //
          if (!x.inherits_p () || !y.inherits_p ())
            return;

          // With the same kind of inheritance (restriction or extension).
          //
          if (typeid (x.inherits ()) != typeid (y.inherits ()))
            return;

          // Bases should be the same.
          //
          // @@ What if bases are anonymous?
          //
          if (&x.inherits ().base () != &y.inherits ().base ())
            return;

          // If it is a restriction, make sure facets match.
          //
          using SemanticGraph::Restricts;

          if (x.inherits ().is_a<Restricts> ())
          {
            Restricts& rx (dynamic_cast<Restricts&> (x.inherits ()));
            Restricts& ry (dynamic_cast<Restricts&> (y.inherits ()));

            if (rx.facets () != ry.facets ())
              return;
          }
        }

        // Check the member list.
        //
        // @@ Ignoring compositors at the moment.
        //
        using SemanticGraph::Scope;

        Scope::NamesIterator ix (x.names_begin ()), iy (y.names_begin ());
        for (; ix != x.names_end () && iy != y.names_end (); ++ix, ++iy)
        {
          if (typeid (ix->named ()) != typeid (iy->named ()))
            return;

          bool equal (false);
          CompareMembers t (iy->named (), equal);
          t.dispatch (ix->named ());

          if (!equal)
            return;
        }

        if (ix != x.names_end () || iy != y.names_end ())
          return;

        result_ = true;
      }

    private:
      SemanticGraph::Type& type_;
      bool& result_;
    };

    //
    //
    class Context
    {
    public:
      Context (SemanticGraph::Schema& schema_,
               SemanticGraph::Path const& file,
               AnonymousNameTranslator& trans_,
               bool du)
          : schema_path_ (file),
            ns_ (0),
            failed_ (false),
            trans (trans_),
            detect_unstable (du),
            schema (schema_),
            schema_path (schema_path_),
            ns (ns_),
            failed (failed_)
      {

      }

    protected:
      Context (Context& c)
          : trans (c.trans),
            detect_unstable (c.detect_unstable),
            schema (c.schema),
            schema_path (c.schema_path),
            ns (c.ns),
            failed (c.failed)
      {
      }

    public:

      bool
      structurally_equal (SemanticGraph::Type& x, SemanticGraph::Type& y)
      {
        if (typeid (x) != typeid (y))
           return false;

        bool r (false);
        CompareTypes t (y, r);
        t.dispatch (x);
        return r;
      }

      struct UnstableConflict
      {
        UnstableConflict (SemanticGraph::Type& type)
            : type_ (type)
        {
        }

        SemanticGraph::Type&
        type () const
        {
          return type_;
        }

      private:
        SemanticGraph::Type& type_;
      };

      SemanticGraph::Type*
      conflict (String const& name)
      {
        using SemanticGraph::Type;
        using SemanticGraph::Schema;

        if (Type* t1 = find (schema, name))
        {
          // Check if this is a stable conflict. A conflict is unstable
          // if a conflicting type is visible from the root schema but
          // is not visible from the schema where the conflicting
          // element is defined.
          //
          if (detect_unstable)
          {
            Schema& s (dynamic_cast<Schema&> (ns->scope ()));

            Type* t2 (find (s, name));

            if (t1 != t2)
              throw UnstableConflict (*t1);
          }

          return t1;
        }

        return 0;
      }

      SemanticGraph::Type*
      find (SemanticGraph::Schema& schema, String const& name)
      {
        using SemanticGraph::Type;
        using SemanticGraph::Scope;
        using SemanticGraph::Namespace;

        String ns_name (ns->name ());

        // Get all namespaces across include/import hierarchy with
        // our namespace name.
        //
        Scope::NamesIteratorPair nip (schema.find (ns_name));

        for (; nip.first != nip.second; ++nip.first)
        {
          Namespace& ns (dynamic_cast<Namespace&> (nip.first->named ()));

          Scope::NamesIteratorPair types (ns.find (name));

          for (; types.first != types.second; ++types.first)
          {
            if (Type* t = dynamic_cast<Type*> (&types.first->named ()))
            {
              return t;
            }
          }
        }

        return 0;
      }

    public:
      SemanticGraph::Path
      path (SemanticGraph::Nameable& n)
      {
        using SemanticGraph::Scope;
        using SemanticGraph::Schema;
        using SemanticGraph::Uses;

        Schema* schema (0);

        for (Scope* s (dynamic_cast<Scope*> (&n)
                       ? dynamic_cast<Scope*> (&n) : &n.scope ());;
             s = &s->scope ())
        {
          if ((schema = dynamic_cast<Schema*> (s)))
            break;
        }

        if (!schema->used_p ())
          return schema_path;

        Uses& u (*schema->used_begin ());
        return u.path ();
      }

    public:
      String
      xpath (SemanticGraph::Nameable& n)
      {
        if (dynamic_cast<SemanticGraph::Namespace*> (&n) != 0)
          return L"<namespace-level>"; // There is a bug if you see this.

        assert (n.named_p ());

        SemanticGraph::Scope& scope (n.scope ());

        if (dynamic_cast<SemanticGraph::Namespace*> (&scope) != 0)
          return n.name ();

        return xpath (scope) + L"/" + n.name ();
      }

    private:
      SemanticGraph::Path const schema_path_;
      SemanticGraph::Namespace* ns_;
      bool failed_;

    public:
      AnonymousNameTranslator& trans;
      bool detect_unstable;

    public:
      SemanticGraph::Schema& schema;
      SemanticGraph::Path const& schema_path;
      SemanticGraph::Namespace*& ns;
      bool& failed;
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

        if (!s.context ().count ("xsd-frontend-anonymous-seen"))
        {
          s.context ().set ("xsd-frontend-anonymous-seen", true);
          Traversal::Uses::traverse (u);
        }
      }
    };

    // Keep track which namespace we are in.
    //
    struct Namespace: Traversal::Namespace
    {
      Namespace (SemanticGraph::Namespace*& ns)
          : ns_ (ns)
      {
      }

      void
      pre (SemanticGraph::Namespace& ns)
      {
        ns_ = &ns;
      }

      void
      post (SemanticGraph::Namespace&)
      {
        ns_ = 0;
      }

    private:
      SemanticGraph::Namespace*& ns_;
    };

    //
    //
    struct Type: Traversal::List,
                 Traversal::Union,
                 Traversal::Complex,
                 protected virtual Context
    {
      Type (Context& c)
          : Context (c)
      {
      }

      virtual void
      traverse (SemanticGraph::List& l)
      {
        SemanticGraph::Type& t (l.argumented ().type ());

        //@@ This IDREF stuff is really ugly!
        //
        if (!t.named_p () &&
            !t.is_a<SemanticGraph::Fundamental::IdRef> () &&
            !t.is_a<SemanticGraph::Fundamental::IdRefs> ())
        {
          try
          {
            // Run the name through the translation service.
            //
            SemanticGraph::Path file (path (l));
            file.normalize ();
            String file_str;

            // Try to use the portable representation of the path. If that
            // fails, fall back to the native representation.
            //
            try
            {
              file_str = file.posix_string ();
            }
            catch (SemanticGraph::InvalidPath const&)
            {
              file_str = file.string ();
            }

            String name (
              trans.translate (
                file_str, ns->name (), l.name () + L"_item", xpath (l)));

            // Make sure the name is unique.
            //
            unsigned long n (1);
            String escaped (name);

            while (conflict (escaped))
            {
              std::wostringstream os;
              os << n++;
              escaped = name + os.str ();
            }

            t.context ().set ("anonymous", true);
            schema.new_edge<SemanticGraph::Names> (*ns, t, escaped);
          }
          catch (UnstableConflict const& ex)
          {
            SemanticGraph::Type& t (ex.type ());

            wcerr << l.file () << ":" << l.line () << ":" << l.column ()
                  << ": error: list type name '" << xpath (l) << "' "
                  << "creates an unstable conflict when used as a base "
                  << "for the item type name"
                  << endl;

            wcerr << t.file () << ":" << t.line () << ":" << t.column ()
                  << ": info: conflicting type is defined here" << endl;

            wcerr << l.file () << ":" << l.line () << ":" << l.column ()
                  << ": info: "
                  << "use --anonymous-regex to resolve this conflict"
                  << endl;

            wcerr << l.file () << ":" << l.line () << ":" << l.column ()
                  << ": info: "
                  << "and don't forget to pass the same option when "
                  << "translating '" << l.file ().leaf () << "' and all "
                  << "the schemas that refer to it" << endl;

            failed = true;
          }
        }
      }

      virtual void
      traverse (SemanticGraph::Union& u)
      {
        String file_str;

        for (SemanticGraph::Union::ArgumentedIterator i (
               u.argumented_begin ()); i != u.argumented_end (); ++i)
        {
          SemanticGraph::Type& t (i->type ());

          if (!t.named_p () &&
              !t.is_a<SemanticGraph::Fundamental::IdRef> () &&
              !t.is_a<SemanticGraph::Fundamental::IdRefs> ())
          {
            try
            {
              // Run the name through the translation service.
              //

              if (!file_str)
              {
                SemanticGraph::Path file (path (u));
                file.normalize ();

                // Try to use the portable representation of the path. If
                // that fails, fall back to the native representation.
                //
                try
                {
                  file_str = file.posix_string ();
                }
                catch (SemanticGraph::InvalidPath const&)
                {
                  file_str = file.string ();
                }
              }

              String name (
                trans.translate (
                  file_str, ns->name (), u.name () + L"_member", xpath (u)));

              // Make sure the name is unique.
              //
              unsigned long n (1);
              String escaped (name);

              while (conflict (escaped))
              {
                std::wostringstream os;
                os << n++;
                escaped = name + os.str ();
              }

              t.context ().set ("anonymous", true);
              schema.new_edge<SemanticGraph::Names> (*ns, t, escaped);
            }
            catch (UnstableConflict const& ex)
            {
              SemanticGraph::Type& t (ex.type ());

              wcerr << u.file () << ":" << u.line () << ":" << u.column ()
                    << ": error: union type name '" << xpath (u) << "' "
                    << "creates an unstable conflict when used as a base "
                    << "for the member type name"
                    << endl;

              wcerr << t.file () << ":" << t.line () << ":" << t.column ()
                    << ": info: conflicting type is defined here" << endl;

              wcerr << u.file () << ":" << u.line () << ":" << u.column ()
                    << ": info: "
                    << "use --anonymous-regex to resolve this conflict"
                    << endl;

              wcerr << u.file () << ":" << u.line () << ":" << u.column ()
                    << ": info: "
                    << "and don't forget to pass the same option when "
                    << "translating '" << u.file ().leaf () << "' and all "
                    << "the schemas that refer to it" << endl;

              failed = true;
            }
          }
        }
      }

      virtual void
      traverse (SemanticGraph::Complex& c)
      {
        if (!c.inherits_p ())
          return;

        SemanticGraph::Type& t (c.inherits ().base ());

        //@@ This IDREF stuff is really ugly!
        //
        if (!t.named_p () &&
            !t.is_a<SemanticGraph::Fundamental::IdRef> () &&
            !t.is_a<SemanticGraph::Fundamental::IdRefs> ())
        {
          try
          {
            // Run the name through the translation service.
            //
            SemanticGraph::Path file (path (c));
            file.normalize ();
            String file_str;

            // Try to use the portable representation of the path. If that
            // fails, fall back to the native representation.
            //
            try
            {
              file_str = file.posix_string ();
            }
            catch (SemanticGraph::InvalidPath const&)
            {
              file_str = file.string ();
            }

            String name (
              trans.translate (
                file_str, ns->name (), c.name () + L"_base", xpath (c)));

            // Make sure the name is unique.
            //
            unsigned long n (1);
            String escaped (name);

            while (conflict (escaped))
            {
              std::wostringstream os;
              os << n++;
              escaped = name + os.str ();
            }

            t.context ().set ("anonymous", true);
            schema.new_edge<SemanticGraph::Names> (*ns, t, escaped);
          }
          catch (UnstableConflict const& ex)
          {
            SemanticGraph::Type& t (ex.type ());

            wcerr << c.file () << ":" << c.line () << ":" << c.column ()
                  << ": error: simple type name '" << xpath (c) << "' "
                  << "creates an unstable conflict when used as a base "
                  << "for the base type name"
                  << endl;

            wcerr << t.file () << ":" << t.line () << ":" << t.column ()
                  << ": info: conflicting type is defined here" << endl;

            wcerr << c.file () << ":" << c.line () << ":" << c.column ()
                  << ": info: "
                  << "use --anonymous-regex to resolve this conflict"
                  << endl;

            wcerr << c.file () << ":" << c.line () << ":" << c.column ()
                  << ": info: "
                  << "and don't forget to pass the same option when "
                  << "translating '" << c.file ().leaf () << "' and all "
                  << "the schemas that refer to it" << endl;

            failed = true;
          }
        }
      }
    };


    //
    //
    struct Member: Traversal::Element,
                   Traversal::Attribute,
                   protected virtual Context
    {
      Member (Context& c)
          : Context (c)
      {
      }

      virtual void
      traverse (SemanticGraph::Element& e)
      {
        SemanticGraph::Type& t (e.type ());

        //@@ This IDREF stuff is really ugly!
        //
        if (!t.named_p () &&
            !t.is_a<SemanticGraph::Fundamental::IdRef> () &&
            !t.is_a<SemanticGraph::Fundamental::IdRefs> ())
        {
          try
          {
            traverse_ (e);
          }
          catch (UnstableConflict const& ex)
          {
            SemanticGraph::Type& t (ex.type ());

            wcerr << e.file () << ":" << e.line () << ":" << e.column ()
                  << ": error: element name '" << xpath (e) << "' "
                  << "creates an unstable conflict when used as a type name"
                  << endl;

            wcerr << t.file () << ":" << t.line () << ":" << t.column ()
                  << ": info: conflicting type is defined here" << endl;

            wcerr << e.file () << ":" << e.line () << ":" << e.column ()
                  << ": info: "
                  << "use --anonymous-regex to resolve this conflict"
                  << endl;

            wcerr << e.file () << ":" << e.line () << ":" << e.column ()
                  << ": info: "
                  << "and don't forget to pass the same option when "
                  << "translating '" << e.file ().leaf () << "' and all "
                  << "the schemas that refer to it" << endl;

            failed = true;
          }
        }
      }

      virtual void
      traverse (SemanticGraph::Attribute& a)
      {
        SemanticGraph::Type& t (a.type ());

        //@@ This IDREF stuff us really ugly!
        //
        if (!t.named_p () &&
            !t.is_a<SemanticGraph::Fundamental::IdRef> () &&
            !t.is_a<SemanticGraph::Fundamental::IdRefs> ())
        {
          try
          {
            traverse_ (a);
          }
          catch (UnstableConflict const& ex)
          {
            SemanticGraph::Type& t (ex.type ());

            wcerr << a.file () << ":" << a.line () << ":" << a.column ()
                  << ": error: attribute name '" << xpath (a) << "' "
                  << "creates an unstable conflict when used as a type name"
                  << endl;

            wcerr << t.file () << ":" << t.line () << ":" << t.column ()
                  << ": info: conflicting type is defined here" << endl;

            wcerr << a.file () << ":" << a.line () << ":" << a.column ()
                  << ": info: "
                  << "use --anonymous-regex to resolve this conflict"
                  << endl;

            wcerr << a.file () << ":" << a.line () << ":" << a.column ()
                  << ": info: "
                  << "and don't forget to pass the same option when "
                  << "translating '" << a.file ().leaf () << "' and all "
                  << "the schemas that refer to it" << endl;

            failed = true;
          }
        }
      }

      void
      traverse_ (SemanticGraph::Member& m)
      {
        using SemanticGraph::Type;

        Type& t (m.type ());

        // Normally this will be the member which also "defined" the type.
        // However, in some cases of cyclic schema inclusion, this does
        // not happen. As a result we need an extra check that will make
        // sure we create the Names edge in the same Schema node as the
        // one which contains the member which initially defined this
        // type. See the cyclic-inclusion test for an example.
        //

        // Find the first member that this type classifies.
        //
        for (Type::ClassifiesIterator i (t.classifies_begin ());
             i != t.classifies_end (); ++i)
        {
          SemanticGraph::Instance& inst (i->instance ());

          if (inst.is_a<SemanticGraph::Member> ())
          {
            // If it is the same member as the one we are traversing,
            // then continue.
            //
            if (&inst == &m)
              break;
            else
              return;
          }
        }

        // Run the name through the translation service.
        //
        SemanticGraph::Path file (path (m));
        file.normalize ();
        String file_str;

        // Try to use the portable representation of the path. If that
        // fails, fall back to the native representation.
        //
        try
        {
          file_str = file.posix_string ();
        }
        catch (SemanticGraph::InvalidPath const&)
        {
          file_str = file.string ();
        }

        String name (
          trans.translate (file_str, ns->name (), m.name (), xpath (m)));

        // Check if this name conflicts.
        //
        unsigned long n (1);
        String escaped (name);

        while (SemanticGraph::Type* other = conflict (escaped))
        {
          // First see if we should just use the other type. It should
          // also have been anonymous and structurally equal to our type.
          //
          if (other->context ().count ("anonymous"))
          {
            if (structurally_equal (t, *other))
            {
              // Reset the elements that are classified by this type to point
              // to the other type.
              //
              for (Type::ClassifiesIterator i (t.classifies_begin ());
                   i != t.classifies_end (); ++i)
              {
                schema.reset_right_node (*i, *other);
              }

              //wcerr << "equal " << name << endl;
              return;
            }
            //else
              //wcerr << "unequal " << name << endl;
          }

          std::wostringstream os;
          os << n++;
          escaped = name + os.str ();
        }

        t.context ().set ("anonymous", true);
        schema.new_edge<SemanticGraph::Names> (*ns, t, escaped);
      }
    };
  }

  namespace Transformations
  {
    Anonymous::
    Anonymous (AnonymousNameTranslator& trans)
        : trans_ (trans)
    {
    }

    void Anonymous::
    transform (SemanticGraph::Schema& s,
               SemanticGraph::Path const& f,
               bool duc)
    {
      Context ctx (s, f, trans_, duc);

      Traversal::Schema schema;
      Uses uses;

      schema >> uses >> schema;

      Traversal::Names schema_names;
      Namespace ns (ctx.ns);
      Traversal::Names ns_names_member;
      Traversal::Names ns_names_type;

      schema >> schema_names >> ns;
      ns >> ns_names_member;
      ns >> ns_names_type;

      Type type (ctx);
      ns_names_type >> type;

      Traversal::Scope scope; // Goes to both types and groups.
      Member member (ctx);

      ns_names_member >> scope;
      ns_names_member >> member;

      Traversal::Names names;

      scope >> names >> member;

      // Some twisted schemas do recusive inclusions.
      //
      s.context ().set ("xsd-frontend-anonymous-seen", true);

      schema.dispatch (s);

      if (ctx.failed)
        throw Failed ();
    }

    AnonymousNameTranslator::
    ~AnonymousNameTranslator ()
    {
    }
  }
}
