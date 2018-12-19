// file      : xsd-frontend/semantic-graph/complex.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cutl/compiler/type-info.hxx>

#include <xsd-frontend/semantic-graph/complex.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    Complex::
    Complex ()
        : abstract_ (false), mixed_ (false), contains_compositor_ (0)
    {
    }

    Complex::
    Complex (Path const& file,
             unsigned long line,
             unsigned long column,
             bool abstract)
        : Node (file, line, column),
          abstract_ (abstract), mixed_ (false), contains_compositor_ (0)
    {
    }

    namespace
    {
      using compiler::type_info;

      struct ComplexInit
      {
        ComplexInit ()
        {
          type_info ti (typeid (Complex));
          ti.add_base (typeid (Type));
          ti.add_base (typeid (Scope));
          insert (ti);
        }
      } complex_init_;
    }
  }
}
