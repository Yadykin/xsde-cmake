// file      : xsd-frontend/semantic-graph/list.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cutl/compiler/type-info.hxx>

#include <xsd-frontend/semantic-graph/list.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    List::
    List (Path const& file, unsigned long line, unsigned long column)
        : Node (file, line, column)
    {
    }

    namespace
    {
      using compiler::type_info;

      struct ListInit
      {
        ListInit ()
        {
          type_info ti (typeid (List));
          ti.add_base (typeid (Specialization));
          insert (ti);
        }
      } list_init_;
    }
  }
}
