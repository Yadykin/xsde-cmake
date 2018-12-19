// file      : xsd-frontend/semantic-graph/elements.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_SEMANTIC_GRAPH_ELEMENTS_HXX
#define XSD_FRONTEND_SEMANTIC_GRAPH_ELEMENTS_HXX

#include <set>
#include <map>
#include <list>
#include <vector>
#include <iosfwd>
#include <cstddef> // std::size_t
#include <utility> // std::pair
#include <cstdlib> // abort
#include <cassert>

#include <cutl/container/graph.hxx>
#include <cutl/container/pointer-iterator.hxx>
#include <cutl/compiler/context.hxx>
#include <cutl/fs/path.hxx>

#include <xsd-frontend/types.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    using namespace cutl;

    using container::pointer_iterator;

    //
    //
    typedef fs::path Path;
    typedef fs::invalid_path InvalidPath;
    typedef std::vector<Path> Paths;

    typedef compiler::context Context;

    //
    //
    class Node;
    class Edge;

    //
    //
    class Annotates;
    class Annotation;

    //
    //
    class Edge
    {
    public:
      Context&
      context () const
      {
        return context_;
      }

      virtual
      ~Edge ()
      {
      }

    public:
      template <typename X>
      bool
      is_a () const
      {
        return dynamic_cast<X const*> (this) != 0;
      }

    private:
      mutable Context context_;
    };

    inline bool
    operator== (Edge const& x, Edge const& y)
    {
      return &x == &y;
    }


    //
    //
    class Node
    {
    public:
      Context&
      context () const
      {
        return context_;
      }

    public:
      Path const&
      file () const
      {
        return file_;
      }

      unsigned long
      line () const
      {
        return line_;
      }

      unsigned long
      column () const
      {
        return column_;
      }

    public:
      bool
      annotated_p () const
      {
        return annotates_ != 0;
      }

      Annotates&
      annotated () const
      {
        return *annotates_;
      }

      Annotation&
      annotation ();

    public:
      template <typename X>
      bool
      is_a () const
      {
        return dynamic_cast<X const*> (this) != 0;
      }

    public:
      virtual
      ~Node () {}

      Node (Path const& file, unsigned long line, unsigned long column)
          : annotates_ (0), file_ (file), line_ (line), column_ (column)
      {
      }

      void
      add_edge_right (Annotates& a)
      {
        annotates_ = &a;
      }

    protected:
      Node () // For virtual inheritance.
      {
        abort (); // Told you so!
      }

    private:
      mutable Context context_;
      Annotates* annotates_;
      Path file_;
      unsigned long line_;
      unsigned long column_;
    };

    inline bool
    operator== (Node const& x, Node const& y)
    {
      return &x == &y;
    }

    //
    //
    typedef container::graph<Node, Edge> graph;

    //
    //
    typedef String Name;


    //
    //
    class Scope;
    class Nameable;


    //
    //
    class Names: public virtual Edge
    {
    public:
      Name
      name () const
      {
        return name_;
      }

      Scope&
      scope () const
      {
        return *scope_;
      }

      Nameable&
      named () const
      {
        return *named_;
      }

    public:
      Names (Name const& name): name_ (name) {}

      void
      set_left_node (Scope& n)
      {
        scope_ = &n;
      }

      void
      set_right_node (Nameable& n)
      {
        named_ = &n;
      }

      void
      clear_left_node (Scope& n)
      {
        assert (scope_ == &n);
        scope_ = 0;
      }

      void
      clear_right_node (Nameable& n)
      {
        assert (named_ == &n);
        named_ = 0;
      }

    private:
      Scope* scope_;
      Nameable* named_;
      Name name_;
    };


    class Nameable: public virtual Node
    {
    public:
      bool
      named_p () const
      {
        return named_ != 0;
      }

      Name
      name () const
      {
        assert (named_p ());
        return named_->name ();
      }

      Scope&
      scope ()
      {
        assert (named_p ());
        return named_->scope ();
      }

      Names&
      named ()
      {
        assert (named_p ());
        return *named_;
      }

    public:
      Nameable (): named_ (0) {}

      void
      add_edge_right (Names& e)
      {
        named_ = &e;
      }

      void
      remove_edge_right (Names& e)
      {
        assert (named_ == &e);
        named_ = 0;
      }

      using Node::add_edge_right;

    private:
      Names* named_;
    };

    //
    //
    typedef std::set<Nameable*> Nameables;

    //
    //
    class Scope: public virtual Nameable
    {
    protected:
      typedef std::list<Names*> NamesList;
      typedef std::map<Names*, NamesList::iterator> ListIteratorMap;
      typedef std::map<Name, NamesList> NamesMap;

    public:
      typedef pointer_iterator<NamesList::iterator> NamesIterator;
      typedef pointer_iterator<NamesList::const_iterator> NamesConstIterator;

      typedef
      std::pair<NamesConstIterator, NamesConstIterator>
      NamesIteratorPair;

      NamesIterator
      names_begin ()
      {
        return names_.begin ();
      }

      NamesIterator
      names_end ()
      {
        return names_.end ();
      }

      NamesConstIterator
      names_begin () const
      {
        return names_.begin ();
      }

      NamesConstIterator
      names_end () const
      {
        return names_.end ();
      }

      std::size_t
      names_size () const
      {
        return names_.size ();
      }

      virtual NamesIteratorPair
      find (Name const& name) const
      {
        NamesMap::const_iterator i (names_map_.find (name));

        if (i == names_map_.end ())
          return NamesIteratorPair (names_.end (), names_.end ());
        else
          return NamesIteratorPair (i->second.begin (), i->second.end ());
      }

      NamesIterator
      find (Names& e)
      {
        ListIteratorMap::iterator i (iterator_map_.find (&e));
        return i != iterator_map_.end () ? i->second : names_.end ();
      }

    public:
      Scope (Path const& file, unsigned long line, unsigned long column)
          : Node (file, line, column)
      {
      }

      void
      add_edge_left (Names& e)
      {
        NamesList::iterator i (names_.insert (names_.end (), &e));
        iterator_map_[&e] = i;
        names_map_[e.name ()].push_back (&e);
      }

      void
      remove_edge_left (Names& e)
      {
        ListIteratorMap::iterator i (iterator_map_.find (&e));
        assert (i != iterator_map_.end ());

        names_.erase (i->second);
        iterator_map_.erase (i);

        NamesMap::iterator j (names_map_.find (e.name ()));

        for (NamesList::iterator i (j->second.begin ());
             i != j->second.end (); ++i)
        {
          if (*i == &e)
            i = j->second.erase (i);
        }
      }

      void
      add_edge_left (Names& e, NamesIterator const& after)
      {
        NamesList::iterator i;

        if (after.base () == names_.end ())
          i = names_.insert (names_.begin (), &e);
        else
        {
          NamesList::iterator j (after.base ());
          i = names_.insert (++j, &e);
        }

        iterator_map_[&e] = i;
        names_map_[e.name ()].push_back (&e);
      }

      using Nameable::add_edge_right;

    protected:
      Scope () {}

    private:
      NamesList names_;
      ListIteratorMap iterator_map_;
      NamesMap names_map_;
    };


    //
    //
    class Belongs;
    class Inherits;
    class Arguments;

    class Type: public virtual Nameable
    {
    protected:
      typedef std::vector<Belongs*> Classifies;
      typedef std::vector<Inherits*> Begets;
      typedef std::set<Arguments*> ArgumentsSet;

    public:
      typedef pointer_iterator<Classifies::const_iterator> ClassifiesIterator;

      ClassifiesIterator
      classifies_begin () const
      {
        return classifies_.begin ();
      }

      ClassifiesIterator
      classifies_end () const
      {
        return classifies_.end ();
      }

      //
      //
      bool
      inherits_p () const
      {
        return inherits_ != 0;
      }

      Inherits&
      inherits () const
      {
        assert (inherits_ != 0);
        return *inherits_;
      }

      //
      //
      typedef pointer_iterator<Begets::const_iterator> BegetsIterator;

      BegetsIterator
      begets_begin () const
      {
        return begets_.begin ();
      }

      BegetsIterator
      begets_end () const
      {
        return begets_.end ();
      }

      //
      //
      typedef pointer_iterator<ArgumentsSet::const_iterator> ArgumentsIterator;

      ArgumentsIterator
      arguments_begin () const
      {
        return arguments_.begin ();
      }

      ArgumentsIterator
      arguments_end () const
      {
        return arguments_.end ();
      }

    public:
      Type (): inherits_ (0) {}

      void
      add_edge_right (Belongs& e)
      {
        classifies_.push_back (&e);
      }

      void
      add_edge_right (Inherits& e)
      {
        begets_.push_back (&e);
      }

      using Nameable::add_edge_right;

      void
      add_edge_left (Arguments& a)
      {
        arguments_.insert (&a);
      }

      void
      remove_edge_left (Arguments&);

      void
      add_edge_left (Inherits& e)
      {
        inherits_ = &e;
      }

    private:
      Inherits* inherits_;
      Begets begets_;
      Classifies classifies_;
      ArgumentsSet arguments_;
    };

    class Instance: public virtual Nameable
    {
    public:
      Belongs&
      belongs () const
      {
        return *belongs_;
      }

      Type&
      type () const;

      bool
      typed_p () const
      {
        return belongs_ != 0;
      }

    public:
      Instance (): belongs_ (0) {}

      void
      add_edge_left (Belongs& e)
      {
        belongs_ = &e;
      }

    private:
      Belongs* belongs_;
    };


    class Belongs: public virtual Edge
    {
    public:
      Instance&
      instance () const
      {
        return *instance_;
      }

      Type&
      type () const
      {
        return *type_;
      }

    public:
      void
      set_left_node (Instance& n)
      {
        instance_ = &n;
      }

      void
      set_right_node (Type& n)
      {
        type_ = &n;
      }

    private:
      Instance* instance_;
      Type* type_;
    };


    //
    //
    class Inherits: public virtual Edge
    {
    public:
      Type&
      base () const
      {
        return *base_;
      }

      Type&
      derived () const
      {
        return *derived_;
      }

    public:
      void
      set_left_node (Type& n)
      {
        derived_ = &n;
      }

      void
      set_right_node (Type& n)
      {
        base_ = &n;
      }

    private:
      Type* base_;
      Type* derived_;
    };

    class Extends: public virtual Inherits
    {
    };

    class Restricts: public virtual Inherits
    {
    public:
      typedef std::map<String, String> Facets;
      typedef Facets::iterator FacetIterator;

      bool
      facet_empty ()
      {
        return facets_.empty ();
      }

      FacetIterator
      facet_begin ()
      {
        return facets_.begin ();
      }

      FacetIterator
      facet_end ()
      {
        return facets_.end ();
      }

      FacetIterator
      facet_find (String const& name)
      {
        return facets_.find (name);
      }

      void
      facet_insert (String const& name, String const& value)
      {
        facets_[name] = value;
      }

      Facets const&
      facets () const
      {
        return facets_;
      }

    protected:
      Facets facets_;
    };


    //
    //
    class Member;
    class Namespace;

    class BelongsToNamespace: public virtual Edge
    {
    public:
      Member&
      member () const
      {
        assert (member_ != 0);
        return *member_;
      }

      Namespace&
      namespace_ () const
      {
        assert (namespace__ != 0);
        return *namespace__;
      }

    public:
      BelongsToNamespace (): member_ (0), namespace__ (0) {}

      void
      set_left_node (Member& n)
      {
        member_ = &n;
      }

      void
      set_right_node (Namespace& n)
      {
        namespace__ = &n;
      }

    private:
      Member* member_;
      Namespace* namespace__;
    };

    //
    //
    class Member: public virtual Instance
    {
    public:
      // Member is global either if it is defined outside any type
      // or it is a ref="" of a global member.
      //
      bool
      global_p () const
      {
        return global_;
      }

      bool
      qualified_p () const
      {
        return qualified_;
      }

      // Note that only qualified members belong to a namespace.
      //
      Namespace&
      namespace_ () const
      {
        assert (belongs_to_namespace_ != 0);
        return belongs_to_namespace_->namespace_ ();
      }


      // Default and fixed value API. Note that the fixed value semantics
      // is a superset of the default value semantics. As such setting the
      // fixed value appears as if the default value was also set.
      //
      bool
      default_p () const
      {
        return value_type_ != ValueType::none;
      }

      bool
      fixed_p () const
      {
        return value_type_ == ValueType::fixed;
      }

      struct NoValue {};

      String
      value () const
      {
        if (value_type_ != ValueType::none)
          return value_;
        else
          throw NoValue ();
      }

      //
      //
      void
      default_ (String const& v)
      {
        value_ = v;
        value_type_ = ValueType::default_;
      }

      void
      fixed (String const& v)
      {
        value_ = v;
        value_type_ = ValueType::fixed;
      }

    public:
      Member (bool global, bool qualified)
          : global_ (global),
            qualified_ (qualified),
            belongs_to_namespace_ (0),
            value_type_ (ValueType::none)
      {
      }

      void
      add_edge_left (BelongsToNamespace& e)
      {
        // In the parser we sometimes re-add the same adge.
        //
        belongs_to_namespace_ = &e;
      }

      using Instance::add_edge_left;

    private:
      bool global_;
      bool qualified_;
      BelongsToNamespace* belongs_to_namespace_;

      struct ValueType
      {
        enum Value
        {
          none,
          default_,
          fixed
        };
      };

      String value_;
      ValueType::Value value_type_;
    };


    // Parametric types.
    //

    class Specialization: public virtual Type
    {
      typedef std::vector<Arguments*> Argumented;

    public:
      typedef pointer_iterator<Argumented::iterator> ArgumentedIterator;
      typedef
      pointer_iterator<Argumented::const_iterator>
      ArgumentedConstIterator;

      ArgumentedIterator
      argumented_begin ()
      {
        return argumented_.begin ();
      }

      ArgumentedConstIterator
      argumented_begin () const
      {
        return argumented_.begin ();
      }

      ArgumentedIterator
      argumented_end ()
      {
        return argumented_.end ();
      }

      ArgumentedConstIterator
      argumented_end () const
      {
        return argumented_.end ();
      }

      // Shortcut for one-argument specializations.
      //
      Arguments&
      argumented () const
      {
        return *argumented_[0];
      }

    public:
      using Type::add_edge_right;

      void
      add_edge_right (Arguments& a)
      {
        argumented_.push_back (&a);
      }

      void
      add_edge_right (Arguments& a, ArgumentedIterator const& pos)
      {
        argumented_.insert (pos.base (), &a);
      }

      void
      remove_edge_right (Arguments&);

    private:
      Argumented argumented_;
    };

    class Arguments: public virtual Edge
    {
    public:
      Type&
      type () const
      {
        return *type_;
      }

      Specialization&
      specialization () const
      {
        return *specialization_;
      }

    public:
      void
      set_left_node (Type& n)
      {
        type_ = &n;
      }

      void
      clear_left_node (Type& n)
      {
        assert (type_ == &n);
        type_ = 0;
      }

      void
      set_right_node (Specialization& s)
      {
        specialization_ = &s;
      }

      void
      clear_right_node (Specialization& s)
      {
        assert (specialization_ == &s);
        specialization_ = 0;
      }

    private:
      Type* type_;
      Specialization* specialization_;
    };


    //
    //
    class AnyType: public virtual Type
    {
    public:
      AnyType (Path const& file, unsigned long line, unsigned long column)
          : Node (file, line, column)
      {
      }

    protected:
      AnyType () {} // For virtual inheritance.
    };


    //
    //
    class AnySimpleType: public virtual Type
    {
    public:
      AnySimpleType (Path const& file, unsigned long line, unsigned long column)
          : Node (file, line, column)
      {
      }

    protected:
      AnySimpleType () {} // For virtual inheritance.
    };
  }
}

// ADL won't find it because Path is a typedef. Note that this
// function prints in native format.
//
std::wostream&
operator<< (std::wostream& os, XSDFrontend::SemanticGraph::Path const& path);

#endif  // XSD_FRONTEND_SEMANTIC_GRAPH_ELEMENTS_HXX
