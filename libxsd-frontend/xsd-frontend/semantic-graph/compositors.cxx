// file      : xsd-frontend/semantic-graph/compositor.cxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cutl/compiler/type-info.hxx>

#include <xsd-frontend/semantic-graph/compositors.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    // ContainsCompositor
    //
    ContainsCompositor::
    ContainsCompositor (unsigned long min, unsigned long max)
        : compositor_ (0), container_ (0), min_ (min), max_ (max)
    {
    }

    // All
    //
    All::
    All (Path const& file, unsigned long line, unsigned long column)
        : Node (file, line, column)
    {
    }

    // Choice
    //
    Choice::
    Choice (Path const& file, unsigned long line, unsigned long column)
        : Node (file, line, column)
    {
    }

    // Sequence
    //
    Sequence::
    Sequence (Path const& file, unsigned long line, unsigned long column)
        : Node (file, line, column)
    {
    }

    namespace
    {
      using compiler::type_info;

      struct ContainsCompositorInit
      {
        ContainsCompositorInit ()
        {
          type_info ti (typeid (ContainsCompositor));
          ti.add_base (typeid (Edge));
          insert (ti);
        }
      } contains_compositor_init_;

      struct CompositorInit
      {
        CompositorInit ()
        {
          type_info ti (typeid (Compositor));
          ti.add_base (typeid (Particle));
          insert (ti);
        }
      } compositor_init_;

      struct AllInit
      {
        AllInit ()
        {
          type_info ti (typeid (All));
          ti.add_base (typeid (Compositor));
          insert (ti);
        }
      } all_init_;

      struct ChoiceInit
      {
        ChoiceInit ()
        {
          type_info ti (typeid (Choice));
          ti.add_base (typeid (Compositor));
          insert (ti);
        }
      } choice_init_;

      struct SequenceInit
      {
        SequenceInit ()
        {
          type_info ti (typeid (Sequence));
          ti.add_base (typeid (Compositor));
          insert (ti);
        }
      } sequence_init_;
    }
  }
}
