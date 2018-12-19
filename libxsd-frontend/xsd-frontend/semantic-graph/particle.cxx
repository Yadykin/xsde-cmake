// file      : xsd-frontend/semantic-graph/particle.cxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cutl/compiler/type-info.hxx>

#include <xsd-frontend/semantic-graph/particle.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    // ContainsParticle
    //
    ContainsParticle::
    ContainsParticle (unsigned long min, unsigned long max)
        : particle_ (0), compositor_ (0), min_ (min), max_ (max)
    {
    }

    // Particle
    //
    Particle::
    Particle ()
        : contained_particle_ (0)
    {
    }

    namespace
    {
      using compiler::type_info;

      struct ContainsParticleInit
      {
        ContainsParticleInit ()
        {
          type_info ti (typeid (ContainsParticle));
          ti.add_base (typeid (Edge));
          insert (ti);
        }
      } contains_particle_init_;

      struct ParticleInit
      {
        ParticleInit ()
        {
          type_info ti (typeid (Particle));
          ti.add_base (typeid (Node));
          insert (ti);
        }
      } particle_init_;
    }
  }
}
