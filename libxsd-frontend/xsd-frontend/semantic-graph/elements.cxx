// file      : xsd-frontend/semantic-graph/elements.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <algorithm>
#include <iostream>

#include <cutl/compiler/type-info.hxx>

#include <xsd-frontend/semantic-graph/elements.hxx>
#include <xsd-frontend/semantic-graph/annotation.hxx>

using namespace std;

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    // Node
    //
    Annotation& Node::
    annotation ()
    {
      return annotates_->annotation ();
    }

    // Type
    //
    void Type::
    remove_edge_left (Arguments& a)
    {
      ArgumentsSet::iterator i (arguments_.find (&a));
      assert (i != arguments_.end ());
      arguments_.erase (i);
    }

    // Specialization
    //
    void Specialization::
    remove_edge_right (Arguments& a)
    {
      // The number of entries should be small so linear search will do.
      //
      Argumented::iterator i (
        std::find (argumented_.begin (), argumented_.end (), &a));

      assert (i != argumented_.end ());
      argumented_.erase (i);
    }

    using compiler::type_info;

    namespace
    {
      // Edge
      //
      struct EdgeInit
      {
        EdgeInit ()
        {
          type_info ti (typeid (Edge));
          insert (ti);
        }
      } edge_init_;

      // Node
      //
      struct NodeInit
      {
        NodeInit ()
        {
          type_info ti (typeid (Node));
          insert (ti);
        }
      } node_init_;

      // Names
      //
      struct NamesInit
      {
        NamesInit ()
        {
          type_info ti (typeid (Names));
          ti.add_base (typeid (Edge));
          insert (ti);
        }
      } names_init_;

      // Nameable
      //
      struct NameableInit
      {
        NameableInit ()
        {
          type_info ti (typeid (Nameable));
          ti.add_base (typeid (Node));
          insert (ti);
        }
      } nameable_init_;

      // Scope
      //
      struct ScopeInit
      {
        ScopeInit ()
        {
          type_info ti (typeid (Scope));
          ti.add_base (typeid (Nameable));
          insert (ti);
        }
      } scope_init_;

      // Type
      //
      struct TypeInit
      {
        TypeInit ()
        {
          type_info ti (typeid (Type));
          ti.add_base (typeid (Nameable));
          insert (ti);
        }
      } type_init_;

      // Instance
      //
      struct InstanceInit
      {
        InstanceInit ()
        {
          type_info ti (typeid (Instance));
          ti.add_base (typeid (Nameable));
          insert (ti);
        }
      } instance_init_;

      // Belongs
      //
      struct BelongsInit
      {
        BelongsInit ()
        {
          type_info ti (typeid (Belongs));
          ti.add_base (typeid (Edge));
          insert (ti);
        }
      } belongs_init_;

      // Inherits
      //
      struct InheritsInit
      {
        InheritsInit ()
        {
          type_info ti (typeid (Inherits));
          ti.add_base (typeid (Edge));
          insert (ti);
        }
      } inherits_init_;

      // Extends
      //
      struct ExtendsInit
      {
        ExtendsInit ()
        {
          type_info ti (typeid (Extends));
          ti.add_base (typeid (Inherits));
          insert (ti);
        }
      } extends_init_;

      // Restricts
      //
      struct RestrictsInit
      {
        RestrictsInit ()
        {
          type_info ti (typeid (Restricts));
          ti.add_base (typeid (Inherits));
          insert (ti);
        }
      } restricts_init_;

      // BelongsToNamespace
      //
      struct BelongsToNamespaceInit
      {
        BelongsToNamespaceInit ()
        {
          type_info ti (typeid (BelongsToNamespace));
          ti.add_base (typeid (Edge));
          insert (ti);
        }
      } belongs_to_namespace_init_;

      // Member
      //
      struct MemberInit
      {
        MemberInit ()
        {
          type_info ti (typeid (Member));
          ti.add_base (typeid (Instance));
          insert (ti);
        }
      } member_init_;

      // Specialization
      //
      struct SpecializationInit
      {
        SpecializationInit ()
        {
          type_info ti (typeid (Specialization));
          ti.add_base (typeid (Type));
          insert (ti);
        }
      } specialization_init_;

      // Arguments
      //
      struct ArgumentsInit
      {
        ArgumentsInit ()
        {
          type_info ti (typeid (Arguments));
          ti.add_base (typeid (Edge));
          insert (ti);
        }
      } arguments_init_;

      /*
      // Contains
      //
      struct ContainsInit
      {
        ContainsInit ()
        {
          type_info ti (typeid (Contains));
          ti.add_base (typeid (Edge));
          insert (ti);
        }
      } contains_init_;

      // Container
      //
      struct ContainerInit
      {
        ContainerInit ()
        {
          type_info ti (typeid (Container));
          ti.add_base (typeid (Node));
          insert (ti);
        }
      } container_init_;
      */

      // AnyType
      //
      struct AnyTypeInit
      {
        AnyTypeInit ()
        {
          type_info ti (typeid (AnyType));
          ti.add_base (typeid (SemanticGraph::Type));
            insert (ti);
        }
      } any_type_init_;

      // AnySimpleType
      //
      struct AnySimpleTypeInit
      {
        AnySimpleTypeInit ()
        {
          type_info ti (typeid (AnySimpleType));
          ti.add_base (typeid (Type));
            insert (ti);
        }
      } any_simple_type_init_;
    }

    // Instance
    //
    Type& Instance::
    type () const
    {
      return belongs ().type ();
    }
  }
}

// Path
//
std::wostream&
operator<< (std::wostream& os, XSDFrontend::SemanticGraph::Path const& path)
{
  return os << path.string ().c_str ();
}
