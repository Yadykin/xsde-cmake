// file      : xsd-frontend/semantic-graph/schema.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_SEMANTIC_GRAPH_SCHEMA_HXX
#define XSD_FRONTEND_SEMANTIC_GRAPH_SCHEMA_HXX

#include <set>
#include <vector>

#include <xsd-frontend/semantic-graph/elements.hxx>
#include <xsd-frontend/semantic-graph/namespace.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    class Schema;

    class Uses: public virtual Edge
    {
    public:
      Schema&
      user () const
      {
        return *user_;
      }

      Schema&
      schema () const
      {
        return *schema_;
      }

      Path
      path () const
      {
        return path_;
      }

    public:
      Uses (Path const& path): path_ (path) {}

      void
      set_left_node (Schema& s)
      {
        user_ = &s;
      }

      void
      set_right_node (Schema& s)
      {
        schema_ = &s;
      }

    private:
      Path path_;
      Schema* user_;
      Schema* schema_;
    };


    //
    //
    class Implies: public virtual Uses
    {
    public:
      Implies (Path const& path): Uses (path) {}
    };


    //
    //
    class Sources: public virtual Uses
    {
    public:
      Sources (Path const& path): Uses (path) {}
    };


    //
    //
    class Includes: public virtual Uses
    {
    public:
      Includes (Path const& path): Uses (path) {}
    };


    //
    //
    class Imports: public virtual Uses
    {
    public:
      Imports (Path const& path): Uses (path) {}
    };

    //
    //
    class Schema: public graph, public virtual Scope
    {
      typedef std::vector<Uses*> UsesList;
      typedef std::vector<Uses*> UsedList;

    public:
      Schema (Path const& file, unsigned long line, unsigned long column)
          : Node (file, line, column), graph_ (*this)
      {
      }

    private:
      Schema (Schema const&);
      Schema& operator= (Schema const&);

    public:
      typedef pointer_iterator<UsesList::const_iterator> UsesIterator;

      UsesIterator
      uses_begin () const
      {
        return uses_.begin ();
      }

      UsesIterator
      uses_end () const
      {
        return uses_.end ();
      }

    public:
      typedef pointer_iterator<UsedList::const_iterator> UsedIterator;

      UsedIterator
      used_begin () const
      {
        return used_.begin ();
      }

      UsedIterator
      used_end () const
      {
        return used_.end ();
      }

      bool
      used_p () const
      {
        return used_begin () != used_end ();
      }

      virtual NamesIteratorPair
      find (Name const& name) const;

    public:
      using graph::new_edge;
      using graph::reset_left_node;
      using graph::reset_right_node;
      using graph::add_edge_left;
      using graph::add_edge_right;
      using graph::delete_node;
      using graph::delete_edge;

      template <typename T>
      T&
      new_node (Path const& file, unsigned long line, unsigned long column)
      {
        return graph_.new_node<T> (file, line, column);
      }

      template <typename T, typename A0>
      T&
      new_node (Path const& file, unsigned long line, unsigned long column,
                A0 const& a0)
      {
        return graph_.new_node<T> (file, line, column, a0);
      }

      template <typename T, typename A0, typename A1>
      T&
      new_node (Path const& file, unsigned long line, unsigned long column,
                A0 const& a0, A1 const& a1)
      {
        return graph_.new_node<T> (file, line, column, a0, a1);
      }

      template <typename T, typename A0, typename A1, typename A2>
      T&
      new_node (Path const& file, unsigned long line, unsigned long column,
                A0 const& a0, A1 const& a1, A2 const& a2)
      {
        return graph_.new_node<T> (file, line, column, a0, a1, a2);
      }

      template <typename T, typename A0, typename A1, typename A2,
                typename A3>
      T&
      new_node (Path const& file, unsigned long line, unsigned long column,
                A0 const& a0, A1 const& a1, A2 const& a2, A3 const& a3)
      {
        return graph_.new_node<T> (file, line, column, a0, a1, a2, a3);
      }

    public:
      using Scope::add_edge_left;
      using Node::add_edge_right;

      void
      add_edge_left (Uses& e)
      {
        uses_.push_back (&e);
      }

      void
      add_edge_right (Uses& e)
      {
        used_.push_back (&e);
      }

    private:
      typedef std::set<Schema const*> SchemaSet;

      void
      find_ (Name const& name, NamesList&, SchemaSet&) const;

    private:
      graph& graph_;

      UsesList uses_;
      UsedList used_;

      mutable NamesList names_;
      mutable SchemaSet schemas_;
    };
  }
}

#endif  // XSD_FRONTEND_SEMANTIC_GRAPH_SCHEMA_HXX
