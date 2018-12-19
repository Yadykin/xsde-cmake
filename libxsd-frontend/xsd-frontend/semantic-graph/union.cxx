// file      : xsd-frontend/semantic-graph/union.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cutl/compiler/type-info.hxx>

#include <xsd-frontend/semantic-graph/union.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    Union::
    Union (Path const& file, unsigned long line, unsigned long column)
        : Node (file, line, column)
    {
    }

    namespace
    {
      using compiler::type_info;

      struct UnionInit
      {
        UnionInit ()
        {
          type_info ti (typeid (Union));
          ti.add_base (typeid (Specialization));
          insert (ti);
        }
      } union_init_;
    }
  }
}
