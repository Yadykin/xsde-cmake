// file      : xsd-frontend/semantic-graph/enumeration.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cutl/compiler/type-info.hxx>

#include <xsd-frontend/semantic-graph/enumeration.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    // Enumeration
    //
    Enumeration::
    Enumeration (Path const& file, unsigned long line, unsigned long column)
        : Node (file, line, column)
    {
    }

    // Enumerator
    //
    Enumerator::
    Enumerator (Path const& file, unsigned long line, unsigned long column)
        : Node (file, line, column)
    {
    }

    namespace
    {
      using compiler::type_info;

      // Enumeration
      //
      struct EnumerationInit
      {
        EnumerationInit ()
        {
          type_info ti (typeid (Enumeration));
          ti.add_base (typeid (Complex));
          insert (ti);
        }
      } enumeration_init_;


      // Enumerator
      //
      struct EnumeratorInit
      {
        EnumeratorInit ()
        {
          type_info ti (typeid (Enumerator));
          ti.add_base (typeid (Instance));
          insert (ti);
        }
      } enumerator_init_;
    }
  }
}
