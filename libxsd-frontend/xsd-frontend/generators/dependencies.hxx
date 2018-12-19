// file      : xsd-frontend/generators/dependencies.hxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_GENERATORS_DEPENDENCIES_HXX
#define XSD_FRONTEND_GENERATORS_DEPENDENCIES_HXX

#include <vector>

#include <xsd-frontend/types.hxx>

#include <xsd-frontend/semantic-graph/elements.hxx> // Path
#include <xsd-frontend/semantic-graph/schema.hxx>

namespace XSDFrontend
{
  namespace Generators
  {
    // Return the list of included/imported schema paths (transitively and
    // including the main schema file) which can then be used to produce
    // make dependencies, etc.
    //
    class Dependencies
    {
    public:
      std::vector<SemanticGraph::Path>
      generate (SemanticGraph::Schema&, SemanticGraph::Path const&);
    };
  }
}

#endif // XSD_FRONTEND_GENERATORS_DEPENDENCIES_HXX
