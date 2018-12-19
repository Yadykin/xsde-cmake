// file      : xsd-frontend/semantic-graph/schema.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cutl/compiler/type-info.hxx>

#include <xsd-frontend/semantic-graph/schema.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    // Schema
    //
    Schema::NamesIteratorPair Schema::
    find (Name const& name) const
    {
      // Here we are going to create an illusion that the namespace
      // hierarchy is flat.
      names_.clear ();
      schemas_.clear ();

      find_ (name, names_, schemas_);

      return NamesIteratorPair (NamesConstIterator (names_.begin ()),
                                NamesConstIterator (names_.end ()));
    }

    void Schema::
    find_ (Name const& name, NamesList& names, SchemaSet& set) const
    {
      set.insert (this);

      // Check our own namespace first so it will end up first in the list.
      //
      NamesIteratorPair pair (Scope::find (name));
      names.insert (names.end (), pair.first.base (), pair.second.base ());

      for (UsesIterator i (uses_begin ()), end (uses_end ()); i != end; ++i)
      {
        Schema& s (i->schema ());

        if (set.find (&s) == set.end ())
          s.find_ (name, names, set);
      }
    }

    namespace
    {
      using compiler::type_info;

      // Uses
      //
      struct UsesInit
      {
        UsesInit ()
        {
          type_info ti (typeid (Uses));
          ti.add_base (typeid (Edge));
          insert (ti);
        }
      } uses_init_;


      // Implies
      //
      struct ImpliesInit
      {
        ImpliesInit ()
        {
          type_info ti (typeid (Implies));
          ti.add_base (typeid (Uses));
          insert (ti);
        }
      } implies_init_;


      // Sources
      //
      struct SourcesInit
      {
        SourcesInit ()
        {
          type_info ti (typeid (Sources));
          ti.add_base (typeid (Uses));
          insert (ti);
        }
      } sources_init_;


      // Includes
      //
      struct IncludesInit
      {
        IncludesInit ()
        {
          type_info ti (typeid (Includes));
          ti.add_base (typeid (Uses));
          insert (ti);
        }
      } includes_init_;


      // Imports
      //
      struct ImportsInit
      {
        ImportsInit ()
        {
          type_info ti (typeid (Imports));
          ti.add_base (typeid (Uses));
          insert (ti);
        }
      } imports_init_;


      // Schema
      //
      struct SchemaInit
      {
        SchemaInit ()
        {
          type_info ti (typeid (Schema));
          ti.add_base (typeid (Scope));
          insert (ti);
        }
      } schema_init_;
    }
  }
}
