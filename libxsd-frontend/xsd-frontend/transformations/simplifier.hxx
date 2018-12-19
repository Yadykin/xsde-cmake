// file      : xsd-frontend/transformations/simplifier.hxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_TRANSFORMATIONS_SIMPLIFIER_HXX
#define XSD_FRONTEND_TRANSFORMATIONS_SIMPLIFIER_HXX

#include <xsd-frontend/types.hxx>

#include <xsd-frontend/semantic-graph/elements.hxx> // Path
#include <xsd-frontend/semantic-graph/schema.hxx>

namespace XSDFrontend
{
  namespace Transformations
  {
    // This transformation performs various schema simplifications
    // (e.g., removing empty compositors, etc). This transformation
    // assumes that there are no anonymous types.
    //
    class Simplifier
    {
    public:
      void
      transform (SemanticGraph::Schema&, SemanticGraph::Path const&);
    };
  }
}

#endif // XSD_FRONTEND_TRANSFORMATIONS_SIMPLIFIER_HXX
