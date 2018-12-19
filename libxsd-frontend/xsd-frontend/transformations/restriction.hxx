// file      : xsd-frontend/transformations/restriction.hxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_TRANSFORMATIONS_RESTRICTION_HXX
#define XSD_FRONTEND_TRANSFORMATIONS_RESTRICTION_HXX

#include <xsd-frontend/types.hxx>

#include <xsd-frontend/semantic-graph/elements.hxx> // Path
#include <xsd-frontend/semantic-graph/schema.hxx>

namespace XSDFrontend
{
  namespace Transformations
  {
    // This transformation performs two major tasks. It transfers omitted
    // attribute declarations from the base to derived-by-restriction type
    // and establishes correspondence between particles and compositors by
    // adding the "xsd-frontend-restriction-correspondence" key-value pair
    // in the context that contains a pointer to the corresponding particle
    // or compositor in the base. Note that restriction of anyType is
    // a special case and is not handled by this transformation.
    //
    class Restriction
    {
    public:
      struct Failed {};

      void
      transform (SemanticGraph::Schema&, SemanticGraph::Path const&);
    };
  }
}

#endif // XSD_FRONTEND_TRANSFORMATIONS_RESTRICTION_HXX
