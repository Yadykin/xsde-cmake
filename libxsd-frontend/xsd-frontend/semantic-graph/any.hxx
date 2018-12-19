// file      : xsd-frontend/semantic-graph/any.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_SEMANTIC_GRAPH_ANY_HXX
#define XSD_FRONTEND_SEMANTIC_GRAPH_ANY_HXX

#include <vector>

#include <xsd-frontend/semantic-graph/elements.hxx>
#include <xsd-frontend/semantic-graph/particle.hxx>
#include <xsd-frontend/semantic-graph/namespace.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    class Any: public virtual Nameable,
               public virtual Particle
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

      Any&
      prototype ()
      {
        assert (prototype_ != 0);
        return *prototype_;
      }

      void
      prototype (Any& a)
      {
        assert (prototype_ == 0);
        prototype_ = &a;
      }

    public:
      Namespace&
      definition_namespace ();

    public:
      Any (Path const& file,
           unsigned long line,
           unsigned long column,
           String const& namespaces);

      Any (Path const& file,
           unsigned long line,
           unsigned long column,
           NamespaceIterator begin,
           NamespaceIterator end);

      using Nameable::add_edge_right;
      using Particle::add_edge_right;

    private:
      Any* prototype_;
      Namespaces namespaces_;
    };
  }
}

#endif  // XSD_FRONTEND_SEMANTIC_GRAPH_ANY_HXX
