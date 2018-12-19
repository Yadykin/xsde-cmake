// file      : tests/dump/driver.cxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsd-frontend/types.hxx>
#include <xsd-frontend/parser.hxx>
#include <xsd-frontend/transformations/anonymous.hxx>
#include <xsd-frontend/transformations/enum-synthesis.hxx>

#include <xsd-frontend/semantic-graph.hxx>
#include <xsd-frontend/traversal.hxx>

#include <iostream>

using namespace std;
using namespace XSDFrontend;

static unsigned long indent;

std::wostream&
ind (std::wostream& os)
{
  for (unsigned long n (0); n < indent; ++n)
    os << L"  ";

  return os;
}

namespace
{
  // Nameable which is named in the namespace scope.
  //
  String
  ref_name (SemanticGraph::Nameable& n)
  {
    String const& scope (n.scope ().name ());

    return scope + (scope.empty () ? L"" : L"#") + n.name ();
  }

  struct List: Traversal::List
  {
    virtual void
    traverse (Type& l)
    {
      if (l.annotated_p ())
        wcout << ind << "<" << l.annotation ().documentation () << ">"
              << endl;

      wcout << ind << "list " <<
        (l.named_p () ? l.name () : String ("<anonymous>"));

      SemanticGraph::Type& t (l.argumented ().type ());

      if (t.named_p ())
        wcout << " " << ref_name (t) << endl;
      else
      {
        wcout << endl
              << ind << "{" << endl;
        indent++;

        edge_traverser ().dispatch (l.argumented ());

        indent--;
        wcout << ind << "}" << endl;
      }
    }
  };

  struct Union: Traversal::Union
  {
    virtual void
    traverse (Type& u)
    {
      if (u.annotated_p ())
        wcout << ind << "<" << u.annotation ().documentation () << ">"
              << endl;

      wcout << ind << "union " <<
        (u.named_p () ? u.name () : String ("<anonymous>")) << " ";

      for (Type::ArgumentedIterator i (u.argumented_begin ());
           i != u.argumented_end (); ++i)
      {
        SemanticGraph::Type& t (i->type ());

        if (t.named_p ())
          wcout << ref_name (t) << " ";
        else
        {
          wcout << endl
                << ind << "{" << endl;
          indent++;

          edge_traverser ().dispatch (*i);

          indent--;
          wcout << ind << "}" << endl;
        }
      }

      wcout << endl;
    }
  };

  struct Enumerator: Traversal::Enumerator
  {
    virtual void
    traverse (Type& e)
    {
      if (e.annotated_p ())
        wcout << ind << "<" << e.annotation ().documentation () << ">"
              << endl;

      wcout << ind << "enumerator " << e.name () << endl;
    }
  };

  struct Enumeration: Traversal::Enumeration
  {
    virtual void
    traverse (Type& e)
    {
      if (e.annotated_p ())
        wcout << ind << "<" << e.annotation ().documentation () << ">"
              << endl;

      wcout << ind << "enumeration " <<
        (e.named_p () ? e.name () : String ("<anonymous>")) << ": " <<
        ref_name (e.inherits ().base ()) << endl
            << ind << "{" << endl;

      indent++;
      Traversal::Enumeration::traverse (e);
      indent--;

      wcout << ind << "}" << endl;
    }
  };

  struct ContainsParticle: Traversal::ContainsParticle
  {
    virtual void
    traverse (Type& cp)
    {
      wcout << ind << "[" << cp.min () << ", ";

      if (cp.max () == 0)
        wcout << "unbounded] ";
      else
        wcout << cp.max () << "] ";

      Traversal::ContainsParticle::traverse (cp);
    }
  };

  struct ContainsCompositor: Traversal::ContainsCompositor
  {
    virtual void
    traverse (Type& cc)
    {
      wcout << ind << "[" << cc.min () << ", ";

      if (cc.max () == 0)
        wcout << "unbounded] ";
      else
        wcout << cc.max () << "] ";

      Traversal::ContainsCompositor::traverse (cc);
    }
  };

  struct Compositor: Traversal::All,
                     Traversal::Choice,
                     Traversal::Sequence
  {
    virtual void
    traverse (SemanticGraph::All& a)
    {
      wcout << "all" << endl
            << ind << "{" << endl;

      indent++;

      Traversal::All::traverse (a);

      indent--;

      wcout << ind << "}" << endl;
    }

    virtual void
    traverse (SemanticGraph::Choice& c)
    {
      wcout << "choice" << endl
            << ind << "{" << endl;

      indent++;

      Traversal::Choice::traverse (c);

      indent--;

      wcout << ind << "}" << endl;
    }

    virtual void
    traverse (SemanticGraph::Sequence& s)
    {
      wcout << "sequence" << endl
            << ind << "{" << endl;

      indent++;

      Traversal::Sequence::traverse (s);

      indent--;

      wcout << ind << "}" << endl;
    }
  };

  struct Attribute: Traversal::Attribute
  {
    virtual void
    traverse (Type& a)
    {
      if (a.annotated_p ())
        wcout << ind << "<" << a.annotation ().documentation () << ">"
              << endl;

      wcout << ind << (a.optional_p () ? "optional" : "required")
            << " attribute " << a.name ();

      if (a.fixed_p ())
        wcout << "==" << a.value ();
      else if (a.default_p ())
        wcout << "=" << a.value ();

      SemanticGraph::Type& t (a.type ());

      if (t.named_p ())
        wcout << " " << ref_name (t) << endl;
      else
      {
        wcout << endl
              << ind << "{" << endl;
        indent++;

        belongs (a);

        indent--;
        wcout << ind << "}" << endl;
      }
    }
  };

  struct AnyAttribute: Traversal::AnyAttribute
  {
    virtual void
    traverse (Type& a)
    {
      if (a.annotated_p ())
        wcout << ind << "<" << a.annotation ().documentation () << ">"
              << endl;

      wcout << ind << "any-attribute '" << a.name () << "'" << endl;
    }
  };

  struct Element: Traversal::Element
  {
    virtual void
    traverse (Type& e)
    {
      wcout << "element " << e.name ();

      if (e.fixed_p ())
        wcout << "==" << e.value ();
      else if (e.default_p ())
        wcout << "=" << e.value ();

      SemanticGraph::Type& t (e.type ());

      if (t.named_p ())
        wcout << " " << ref_name (t) << endl;
      else
      {
        wcout << endl
              << ind << "{" << endl;
        indent++;

        belongs (e);

        indent--;
        wcout << ind << "}" << endl;
      }
    }
  };

  struct ElementFlat: Traversal::Element
  {
    virtual void
    traverse (Type& e)
    {
      if (e.annotated_p ())
        wcout << ind << "<" << e.annotation ().documentation () << ">"
              << endl;

      wcout << ind << "element " << e.name ();

      if (e.fixed_p ())
        wcout << "==" << e.value ();
      else if (e.default_p ())
        wcout << "=" << e.value ();

      wcout << endl;
    }
  };

  struct Any: Traversal::Any
  {
    virtual void
    traverse (Type& a)
    {
      wcout << "any '" << a.name () << "'" << endl;
    }
  };

  struct AnyFlat: Traversal::Any
  {
    virtual void
    traverse (Type& a)
    {
      if (a.annotated_p ())
        wcout << ind << "<" << a.annotation ().documentation () << ">"
              << endl;

      wcout << ind << "any '" << a.name () << "'" << endl;
    }
  };

  struct Complex: Traversal::Complex
  {
    virtual void
    traverse (Type& c)
    {
      // Anonymous type definition can recursively refer to itself.
      //
      if (c.context ().count ("seen"))
      {
        wcout << ind << "complex <recursive-anonymous>" << endl;
        return;
      }

      c.context ().set ("seen", true);

      if (c.annotated_p ())
        wcout << ind << "<" << c.annotation ().documentation () << ">"
              << endl;

      wcout << ind << "complex " <<
        (c.named_p () ? c.name () : String ("<anonymous>"));

      if (c.inherits_p ())
        wcout << ": " << ref_name (c.inherits ().base ());

      wcout << endl
            << ind << "{" << endl;
      indent++;

      Traversal::Complex::traverse (c);

      indent--;
      wcout << ind << "}" << endl;

      c.context ().remove ("seen");
    }
  };

  struct GlobalAttribute: Traversal::Attribute
  {
    virtual void
    traverse (Type& a)
    {
      if (a.annotated_p ())
        wcout << ind << "<" << a.annotation ().documentation () << ">"
              << endl;

      wcout << ind << "attribute " << a.name ();

      if (a.fixed_p ())
        wcout << "==" << a.value ();
      else if (a.default_p ())
        wcout << "=" << a.value ();

      SemanticGraph::Type& t (a.type ());

      if (t.named_p ())
        wcout << " " << ref_name (t) << endl;
      else
      {
        wcout << endl
              << ind << "{" << endl;
        indent++;

        belongs (a);

        indent--;
        wcout << ind << "}" << endl;
      }
    }
  };

  struct GlobalElement: Traversal::Element
  {
    virtual void
    traverse (Type& e)
    {
      if (e.annotated_p ())
        wcout << ind << "<" << e.annotation ().documentation () << ">"
              << endl;

      wcout << ind << "element " << e.name ();

      if (e.fixed_p ())
        wcout << "==" << e.value ();
      else if (e.default_p ())
        wcout << "=" << e.value ();

      SemanticGraph::Type& t (e.type ());

      if (t.named_p ())
        wcout << " " << ref_name (t) << endl;
      else
      {
        wcout << endl
              << ind << "{" << endl;
        indent++;

        belongs (e);

        indent--;
        wcout << ind << "}" << endl;
      }
    }
  };

  struct Namespace: Traversal::Namespace
  {
    virtual void
    traverse (Type& n)
    {
      wcout << ind << "namespace " << n.name () << endl
            << ind << "{" << endl;
      indent++;
      Traversal::Namespace::traverse (n);
      indent--;
      wcout << ind << "}" << endl;
    }
  };

  // Go into implied/included/imported schemas while making sure
  // we don't recurse forever.
  //
  struct Uses: Traversal::Imports,
               Traversal::Includes,
               Traversal::Sources
               //Traversal::Implies @@ Need a --with-implies option
  {
    virtual void
    traverse (SemanticGraph::Imports& i)
    {
      if (traverse_uses (i, "imports"))
        Traversal::Imports::traverse (i);
    }

    virtual void
    traverse (SemanticGraph::Includes& i)
    {
      if (traverse_uses (i, "includes"))
        Traversal::Includes::traverse (i);
    }

    virtual void
    traverse (SemanticGraph::Sources& s)
    {
      if (traverse_uses (s, "sources"))
        Traversal::Sources::traverse (s);
    }

    /*
    virtual void
    traverse (SemanticGraph::Implies& i)
    {
      if (traverse_uses (i, "implies"))
        Traversal::Implies::traverse (i);
    }
    */

    bool
    traverse_uses (SemanticGraph::Uses& u, String const& type)
    {
      SemanticGraph::Schema& s (u.schema ());

      if (s.context ().count ("seen"))
      {
        wcout << ind << "recursively " << type << " " << u.path () << endl;
        return false;
      }

      s.context ().set ("seen", true);

      if (s.annotated_p ())
        wcout << ind << "<" << s.annotation ().documentation () << ">" << endl;

      wcout << ind << type << " " << u.path () << endl;

      return true;
    }
  };

  struct Schema: Traversal::Schema
  {
    virtual void
    traverse (Type& s)
    {
      wcout << ind << "{" << endl;
      indent++;
      Traversal::Schema::traverse (s);
      indent--;
      wcout << ind << "}" << endl;
    }
  };
}

struct AnonymousNameTranslator: Transformations::AnonymousNameTranslator
{
  virtual String
  translate (String const& /*file*/,
             String const& ns,
             String const& name,
             String const& xpath)
  {
    wcout << "anonymous: " << ns << " " << name << " " << xpath << endl;
    return name;
  }
};

int
main (int argc, char* argv[])
{
  try
  {
    if (argc < 2)
    {
      wcerr << argv[0] << ": error: no input file." << endl;
      return 1;
    }

    // Parse options.
    //
    int i (1);
    bool anon (false);
    bool enum_synth (false);

    for (; i < argc; ++i)
    {
      if (argv[i] == NarrowString ("--anonymous"))
        anon = true;
      else if (argv[i] == NarrowString ("--enum-synthesis"))
        enum_synth = true;
      else
        break;
    }

    // Parse schema.
    //
    SemanticGraph::Path path (argv[i]);

    Parser parser (true, false, true);
    auto_ptr<SemanticGraph::Schema> tu (parser.parse (path));

    //
    //
    if (anon)
    {
      try
      {
        AnonymousNameTranslator transl;
        Transformations::Anonymous transf (transl);
        transf.transform (*tu, path, true);
      }
      catch (Transformations::Anonymous::Failed const&)
      {
        // Diagnostics has already been issued.
        //
        return 1;
      }
    }

    //
    //
    if (enum_synth)
    {
      Transformations::EnumSynthesis transf;
      transf.transform (*tu, path);
    }

    //
    //
    Schema schema;
    Uses uses;

    schema >> uses >> schema;

    Traversal::Names schema_names;
    Namespace ns;
    Traversal::Names ns_names;

    schema >> schema_names >> ns >> ns_names;

    //
    //
    List list;
    Union union_;
    Complex complex;
    Enumeration enumeration;
    GlobalElement global_element;
    GlobalAttribute global_attribute;

    Traversal::Names complex_names;
    Traversal::Names enumeration_names;
    ContainsCompositor contains_compositor;

    ns_names >> list;
    ns_names >> union_;
    ns_names >> complex;
    ns_names >> enumeration;
    ns_names >> global_attribute;
    ns_names >> global_element;

    complex >> complex_names;
    complex >> contains_compositor;

    enumeration >> enumeration_names;

    //
    //
    Compositor compositor;
    ContainsParticle contains_particle;

    contains_compositor >> compositor;
    compositor >> contains_particle >> compositor;

    //
    //
    Any any;
    AnyFlat any_flat;
    Element element;
    ElementFlat element_flat;
    Attribute attribute;
    AnyAttribute any_attribute;
    Traversal::Belongs belongs;

    element >> belongs;
    attribute >> belongs;

    global_element >> belongs;
    global_attribute >> belongs;

    complex_names >> attribute;
    complex_names >> any_attribute;
    complex_names >> any_flat;
    complex_names >> element_flat;

    contains_particle >> any;
    contains_particle >> element;

    belongs >> list;
    belongs >> union_;
    belongs >> complex;
    belongs >> enumeration;

    //
    //
    Traversal::Argumented argumented;
    list >> argumented;
    union_ >> argumented;

    argumented >> list;
    argumented >> union_;
    argumented >> complex;
    argumented >> enumeration;

    //
    //
    Enumerator enumerator;
    enumeration_names >> enumerator;

    //
    //
    if (tu->annotated_p ())
      wcout << ind << "<" << tu->annotation ().documentation () << ">"
            << endl;

    wcout << ind << "primary" << endl;
    tu->context ().set ("seen", true);
    schema.dispatch (*tu);

    return 0;
  }
  catch (InvalidSchema const&)
  {
    // Diagnostic has already been issued.
  }
  catch (SemanticGraph::InvalidPath const&)
  {
    wcerr << argv[0] << ": error: '" << argv[1] << "' is not a valid "
      << "filesystem path" << endl;
  }

  return 1;
}
