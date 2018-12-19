// file      : xsd-frontend/semantic-graph/complex.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_SEMANTIC_GRAPH_COMPLEX_HXX
#define XSD_FRONTEND_SEMANTIC_GRAPH_COMPLEX_HXX

#include <xsd-frontend/semantic-graph/elements.hxx>
#include <xsd-frontend/semantic-graph/compositors.hxx>

namespace XSDFrontend
{
  namespace SemanticGraph
  {
    class Complex: public virtual Type, public virtual Scope
    {
    public:
      bool
      abstract_p () const {return abstract_;}

      bool
      mixed_p () const
      {
        if (mixed_)
          return true;

        // If we have empty content, then we have the same content
        // type as our base.
        //
        if (!contains_compositor_p () && inherits_p ())
        {
          if (Complex* b = dynamic_cast<Complex*> (&inherits ().base ()))
            return b->mixed_p ();
        }

        return false;
      }

    public:
      bool
      contains_compositor_p () const
      {
        return contains_compositor_ != 0;
      }

      ContainsCompositor&
      contains_compositor ()
      {
        assert (contains_compositor_ != 0);
        return *contains_compositor_;
      }

    public:
      void
      mixed_p (bool m)
      {
        mixed_ = m;
      }

    public:
      Complex (Path const& file,
               unsigned long line,
               unsigned long column,
               bool abstract);

      void
      add_edge_left (ContainsCompositor& e)
      {
        contains_compositor_ = &e;
      }

      void
      remove_edge_left (ContainsCompositor& e)
      {
        assert (contains_compositor_ == &e);
        contains_compositor_ = 0;
      }

      using Type::add_edge_right;
      using Type::add_edge_left;
      using Scope::add_edge_left;

    protected:
      Complex (); // For virtual inheritance (Enumeration).

    private:
      bool abstract_;
      bool mixed_;
      ContainsCompositor* contains_compositor_;
    };
  }
}

#endif  // XSD_FRONTEND_SEMANTIC_GRAPH_COMPLEX_HXX
