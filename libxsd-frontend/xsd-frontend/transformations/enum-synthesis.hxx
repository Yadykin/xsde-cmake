// file      : xsd-frontend/transformations/enum-synthesis.hxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_TRANSFORMATIONS_ENUM_SYNTHESIS_HXX
#define XSD_FRONTEND_TRANSFORMATIONS_ENUM_SYNTHESIS_HXX

#include <xsd-frontend/types.hxx>

#include <xsd-frontend/semantic-graph/elements.hxx> // Path
#include <xsd-frontend/semantic-graph/schema.hxx>

namespace XSDFrontend
{
  namespace Transformations
  {
    // This transformation replaces unions of one or more enumerations
    // with the same base with an equivalent synthesized enumeration.
    // This transformation assumes that there are no anonymous types.
    //
    class EnumSynthesis
    {
    public:
      void
      transform (SemanticGraph::Schema&, SemanticGraph::Path const&);
    };
  }
}

#endif // XSD_FRONTEND_TRANSFORMATIONS_ENUM_SYNTHESIS_HXX
