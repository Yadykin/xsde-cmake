// file      : xsd-frontend/semantic-graph/attribute-group.cxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cutl/compiler/type-info.hxx>

#include <xsd-frontend/semantic-graph/attribute-group.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    AttributeGroup::
    AttributeGroup (Path const& file, unsigned long line, unsigned long column)
        : Node (file, line, column)
    {
    }

    namespace
    {
      using compiler::type_info;

      struct AttributeGroupInit
      {
        AttributeGroupInit ()
        {
          type_info ti (typeid (AttributeGroup));
          ti.add_base (typeid (Scope));
          insert (ti);
        }
      } attribute_group_init_;
    }
  }
}
