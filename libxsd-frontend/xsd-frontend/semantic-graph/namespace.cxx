// file      : xsd-frontend/semantic-graph/namespace.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cutl/compiler/type-info.hxx>

#include <xsd-frontend/semantic-graph/namespace.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    Namespace::
    Namespace (Path const& file, unsigned long line, unsigned long column)
        : Node (file, line, column)
    {
    }

    namespace
    {
      using compiler::type_info;

      struct NamespaceInit
      {
        NamespaceInit ()
        {
          type_info ti (typeid (Namespace));
          ti.add_base (typeid (Scope));
          insert (ti);
        }
      } namespace_init_;
    }
  }
}
