// file      : xsd-frontend/semantic-graph/any-attribute.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_SEMANTIC_GRAPH_ANY_ATTRIBUTE_HXX
#define XSD_FRONTEND_SEMANTIC_GRAPH_ANY_ATTRIBUTE_HXX

#include <vector>

#include <xsd-frontend/semantic-graph/elements.hxx>
#include <xsd-frontend/semantic-graph/namespace.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    class AnyAttribute: public virtual Nameable
    {
      typedef std::vector<String> Namespaces;

    public:
      typedef Namespaces::const_iterator NamespaceIterator;

      NamespaceIterator
      namespace_begin () const
      {
        return namespaces_.begin ();
      }

      NamespaceIterator
      namespace_end () const
      {
        return namespaces_.end ();
      }

    public:
      bool
      prototype_p ()
      {
        return prototype_ != 0;
      }

      AnyAttribute&
      prototype ()
      {
        assert (prototype_ != 0);
        return *prototype_;
      }

      void
      prototype (AnyAttribute& a)
      {
        assert (prototype_ == 0);
        prototype_ = &a;
      }

    public:
      Namespace&
      definition_namespace ();

    public:
      AnyAttribute (Path const& file,
                    unsigned long line,
                    unsigned long column,
                    String const& namespaces);

      AnyAttribute (Path const& file,
                    unsigned long line,
                    unsigned long column,
                    NamespaceIterator begin,
                    NamespaceIterator end);

    private:
      AnyAttribute* prototype_;
      Namespaces namespaces_;
    };
  }
}

#endif  // XSD_FRONTEND_SEMANTIC_GRAPH_ANY_ATTRIBUTE_HXX
