// file      : xsd-frontend/traversal/particle.hxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_TRAVERSAL_PARTICLE_HXX
#define XSD_FRONTEND_TRAVERSAL_PARTICLE_HXX

#include <xsd-frontend/traversal/elements.hxx>
#include <xsd-frontend/semantic-graph/particle.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    struct Particle : Node<SemanticGraph::Particle>
    {
      virtual void
      traverse (Type&);

      virtual void
      pre (Type&);

      virtual void
      post (Type&);
    };
  }
}

#endif  // XSD_FRONTEND_TRAVERSAL_PARTICLE_HXX
