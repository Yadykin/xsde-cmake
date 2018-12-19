// file      : xsd-frontend/semantic-graph/any-attribute.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cutl/compiler/type-info.hxx>

#include <xsd-frontend/semantic-graph/any-attribute.hxx>
#include <xsd-frontend/semantic-graph/compositors.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    AnyAttribute::
    AnyAttribute (Path const& file,
                  unsigned long line,
                  unsigned long column,
                  String const& namespaces)
        : Node (file, line, column),
          prototype_ (0)
    {
      // Not sure if the separator is just space or any white-space
      // chararcter.
      //

      for (size_t i (0), j (namespaces.find (L' '));;)
      {
        if (j != String::npos)
        {
          namespaces_.push_back (String (namespaces, i, j - i));

          i = j + 1;
          j = namespaces.find (L' ', i);
        }
        else
        {
          // Last element.
          //
          namespaces_.push_back (String (namespaces, i));
          break;
        }
      }
    }

    AnyAttribute::
    AnyAttribute (Path const& file,
                  unsigned long line,
                  unsigned long column,
                  NamespaceIterator begin,
                  NamespaceIterator end)
        : Node (file, line, column),
          prototype_ (0)
    {
      for (; begin != end; ++begin)
        namespaces_.push_back (*begin);
    }

    namespace
    {
      Namespace&
      namespace_ (Nameable& n)
      {
        // The basic idea goes like this: go up Names edges until you
        // reach Namespace. There are, however, anonymous types which
        // need special handling. In the case of an anonymous type we
        // will go up the first Belongs edge (because the first edge
        // is where the type was defined.
        //

        if (n.named_p ())
        {
          Scope& s (n.scope ());
          Namespace* ns (dynamic_cast<Namespace*> (&n));

          return ns ? *ns : namespace_ (s);
        }
        else
        {
          Type& t (dynamic_cast<Type&> (n));
          Belongs& b (*t.classifies_begin ());

          return namespace_ (b.instance ());
        }
      }
    }

    Namespace& AnyAttribute::
    definition_namespace ()
    {
      if (prototype_p ())
        return prototype ().definition_namespace ();

      return namespace_ (scope ());
    }

    namespace
    {
      using compiler::type_info;

      struct AnyAttributeInit
      {
        AnyAttributeInit ()
        {
          type_info ti (typeid (AnyAttribute));
          ti.add_base (typeid (Nameable));
          insert (ti);
        }
      } any_attribute_init_;
    }
  }
}
