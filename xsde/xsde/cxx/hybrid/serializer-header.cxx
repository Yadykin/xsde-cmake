// file      : xsde/cxx/hybrid/serializer-header.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <cxx/hybrid/serializer-header.hxx>

#include <xsd-frontend/semantic-graph.hxx>
#include <xsd-frontend/traversal.hxx>

namespace CXX
{
  namespace Hybrid
  {
    namespace
    {
      //
      //
      struct PreOverride: Traversal::Complex, Context
      {
        PreOverride (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (SemanticGraph::Complex& c)
        {
          if (c.inherits_p ())
          {
            SemanticGraph::Type& b (c.inherits ().base ());

            if (polymorphic (b))
            {
              if (tiein)
                dispatch (b);

              os << "virtual void" << endl
                 << "pre (" << sarg_type (b) << ");"
                 << endl;
            }
          }
        }
      };

      //
      //
      struct Enumeration: Traversal::Enumeration, Context
      {
        Enumeration (Context& c, Traversal::Complex& complex)
            : Context (c), complex_ (complex), pre_override_ (c)
        {
        }

        virtual void
        traverse (Type& e)
        {
          // First see if we should delegate this one to the Complex
          // generator.
          //
          Type* base_enum (0);

          if (!enum_ || !enum_mapping (e, &base_enum))
          {
            complex_.traverse (e);
            return;
          }

          String const& name (esimpl_custom (e));

          // We may not need to generate the class if this serializer is
          // being customized.
          //
          if (name)
          {
            String const& arg (sarg_type (e));
            SemanticGraph::Type& b (e.inherits ().base ());

            os << "class " << name << ": public " <<
              (mixin ? "virtual " : "") << esskel (e);

            // Derive from base simpl even if base_enum == 0. This is done
            // so that we have implementations for all the (otherwise pure
            // virtual) pre() functions.
            //
            if (mixin)
              os << "," << endl
                 << "  public " << fq_name (b, "s:impl");

            os << "{"
               << "public:" << endl;

            // c-tor
            //
            if (tiein)
              os << name << " ();"
                 << endl;

            // pre
            //
            if (polymorphic (e))
              pre_override_.dispatch (e);

            os << "virtual void" << endl
               << "pre (" << arg << ");"
               << endl;

            // _serialize_content
            //
            if (!base_enum)
              os << "virtual void" << endl
                 << "_serialize_content ();"
                 << endl;

            // State.
            //
            if (!base_enum)
            {
              os << (tiein ? "public:" : "protected:") << endl
                 << "const " << fq_name (e) << "* " << esstate (e) << ";";
            }
            else if (tiein)
            {
              os << "public:" << endl
                 << fq_name (b, "s:impl") << " base_impl_;";
            }

            os << "};";
          }

          // Generate include for custom serializer.
          //
          if (e.context ().count ("s:impl-include"))
          {
            close_ns ();

            os << "#include " << process_include_path (
              e.context ().get<String> ("s:impl-include")) << endl
               << endl;

            open_ns ();
          }
        }

      private:
        Traversal::Complex& complex_;
        PreOverride pre_override_;
      };

      //
      //
      struct List: Traversal::List, Context
      {
        List (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (Type& l)
        {
          String const& name (esimpl_custom (l));

          // We may not need to generate the class if this serializer is
          // being customized.
          //
          if (name)
          {
            String const& skel (esskel (l));
            String const& arg (sarg_type (l));
            String const& ret (sret_type (l.argumented ().type ()));

            String item (unclash (skel, "item"));
            String item_next (unclash (skel, "item_next"));

            os << "class " << name << ": public " <<
              (mixin ? "virtual " : "") << skel
               << "{"
               << "public:" << endl;

            // pre
            //
            os << "virtual void" << endl
               << "pre (" << arg << ");"
               << endl;

            // item_next
            //
            os << "virtual bool" << endl
               << item_next << " ();"
               << endl;

            // item
            //
            os << "virtual " << ret << endl
               << item << " ();"
               << endl;

            // State.
            //
            os << (tiein ? "public:" : "protected:") << endl;

            String const& type (fq_name (l));
            String const& state_type (esstate_type (l));

            os << "struct " << state_type
               << "{"
               << type << "::const_iterator i_;"
               << type << "::const_iterator end_;"
               << "};"
               << state_type << " " << esstate (l) << ";"
               << "};";
          }

          // Generate include for custom serializer.
          //
          if (l.context ().count ("s:impl-include"))
          {
            close_ns ();

            os << "#include " << process_include_path (
              l.context ().get<String> ("s:impl-include")) << endl
               << endl;

            open_ns ();
          }
        }
      };

      //
      //
      struct Union: Traversal::Union, Context
      {
        Union (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (Type& u)
        {
          String const& name (esimpl_custom (u));

          // We may not need to generate the class if this serializer is
          // being customized.
          //
          if (name)
          {
            String const& arg (sarg_type (u));

            os << "class " << name << ": public " <<
              (mixin ? "virtual " : "") << esskel (u)
               << "{"
               << "public:" << endl;

            // pre
            //
            os << "virtual void" << endl
               << "pre (" << arg << ");"
               << endl;

            // _serialize_content
            //
            os << "virtual void" << endl
               << "_serialize_content ();"
               << endl;

            // State.
            //
            os << (tiein ? "public:" : "protected:") << endl
               << "const " << fq_name (u) << "* " << esstate (u) << ";"
               << "};";
          }

          // Generate include for custom serializer.
          //
          if (u.context ().count ("s:impl-include"))
          {
            close_ns ();

            os << "#include " << process_include_path (
              u.context ().get<String> ("s:impl-include")) << endl
               << endl;

            open_ns ();
          }
        }
      };

      //
      // State.
      //

      struct CompositorState: Traversal::Compositor, Context
      {
        CompositorState (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (SemanticGraph::Compositor& c)
        {
          if (c.max () != 1)
          {
            String scope (fq_scope (c));
            String const& const_iterator (econst_iterator (c));

            os << scope << "::" << const_iterator << " " <<
              esstate_member (c) << ";"
               << scope << "::" << const_iterator << " " <<
              esstate_member_end (c) << ";";
          }

          Compositor::traverse (c);
        }
      };

      struct ParticleState: Traversal::Element, Context
      {
        ParticleState (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (SemanticGraph::Element& e)
        {
          if (e.max () != 1)
          {
            String scope (fq_scope (e));
            String const& const_iterator (econst_iterator (e));

            os << scope << "::" << const_iterator << " " <<
              esstate_member (e) << ";"
               << scope << "::" << const_iterator << " " <<
              esstate_member_end (e) << ";";
          }
        }
      };

      //
      // Callbacks.
      //

      struct CompositorCallback: Traversal::All,
                                 Traversal::Choice,
                                 Traversal::Sequence,
                                 Context
      {
        CompositorCallback (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (SemanticGraph::All& a)
        {
          // For the all compositor, maxOccurs=1 and minOccurs={0,1}.
          //
          if (a.min () == 0)
          {
            os << "virtual bool" << endl
               << espresent (a) << " ();"
               << endl;
          }

          Traversal::All::traverse (a);
        }

        virtual void
        traverse (SemanticGraph::Choice& c)
        {
          if (c.contains_begin () != c.contains_end ())
          {
            size_t min (c.min ()), max (c.max ());

            if (min == 0 && max == 1)
            {
              os << "virtual bool" << endl
                 << espresent (c) << " ();"
                 << endl;
            }
            else if (max != 1)
            {
              os << "virtual bool" << endl
                 << esnext (c) << " ();"
                 << endl;
            }

            os << "virtual " << esarm_tag (c) << endl
               << esarm (c) << " ();"
               << endl;

            Traversal::Choice::traverse (c);
          }
        }

        virtual void
        traverse (SemanticGraph::Sequence& s)
        {
          size_t min (s.min ()), max (s.max ());

          if (min == 0 && max == 1)
          {
            os << "virtual bool" << endl
               << espresent (s) << " ();"
               << endl;
          }
          else if (max != 1)
          {
            os << "virtual bool" << endl
               << esnext (s) << " ();"
               << endl;
          }

          Traversal::Sequence::traverse (s);
        }
      };

      struct ParticleCallback: Traversal::Element, Context
      {
        ParticleCallback (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (SemanticGraph::Element& e)
        {
          size_t min (e.min ()), max (e.max ());

          if (min == 0 && max == 1)
          {
            os << "virtual bool" << endl
               << espresent (e) << " ();"
               << endl;
          }
          else if (max != 1)
          {
            os << "virtual bool" << endl
               << esnext (e) << " ();"
               << endl;
          }

          String const& ret (sret_type (e.type ()));

          os << "virtual " << ret << endl
             << esname (e) << " ();"
             << endl;
        }
      };

      struct AttributeCallback: Traversal::Attribute, Context
      {
        AttributeCallback (Context& c)
            : Context (c)
        {
        }

        virtual void
        traverse (SemanticGraph::Attribute& a)
        {
          if (a.optional_p ())
          {
            os << "virtual bool" << endl
               << espresent (a) << " ();"
               << endl;
          }

          String const& ret (sret_type (a.type ()));

          os << "virtual " << ret << endl
             << esname (a) << " ();"
             << endl;
        }
      };

      //
      //
      struct Complex : Traversal::Complex, Context
      {
        Complex (Context& c)
            : Context (c),
              pre_override_ (c),

              // State.
              //
              compositor_state_ (c),
              particle_state_ (c),

              // Callbacks.
              //
              compositor_callback_ (c),
              particle_callback_ (c),
              attribute_callback_ (c)
        {
          // State.
          //
          contains_compositor_state_ >> compositor_state_;
          compositor_state_ >> contains_particle_state_;
          contains_particle_state_ >> compositor_state_;
          contains_particle_state_ >> particle_state_;


          // Callbacks.
          //
          contains_compositor_callback_ >> compositor_callback_;
          compositor_callback_ >> contains_particle_callback_;
          contains_particle_callback_ >> compositor_callback_;
          contains_particle_callback_ >> particle_callback_;

          names_attribute_callback_ >> attribute_callback_;
        }

        virtual void
        traverse (Type& c)
        {
          String const& name (esimpl_custom (c));

          // We may not need to generate the class if this serializer is
          // being customized.
          //
          if (name)
          {
            // In case of an inheritance-by-restriction, we don't need to
            // generate serializer callbacks, etc. since they are the same
            // as in the base. We only need the serialization/validation code.
            //
            bool restriction (restriction_p (c));

            bool hb (c.inherits_p ());
            bool he (has<Traversal::Element> (c));
            bool ha (has<Traversal::Attribute> (c));

            bool hae (has_particle<Traversal::Any> (c));
            bool haa (has<Traversal::AnyAttribute> (c));

            bool rec (recursive (c));

            String const& arg (sarg_type (c));

            os << "class " << name << ": public " <<
              (mixin ? "virtual " : "") << esskel (c);

            if (mixin && hb)
              os << "," << endl
                 << "  public " << fq_name (c.inherits ().base (), "s:impl");

            os << "{"
               << "public:" << endl;

            // c-tor
            //
            if ((rec && !restriction) || (tiein && hb))
              os << name << " ();"
                 << endl;

            // pre
            //
            if (polymorphic (c))
              pre_override_.dispatch (c);

            os << "virtual void" << endl
               << "pre (" << arg << ");"
               << endl;

            // Member callbacks.
            //
            if (!restriction && (ha || he || hae || haa))
            {
              if (ha || haa)
              {
                os << "// Attributes." << endl
                   << "//" << endl;

                names (c, names_attribute_callback_);
              }

              if (he || hae)
              {
                os << "// Elements." << endl
                   << "//" << endl;

                contains_compositor (c, contains_compositor_callback_);
              }
            }

            if (rec && !restriction)
            {
              // _post
              //
              if (hb && !recursive (c.inherits ().base ()))
                os << "virtual void" << endl
                   << "_post ();"
                   << endl;

              // post
              //
              os << "virtual void" << endl
                 << "post ();"
                 << endl;
            }

            // reset
            //
            if (reset && ((rec && !restriction) ||
                          (mixin && recursive_base (c))))
            {
              // If we are using mixin and this type has a base with _reset(),
              // then we need to provide _reset() in the whole hierarchy to
              // resolve an ambiguity.
              //
              os << "virtual void" << endl
                 << "_reset ();"
                 << endl;
            }

            if (tiein && hb)
              os << "public:" << endl
                 << fq_name (c.inherits ().base (), "s:impl") << " base_impl_;"
                 << endl;

            if (!restriction)
            {
              String const& state_type (esstate_type (c));
              String const& member (esstate_member (c));

              os << (tiein ? "public:" : "protected:") << endl
                 << "struct " << state_type
                 << "{"
                 << "const " << fq_name (c) << "* " << member << ";";

              if (c.contains_compositor_p ())
                contains_compositor (c, contains_compositor_state_);

              os << "};";

              if (rec)
              {
                os << state_type << " " << esstate_first (c) << ";"
                   << "::xsde::cxx::stack " << esstate (c) << ";";

                if (hb && !recursive (c.inherits ().base ()))
                  os << "bool " << esstate_top (c) << ";";
              }
              else
                os << state_type <<  " " << esstate (c) << ";";
            }

            os << "};";
          }

          // Generate include for custom serializer.
          //
          SemanticGraph::Context& ctx (c.context ());

          if (ctx.count ("s:impl-include"))
          {
            close_ns ();

            os << "#include " << process_include_path (
              ctx.get<String> ("s:impl-include")) << endl
               << endl;

            open_ns ();
          }
        }

      private:
        PreOverride pre_override_;

        // State.
        //
        CompositorState compositor_state_;
        ParticleState particle_state_;
        Traversal::ContainsCompositor contains_compositor_state_;
        Traversal::ContainsParticle contains_particle_state_;

        // Callbacks.
        //
        CompositorCallback compositor_callback_;
        ParticleCallback particle_callback_;
        Traversal::ContainsCompositor contains_compositor_callback_;
        Traversal::ContainsParticle contains_particle_callback_;

        AttributeCallback attribute_callback_;
        Traversal::Names names_attribute_callback_;
      };
    }

    void
    generate_serializer_header (Context& ctx)
    {
      ctx.os << "#include <xsde/cxx/stack.hxx>" << endl
             << endl;

      Traversal::Schema schema;

      Sources sources;
      Includes includes (ctx, Includes::impl_header);
      Traversal::Names schema_names;

      Namespace ns (ctx, true);
      Traversal::Names names;

      schema >> includes;
      schema >> sources >> schema;
      schema >> schema_names >> ns >> names;

      List list (ctx);
      Union union_ (ctx);
      Complex complex (ctx);
      Enumeration enumeration (ctx, complex);

      names >> list;
      names >> union_;
      names >> complex;
      names >> enumeration;

      schema.dispatch (ctx.schema_root);
    }
  }
}
