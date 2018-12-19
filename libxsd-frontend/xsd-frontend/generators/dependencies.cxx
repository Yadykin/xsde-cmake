// file      : xsd-frontend/generators/dependencies.cxx
// copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsd-frontend/semantic-graph.hxx>
#include <xsd-frontend/traversal.hxx>

#include <xsd-frontend/generators/dependencies.hxx>

namespace XSDFrontend
{
  typedef std::vector<SemanticGraph::Path> Paths;

  namespace
  {
    // Go into included/imported (but not implied) schemas while making
    // sure we don't process the same stuff more than once.
    //
    struct Uses: Traversal::Uses
    {
      Uses (Paths& p): paths_ (p) {}

      virtual void
      traverse (Type& u)
      {
        if (u.is_a<SemanticGraph::Implies> ())
          return;

        SemanticGraph::Schema& s (u.schema ());

        if (!s.context ().count ("xsd-frontend-dependencies-seen"))
        {
          s.context ().set ("xsd-frontend-dependencies-seen", true);

          // While the edge contains the exact path that was found in the
          // schema, the schema node itself has the reative to the including
          // or importing schema path, which is what we want.
          //
          paths_.push_back (s.file ());
          Traversal::Uses::traverse (u);
        }
      }

    private:
      Paths& paths_;
    };
  }

  namespace Generators
  {
    Paths Dependencies::
    generate (SemanticGraph::Schema& s, SemanticGraph::Path const& p)
    {
      Paths r;
      r.push_back (p);

      Traversal::Schema schema;
      Uses uses (r);

      schema >> uses >> schema;

      // Some twisted schemas do recusive inclusions.
      //
      s.context ().set ("xsd-frontend-dependencies-seen", true);

      schema.dispatch (s);
      return r;
    }
  }
}
