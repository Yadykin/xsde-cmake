// file      : xsd-frontend/semantic-graph/element-group.cxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cutl/compiler/type-info.hxx>

#include <xsd-frontend/semantic-graph/element-group.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    ElementGroup::
    ElementGroup (Path const& file, unsigned long line, unsigned long column)
        : Node (file, line, column), contains_compositor_ (0)
    {
    }

    namespace
    {
      using compiler::type_info;

      struct ElementGroupInit
      {
        ElementGroupInit ()
        {
          type_info ti (typeid (ElementGroup));
          ti.add_base (typeid (Scope));
          insert (ti);
        }
      } element_group_init_;
    }
  }
}
