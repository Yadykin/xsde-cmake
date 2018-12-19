// file      : xsd-frontend/parser.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <map>
#include <stack>
#include <vector>
#include <iostream>
#include <sstream>

#include <cutl/compiler/type-id.hxx>

#include <xsd-frontend/version.hxx> // Check Xerces-C++ version.
#include <xsd-frontend/xml.hxx>
#include <xsd-frontend/parser.hxx>
#include <xsd-frontend/schema-dom-parser.hxx>

#include <xsd-frontend/semantic-graph.hxx>
#include <xsd-frontend/traversal.hxx>

//@@ Do i need this?
//
#include <xercesc/dom/DOM.hpp>

#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/util/BinFileInputStream.hpp>

#include <xercesc/validators/common/Grammar.hpp>

#include <xercesc/sax/InputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>

using namespace std;

using cutl::compiler::type_id;

namespace XSDFrontend
{
  namespace Xerces = XML::Xerces;
  using namespace SemanticGraph;

  //@@ Port to tracing facility.
  //
  bool trace_ = false;

  String const xsd = L"http://www.w3.org/2001/XMLSchema";
  String const xse = L"http://www.codesynthesis.com/xmlns/xml-schema-extension";

  namespace
  {
    //
    // Exceptions.
    //

    struct NotNamespace
    {
      NotNamespace (String const& ns)
          : ns_ (ns)
      {
      }

      String const&
      ns () const
      {
        return ns_;
      }

    private:
      String ns_;
    };

    struct NotName
    {
      NotName (String const& ns, String const& name)
          : ns_ (ns), name_ (name)
      {
      }

      String const&
      ns () const
      {
        return ns_;
      }

      String const&
      name () const
      {
        return name_;
      }

    private:
      String ns_;
      String name_;
    };

    // Trim leading and trailing whitespaces.
    //
    template <typename C>
    StringTemplate<C>
    trim (StringTemplate<C> const& s)
    {
      typedef StringTemplate<C> String;

      size_t size (s.size ());

      if (size == 0)
        return s;

      C const* f (s.c_str ());
      C const* l (f + size);

      C const* of (f);

      while (f < l &&
             (*f == C (0x20) || *f == C (0x0A) ||
              *f == C (0x0D) || *f == C (0x09)))
        ++f;

      --l;

      C const* ol (l);

      while (l > f &&
             (*l == C (0x20) || *l == C (0x0A) ||
              *l == C (0x0D) || *l == C (0x09)))
        --l;

      if (f != of || l != ol)
        return f <= l ? String (f, l - f + 1) : String ();
      else
        return s;
    }

    // Name cache. We only support maximum two nodes with the same
    // name in the cache (e.g., element and type). For (rare) cases
    // where there is three or more names, there will be a cache miss.
    //
    struct CacheNodes
    {
      CacheNodes () : first (0), second (0) {}

      Nameable* first;
      Nameable* second;
    };

    typedef std::map<String, CacheNodes> NodeMap;
    typedef std::map<String, NodeMap> NamespaceMap;
    typedef std::vector<SemanticGraph::Member*> DefaultValues;

    template <typename X>
    X&
    resolve (String const& ns_name,
             String const& uq_name,
             Schema& s_,
             NamespaceMap& cache)
    {
      // First check the cache.
      //
      NamespaceMap::iterator i (cache.find (ns_name));

      if (i != cache.end ())
      {
        NodeMap::iterator j (i->second.find (uq_name));

        if (j != i->second.end ())
        {
          X* x;

          if ((x = dynamic_cast<X*> (j->second.first)) ||
              (x = dynamic_cast<X*> (j->second.second)))
              return *x;
        }
      }

      Scope::NamesIteratorPair nss (s_.find (ns_name));

      if (nss.first == nss.second)
        throw NotNamespace (ns_name);

      for (; nss.first != nss.second; ++nss.first)
      {
        Namespace& ns (dynamic_cast<Namespace&> (nss.first->named ()));

        Scope::NamesIteratorPair types (ns.find (uq_name));

        for (; types.first != types.second; ++types.first)
        {
          if (X* x = dynamic_cast<X*> (&types.first->named ()))
          {
            if (trace_)
              wcout << "successfully resolved '" << ns_name << '#' << uq_name
                    << "'" << endl;

            // Add to the cache if there are free slots.
            //
            NodeMap& m (i != cache.end () ? i->second : cache[ns_name]);
            CacheNodes& n (m[uq_name]);

            if (n.first == 0)
              n.first = x;
            else if (n.second == 0)
              n.second = x;

            return *x;
          }
        }
      }

      throw NotName (ns_name, uq_name);
    }

    //
    //
    typedef std::map<String, String> Facets;

    void
    copy_facets (Restricts& r, Facets const& f)
    {
      for (Facets::const_iterator i (f.begin ()), e (f.end ()); i != e; ++i)
        r.facet_insert (i->first, i->second);
    }

    //
    //
    struct UnionMemberType
    {
      UnionMemberType (String const& ns, String const& uq)
          : ns_name (ns), uq_name (uq)
      {
      }

      String ns_name;
      String uq_name;
    };

    typedef std::vector<UnionMemberType> UnionMemberTypes;

    //
    //
    struct ElementGroupRef
    {
      ElementGroupRef (String const& uq_name_, String const& ns_name_,
                       unsigned long min_, unsigned long max_,
                       Compositor& compositor, Scope& scope)
          : uq_name (uq_name_), ns_name (ns_name_),
            min (min_), max (max_)
      {
        contains_pos  = compositor.contains_end ();
        if (compositor.contains_begin () != contains_pos)
          --contains_pos;

        names_pos = scope.names_end ();
        if (scope.names_begin () != names_pos)
          --names_pos;
      }

      ElementGroupRef (String const& uq_name_, String const& ns_name_,
                       unsigned long min_, unsigned long max_,
                       Scope& scope)
          : uq_name (uq_name_), ns_name (ns_name_),
            min (min_), max (max_)
      {
        names_pos  = scope.names_end ();
        if (scope.names_begin () != names_pos)
          --names_pos;
      }

      String uq_name;
      String ns_name;
      unsigned long min, max;
      Compositor::ContainsIterator contains_pos;
      Scope::NamesIterator names_pos;
    };

    typedef std::vector<ElementGroupRef> ElementGroupRefs;

    //
    //
    struct AttributeGroupRef
    {
      AttributeGroupRef (String const& uq_name_,
                         String const& ns_name_,
                         Scope& scope)
          : uq_name (uq_name_), ns_name (ns_name_)
      {
        names_pos = scope.names_end ();
        if (scope.names_begin () != names_pos)
          --names_pos;
      }

      String uq_name;
      String ns_name;
      Scope::NamesIterator names_pos;
    };

    typedef std::vector<AttributeGroupRef> AttributeGroupRefs;


    //
    //
    template <typename N, typename A>
    struct NodeArgs
    {
      NodeArgs (N& node, A arg)
          : node_ (node), arg_ (arg)
      {
      }

      operator N& () const
      {
        return node_;
      }

      template <typename E>
      void
      add_edge_left (E& e)
      {
        node_.add_edge_left (e, arg_);
      }

      template <typename E>
      void
      add_edge_right (E& e)
      {
        node_.add_edge_right (e, arg_);
      }

    private:
      N& node_;
      A arg_;
    };


    //
    //
    struct Resolver : Traversal::Element,
                      Traversal::Attribute,
                      Traversal::Fundamental::IdRef,
                      Traversal::Fundamental::IdRefs,
                      Traversal::List,
                      Traversal::Union,
                      Traversal::Complex,
                      Traversal::Enumeration,
                      Traversal::ElementGroup,
                      Traversal::AttributeGroup,
                      Traversal::Compositor
    {
      Resolver (Schema& s,
                bool& valid,
                NamespaceMap& cache,
                DefaultValues& default_values)
          : s_ (s),
            valid_ (valid),
            cache_ (cache),
            default_values_ (default_values)
      {
        *this >> contains_compositor >> *this;
      }

      void
      traverse (SemanticGraph::Attribute& a)
      {
        // Avoid traversing attribute more than once.
        //
        if (!a.context ().count ("attribute-traversed"))
        {
          a.context ().set ("attribute-traversed", true);
          SemanticGraph::Member& m (a);
          resolve_member (m);
        }
      }

      void
      traverse (SemanticGraph::Element& e)
      {
        resolve_element (e);
      }

      void
      resolve_element (SemanticGraph::Element& e)
      {
        // Avoid resolving element more than once.
        //
        if (e.context ().count ("element-resolved"))
          return;

        e.context ().set ("element-resolved", true);

        {
          SemanticGraph::Member& m (e);
          resolve_member (m);
        }

        if (e.context ().count ("substitution-ns-name"))
        {
          String ns_name (e.context ().get<String> ("substitution-ns-name"));
          String uq_name (e.context ().get<String> ("substitution-uq-name"));

          e.context ().remove ("substitution-ns-name");
          e.context ().remove ("substitution-uq-name");

          try
          {
            SemanticGraph::Element& root (
              resolve<SemanticGraph::Element> (ns_name, uq_name, s_, cache_));

            s_.new_edge<Substitutes> (e, root);

            // See if we need to derive the type of this element from the
            // one it substitutes.
            //
            if (!e.typed_p ())
            {
              resolve_element (root); // Make sure the type is resolved.
              s_.new_edge<Belongs> (e, root.type ());
            }
          }
          catch (NotNamespace const& ex)
          {
            if (valid_)
            {
              wcerr << "ice: unable to resolve namespace '" << ex.ns () << "'"
                    << endl;
              abort ();
            }
          }
          catch (NotName const& ex)
          {
            if (valid_)
            {
              wcerr << "ice: unable to resolve name '" << ex.name ()
                    << "' inside namespace '" << ex.ns () << "'" <<endl;
              abort ();
            }
          }
        }
      }

      void
      resolve_member (SemanticGraph::Member& m)
      {
        using SemanticGraph::Member;
        using SemanticGraph::Element;
        using SemanticGraph::Attribute;

        try
        {
          String ns_name;
          String uq_name;

          if (m.context ().count ("type-ns-name"))
          {
            ns_name = m.context ().get<String> ("type-ns-name");
            uq_name = m.context ().get<String> ("type-uq-name");

            m.context ().remove ("type-ns-name");
            m.context ().remove ("type-uq-name");
            m.context ().remove ("edge-type-id");

            s_.new_edge<Belongs> (
              m, resolve<SemanticGraph::Type> (ns_name, uq_name, s_, cache_));
          }
          else if (m.context ().count ("instance-ns-name"))
          {
            ns_name = m.context ().get<String> ("instance-ns-name");
            uq_name = m.context ().get<String> ("instance-uq-name");

            m.context ().remove ("instance-ns-name");
            m.context ().remove ("instance-uq-name");

            // Resolve the name to the same type. It is legal to have
            // an element and an attribute with the same name.
            //
            Member& ref (
              m.is_a<Element> ()
              ? static_cast<Member&> (
                  resolve<Element> (ns_name, uq_name, s_, cache_))
              : static_cast<Member&> (
                  resolve<Attribute> (ns_name, uq_name, s_, cache_)));

            // Make sure the referenced member is fully resolved.
            // @@ Substitutes edge won't be resolved.
            //
            resolve_member (ref);


            // Substitution group info. We have to test for both resolved
            // and unresolved cases since we don't know whether it was
            // resolved or not.
            //
            if (ref.is_a<Element> ())
            {
              Element& m_e (dynamic_cast<Element&> (m));
              Element& ref_e (dynamic_cast<Element&> (ref));

              if (ref_e.substitutes_p ())
              {
                s_.new_edge<Substitutes> (m_e, ref_e.substitutes ().root ());
              }
              else if (ref_e.context ().count ("substitution-ns-name"))
              {
                m_e.context ().set (
                  "substitution-ns-name",
                  ref_e.context ().get<String> ("substitution-ns-name"));

                m_e.context ().set (
                  "substitution-uq-name",
                  ref_e.context ().get<String> ("substitution-uq-name"));
              }
            }

            //
            //
            s_.new_edge<BelongsToNamespace> (m, ref.namespace_ ());

            // Transfer default and fixed values if we haven't already
            // gotten them.
            //
            if (!m.default_p ())
            {
              if (ref.fixed_p ())
                m.fixed (ref.value ());
              else if (ref.default_p ())
              {
                // Default value applies only if the attribute is optional.
                //
                if (Attribute* a = dynamic_cast<Attribute*> (&m))
                {
                  if (a->optional_p ())
                    m.default_ (ref.value ());
                }
                else
                  m.default_ (ref.value ());
              }

              if (m.default_p ())
              {
                m.context ().set (
                  "dom-node",
                  ref.context ().get<Xerces::DOMElement*> ("dom-node"));
                default_values_.push_back (&m);
              }
            }

            // Transfer annotation if we haven't already gotten it.
            //
            if (!m.annotated_p () && ref.annotated_p ())
              s_.new_edge<Annotates> (ref.annotation (), m);

            // Type info. Can be missing for a substitution group member.
            //
            if (ref.typed_p ())
              s_.new_edge<Belongs> (m, ref.type ());
          }
        }
        catch (NotNamespace const& ex)
        {
          if (valid_)
          {
            wcerr << "ice: unable to resolve namespace '" << ex.ns () << "'"
                  << endl;
            abort ();
          }
        }
        catch (NotName const& ex)
        {
          if (valid_)
          {
            wcerr << "ice: unable to resolve name '" << ex.name ()
                  << "' inside namespace '" << ex.ns () << "'" <<endl;
            abort ();
          }
        }
      }

      void
      traverse (SemanticGraph::Fundamental::IdRef& i)
      {
        ref_type (i);
      }

      void
      traverse (SemanticGraph::Fundamental::IdRefs& i)
      {
        ref_type (i);
      }

      void
      ref_type (SemanticGraph::Specialization& s)
      {
        if (s.context ().count ("type-ns-name"))
        {
          String ns_name (s.context ().get<String> ("type-ns-name"));
          String uq_name (s.context ().get<String> ("type-uq-name"));

          s.context ().remove ("type-ns-name");
          s.context ().remove ("type-uq-name");
          s.context ().remove ("edge-type-id");

          try
          {
            s_.new_edge<Arguments> (
              resolve<SemanticGraph::Type> (ns_name, uq_name, s_, cache_), s);
          }
          catch (NotName const& ex)
          {
            wcerr << s.file () << ":" << s.line () << ":" << s.column () << ": "
                  << "error: unable to resolve type '" << uq_name << "' "
                  << "in namespace '" << ns_name << "'" << endl;

            valid_ = false;
          }
        }
      }

      void
      traverse (SemanticGraph::List& l)
      {
        if (l.context ().count ("type-ns-name"))
        {
          String ns_name (l.context ().get<String> ("type-ns-name"));
          String uq_name (l.context ().get<String> ("type-uq-name"));

          l.context ().remove ("type-ns-name");
          l.context ().remove ("type-uq-name");
          l.context ().remove ("edge-type-id");

          try
          {
            s_.new_edge<Arguments> (
              resolve<SemanticGraph::Type> (ns_name, uq_name, s_, cache_), l);
          }
          catch (NotName const& ex)
          {
            wcerr << l.file () << ":" << l.line () << ":" << l.column () << ": "
                  << "error: unable to resolve item type '" << uq_name << "' "
                  << "in namespace '" << ns_name << "'" << endl;

            valid_ = false;
          }
        }

        Traversal::List::traverse (l);
      }

      void
      traverse (SemanticGraph::Union& u)
      {
        using SemanticGraph::Union;

        if (u.context ().count ("union-member-types"))
        {
          UnionMemberTypes const& m (
            u.context ().get<UnionMemberTypes> ("union-member-types"));

          // Process it backwards so that we can just insert each
          // edge in the front.
          //
          for (UnionMemberTypes::const_reverse_iterator i (m.rbegin ());
               i != m.rend (); i++)
          {
            try
            {
              NodeArgs<Union, Union::ArgumentedIterator> na (
                u, u.argumented_begin ());

              s_.new_edge<Arguments> (
                resolve<SemanticGraph::Type> (
                  i->ns_name, i->uq_name, s_, cache_), na);
            }
            catch (NotName const& ex)
            {
              wcerr << u.file () << ":" << u.line () << ":" << u.column () << ": "
                    << "error: unable to resolve item type '" << i->uq_name << "' "
                    << "in namespace '" << i->ns_name << "'" << endl;

              valid_ = false;
            }
          }

          u.context ().remove ("union-member-types");
        }

        Traversal::Union::traverse (u);
      }

      void
      traverse (SemanticGraph::Complex& c)
      {
        // Avoid traversing complex type more than once.
        //
        if (c.context ().count ("complex-type-resolved"))
          return;

        c.context ().set ("complex-type-resolved", true);

        // Resolve base type if any.
        //
        if (c.context ().count ("type-ns-name"))
        {
          String ns_name (c.context ().get<String> ("type-ns-name"));
          String uq_name (c.context ().get<String> ("type-uq-name"));
          type_id edge_id (c.context ().get<type_id> ("edge-type-id"));

          c.context ().remove ("type-ns-name");
          c.context ().remove ("type-uq-name");
          c.context ().remove ("edge-type-id");

          try
          {
            if (edge_id == typeid (Extends))
            {
              s_.new_edge<Extends> (
                c, resolve<SemanticGraph::Type> (
                  ns_name, uq_name, s_, cache_));
            }
            else if (edge_id == typeid (Restricts))
            {
              Restricts& r (
                s_.new_edge<Restricts> (
                  c, resolve<SemanticGraph::Type> (
                    ns_name, uq_name, s_, cache_)));

              if (c.context ().count ("facets"))
              {
                Facets const& f (c.context ().get<Facets> ("facets"));
                copy_facets (r, f);
                c.context ().remove ("facets");
              }
            }
            else
              assert (false);
          }
          catch (NotName const& ex)
          {
            wcerr << c.file () << ":" << c.line () << ":" << c.column () << ": "
                  << "error: unable to resolve base type '" << uq_name << "' "
                  << "in namespace '" << ns_name << "'" << endl;

            valid_ = false;
          }
        }

        // Resolve attribute-group-refs. Do it before element-group-refs
        // so that if the scope was empty they end up at the end.
        //
        if (c.context ().count ("attribute-group-refs"))
        {
          AttributeGroupRefs& refs (
            c.context ().get<AttributeGroupRefs> ("attribute-group-refs"));

          // Handle refs from last to first so that multiple insertions
          // to an empty list (always front) end up in proper order.
          //
          for (AttributeGroupRefs::reverse_iterator i (refs.rbegin ());
               i != refs.rend (); ++i)
          {
            clone_attribute_group_content (*i, c);
          }

          c.context ().remove ("attribute-group-refs");
        }

        // Resolve element-group-ref if any.
        //
        if (c.context ().count ("element-group-ref"))
        {
          using SemanticGraph::Compositor;

          ElementGroupRef& ref (
            c.context ().get<ElementGroupRef> ("element-group-ref"));

          Compositor* comp (clone_element_group_content (c, ref));

          // Create ContainsCompositor edge.
          //
          if (comp)
            s_.new_edge<ContainsCompositor> (c, *comp, ref.min, ref.max);

          c.context ().remove ("element-group-ref");
        }

        Traversal::Complex::traverse (c);
      }

      void
      traverse (SemanticGraph::Enumeration& e)
      {
        // Resolve base type if any.
        //
        if (e.context ().count ("type-ns-name"))
        {
          String ns_name (e.context ().get<String> ("type-ns-name"));
          String uq_name (e.context ().get<String> ("type-uq-name"));

          e.context ().remove ("type-ns-name");
          e.context ().remove ("type-uq-name");
          e.context ().remove ("edge-type-id");

          try
          {
            Restricts& r (
              s_.new_edge<Restricts> (
                e, resolve<SemanticGraph::Type> (
                  ns_name, uq_name, s_, cache_)));

            if (e.context ().count ("facets"))
            {
              Facets const& f (e.context ().get<Facets> ("facets"));
              copy_facets (r, f);
              e.context ().remove ("facets");
            }
          }
          catch (NotName const& ex)
          {
            wcerr << e.file () << ":" << e.line () << ":" << e.column () << ": "
                  << "error: unable to resolve base type '" << uq_name << "' "
                  << "in namespace '" << ns_name << "'" << endl;

            valid_ = false;
          }
        }

        Traversal::Enumeration::traverse (e);
      }

      void
      traverse (SemanticGraph::ElementGroup& g)
      {
        // Avoid traversing groups more than once.
        //
        if (!g.context ().count ("element-group-traversed"))
        {
          g.context ().set ("element-group-traversed", true);
          Traversal::ElementGroup::traverse (g);

          // Note that setting element-group-resolved after traversing
          // the group allows for a recursive shallow resolution using
          // resolve_element_group.
          //
          g.context ().set ("element-group-resolved", true);
        }
      }

      // We need a "shallow" resolve to break possible recursing:
      // group->element->complexType->group.
      //
      void
      resolve_element_group (SemanticGraph::ElementGroup& g)
      {
        using SemanticGraph::Scope;
        using SemanticGraph::Element;

        // Avoid resolving groups more than once.
        //
        if (!g.context ().count ("element-group-resolved"))
        {
          g.context ().set ("element-group-resolved", true);

          for (Scope::NamesIterator i (g.names_begin ());
               i != g.names_end (); ++i)
          {
            if (Element* e = dynamic_cast<Element*> (&i->named ()))
              resolve_element (*e);
          }

          traverse (g.contains_compositor ().compositor ());
        }
      }

      void
      traverse (SemanticGraph::AttributeGroup& g)
      {
        // Avoid traversing groups more than once.
        //
        if (g.context ().count ("attribute-group-resolved"))
          return;

        g.context ().set ("attribute-group-resolved", true);

        // Resolve attribute-group-refs.
        //
        if (g.context ().count ("attribute-group-refs"))
        {
          AttributeGroupRefs& refs (
            g.context ().get<AttributeGroupRefs> ("attribute-group-refs"));

          // Handle refs from last to first so that multiple insertions
          // to an empty list (always front) end up in proper order.
          //
          for (AttributeGroupRefs::reverse_iterator i (refs.rbegin ());
               i != refs.rend (); ++i)
          {
            clone_attribute_group_content (*i, g);
          }

          g.context ().remove ("attribute-group-refs");
        }

        Traversal::AttributeGroup::traverse (g);
      }

      void
      traverse (SemanticGraph::Compositor& c)
      {
        using SemanticGraph::Compositor;

        // Resolve element-group-refs if any.
        //
        if (c.context ().count ("element-group-refs"))
        {
          using SemanticGraph::Scope;

          ElementGroupRefs& refs (
            c.context ().get<ElementGroupRefs> ("element-group-refs"));

          // Handle refs from last to first so that multiple insertions
          // to an empty list (always front) end up in proper order.
          //
          for (ElementGroupRefs::reverse_iterator i (refs.rbegin ());
               i != refs.rend (); ++i)
          {
            // Find our scope.
            //
            Compositor* j (&c);

            while(!j->contained_compositor_p ())
              j = &j->contained_particle ().compositor ();

            Compositor* comp (
              clone_element_group_content (
                dynamic_cast<Scope&> (j->contained_compositor ().container ()),
                *i));

            // Create ContainsParticle edge.
            //
            if (comp)
            {
              NodeArgs<Compositor, Compositor::ContainsIterator> na (
                c, i->contains_pos);
              s_.new_edge<ContainsParticle> (na, *comp, i->min, i->max);
            }
          }

          c.context ().remove ("element-group-refs");
        }

        // Traverse recursively but only particles that are compositors.
        // This way we won't trigger anonymous type traversal (via member)
        // and therefore can call this functions from resolve_element_group
        // to completely resolve a group.
        //
        for (Compositor::ContainsIterator i (c.contains_begin ()),
               e (c.contains_end ()); i != e; ++i)
        {
          SemanticGraph::Particle& p (i->particle ());

          if (p.is_a<Compositor> ())
            dispatch (p);
        }

        // Traversal::Compositor::traverse (c);
      }

      SemanticGraph::Compositor*
      clone_element_group_content (SemanticGraph::Scope& s,
                                   ElementGroupRef const& ref)
      {
        using SemanticGraph::Scope;
        using SemanticGraph::Compositor;
        using SemanticGraph::ElementGroup;

        try
        {
          ElementGroup& g (
            resolve<ElementGroup> (ref.ns_name, ref.uq_name, s_, cache_));

          // Make sure the group and all its content are fully resolved.
          //
          resolve_element_group (g);

          Scope::NamesIterator pos (ref.names_pos);
          Compositor& root (g.contains_compositor ().compositor ());
          Compositor& copy (clone_compositor (root, s, pos));

          return &copy;
        }
        catch (NotNamespace const& ex)
        {
          if (valid_)
          {
            wcerr << "ice: unable to resolve namespace '" << ex.ns () << "'"
                  << endl;
            abort ();
          }
        }
        catch (NotName const& ex)
        {
          if (valid_)
          {
            wcerr << "ice: unable to resolve name '" << ex.name ()
                  << "' inside namespace '" << ex.ns () << "'" << endl;
            abort ();
          }
        }

        return 0;
      }

      SemanticGraph::Compositor&
      clone_compositor (SemanticGraph::Compositor& c,
                        SemanticGraph::Scope& scope,
                        SemanticGraph::Scope::NamesIterator& pos)
      {
        using SemanticGraph::Any;
        using SemanticGraph::Element;
        using SemanticGraph::Particle;
        using SemanticGraph::Compositor;

        Compositor* tmp (0);

        if (c.is_a<All> ())
          tmp = &s_.new_node<All> (c.file (), c.line (), c.column ());
        else if (c.is_a<Choice> ())
          tmp = &s_.new_node<Choice> (c.file (), c.line (), c.column ());
        else if (c.is_a<Sequence> ())
          tmp = &s_.new_node<Sequence> (c.file (), c.line (), c.column ());
        else
          assert (false);

        Compositor& copy (*tmp);

        // Copy annotation.
        //
        if (c.annotated_p ())
          s_.new_edge<Annotates> (c.annotation (), copy);

        for (Compositor::ContainsIterator i (c.contains_begin ());
             i != c.contains_end (); ++i)
        {
          Particle& p (i->particle ());

          if (p.is_a<Compositor> ())
          {
            Compositor& c (dynamic_cast<Compositor&> (p));
            Compositor& cc (clone_compositor (c, scope, pos));

            s_.new_edge<ContainsParticle> (copy, cc, i->min (), i->max ());
          }
          else if (p.is_a<Element> ())
          {
            Element& e (dynamic_cast<Element&> (p));
            Element& ec (clone_element (e));

            s_.new_edge<ContainsParticle> (copy, ec, i->min (), i->max ());

            NodeArgs<Scope, Scope::NamesIterator> na (scope, pos);
            s_.new_edge<Names> (na, ec, e.name ());
            ++pos;
          }
          else if (p.is_a<Any> ())
          {
            Any& a (dynamic_cast<Any&> (p));
            Any& ac (
              s_.new_node<Any> (a.file (), a.line (), a.column (),
                                a.namespace_begin (), a.namespace_end ()));

            ac.prototype (a);

            s_.new_edge<ContainsParticle> (copy, ac, i->min (), i->max ());

            // Transfer annotation.
            //
            if (a.annotated_p ())
              s_.new_edge<Annotates> (a.annotation (), ac);

            // Any has no name so we have to come up with a fake one in
            // order to put it into the scope. Note that we cannot reuse
            // the name from the prototype.

            unsigned long count;
            SemanticGraph::Context& ctx (scope.context ());

            if (!ctx.count ("any-name-count"))
            {
              count = 0;
              ctx.set ("any-name-count", count);
            }
            else
              count = ++(ctx.get<unsigned long> ("any-name-count"));

            std::basic_ostringstream<wchar_t> os;
            os << "any #" << count;

            NodeArgs<Scope, Scope::NamesIterator> na (scope, pos);
            s_.new_edge<Names> (na, ac, os.str ());
            ++pos;
          }
          else
            assert (false);
        }

        return copy;
      }

      // Clone a fully-resolved element. Note that it cannot be used as
      // is to clone ref'ed element (default/fixed value, etc).
      //
      SemanticGraph::Element&
      clone_element (SemanticGraph::Element& e)
      {
        using SemanticGraph::Element;

        Element& copy (
        s_.new_node<Element> (
          e.file (), e.line (), e.column (), e.global_p (), e.qualified_p ()));

        if (e.qualified_p ())
          s_.new_edge<BelongsToNamespace> (copy, e.namespace_ ());

        // Transfer default and fixed values.
        //
        if (e.fixed_p ())
          copy.fixed (e.value ());
        else if (e.default_p ())
          copy.default_ (e.value ());

        if (copy.default_p ())
        {
          copy.context ().set (
            "dom-node",
            e.context ().get<Xerces::DOMElement*> ("dom-node"));
          default_values_.push_back (&copy);
        }

        // Transfer annotation.
        //
        if (e.annotated_p ())
          s_.new_edge<Annotates> (e.annotation (), copy);

        // Belongs edge.
        //
        if (e.typed_p ())
          s_.new_edge<Belongs> (copy, e.type ());
        else
          assert (!valid_);

        // Substitutes edge.
        //
        if (e.substitutes_p ())
          s_.new_edge<Substitutes> (copy, e.substitutes ().root ());

        return copy;
      }

      void
      clone_attribute_group_content (AttributeGroupRef& ref,
                                     SemanticGraph::Scope& s)
      {
        using SemanticGraph::Scope;
        using SemanticGraph::Attribute;
        using SemanticGraph::AttributeGroup;

        try
        {
          AttributeGroup& g (
            resolve<AttributeGroup> (ref.ns_name, ref.uq_name, s_, cache_));

          // Make sure the group and all its content are fully resolved.
          //
          traverse (g);

          Scope::NamesIterator pos (ref.names_pos);

          for (Scope::NamesIterator i (g.names_begin ());
               i != g.names_end (); ++i)
          {
            if (Attribute* p = dynamic_cast<Attribute*> (&i->named ()))
            {
              Attribute& a (
                s_.new_node<Attribute> (p->file (),
                                        p->line (),
                                        p->column (),
                                        p->optional_p (),
                                        p->global_p (),
                                        p->qualified_p ()));

              NodeArgs<Scope, Scope::NamesIterator> na (s, pos);
              s_.new_edge<Names> (na, a, p->name ());
              ++pos;

              if (p->qualified_p ())
                s_.new_edge<BelongsToNamespace> (a, p->namespace_ ());

              // Transfer default and fixed values if any.
              //
              if (p->fixed_p ())
                a.fixed (p->value ());
              else if (p->default_p ())
                a.default_ (p->value ());

              if (a.default_p ())
              {
                a.context ().set (
                  "dom-node",
                  p->context ().get<Xerces::DOMElement*> ("dom-node"));
                default_values_.push_back (&a);
              }

              // Transfer annotation.
              //
              if (p->annotated_p ())
                s_.new_edge<Annotates> (p->annotation (), a);

              // Belongs edge.
              //
              if (p->typed_p ())
                s_.new_edge<Belongs> (a, p->type ());
              else
                assert (!valid_);
            }
            else if (
              AnyAttribute* p = dynamic_cast<AnyAttribute*> (&i->named ()))
            {
              AnyAttribute& any (
                s_.new_node<AnyAttribute> (p->file (),
                                           p->line (),
                                           p->column (),
                                           p->namespace_begin (),
                                           p->namespace_end ()));

              any.prototype (*p);

              // Transfer annotation.
              //
              if (p->annotated_p ())
                s_.new_edge<Annotates> (p->annotation (), any);

              // AnyAttribute has no name so we have to come up with a fake
              // one in order to put it into the scope. Note that we cannot
              // reuse the name from the attribute group.

              unsigned long count;
              SemanticGraph::Context& ctx (s.context ());

              if (!ctx.count ("any-attribute-name-count"))
              {
                count = 0;
                ctx.set ("any-attribute-name-count", count);
              }
              else
                count = ++(ctx.get<unsigned long> ("any-attribute-name-count"));

              std::basic_ostringstream<wchar_t> os;
              os << "any-attribute #" << count;

              NodeArgs<Scope, Scope::NamesIterator> na (s, pos);
              s_.new_edge<Names> (na, any, os.str ());
              ++pos;
            }
          }
        }
        catch (NotNamespace const& ex)
        {
          if (valid_)
          {
            wcerr << "ice: unable to resolve namespace '" << ex.ns () << "'"
                  << endl;
            abort ();
          }
        }
        catch (NotName const& ex)
        {
          if (valid_)
          {
            wcerr << "ice: unable to resolve attribute group name '"
                  << ex.name () << "' inside namespace '" << ex.ns () << "'"
                  << endl;
            abort ();
          }
        }
      }

    private:
      Schema& s_;
      bool& valid_;
      NamespaceMap& cache_;
      DefaultValues& default_values_;

    private:
      //Traversal::ContainsParticle contains_particle;
      Traversal::ContainsCompositor contains_compositor;
    };
  }

  // Parser::Impl
  //

  class Parser::Impl
  {
    Impl (Impl const&);
    Impl& operator= (Impl const&);

  public:
    ~Impl ();

    Impl (bool proper_restriction,
          bool multiple_imports,
          bool full_schema_check,
          LocationTranslator*,
          const WarningSet*);

    auto_ptr<Schema>
    parse (Path const&);

    auto_ptr<Schema>
    parse (Paths const&);

    auto_ptr<Schema>
    xml_schema (Path const&);

  private:
    void
    fill_xml_schema (Schema&, Path const&);

  private:
    XML::AutoPtr<Xerces::DOMDocument>
    dom (SemanticGraph::Path const&, bool validate);

    void
    schema (XML::Element const&);

    SemanticGraph::Annotation*
    annotation (bool process);

    void
    import (XML::Element const&);

    void
    include (XML::Element const&);

    void
    element_group (XML::Element const&, bool in_compositor);

    SemanticGraph::Type*
    simple_type (XML::Element const&);

    SemanticGraph::Type*
    list (XML::Element const& l, XML::Element const& type);

    SemanticGraph::Type*
    union_ (XML::Element const& u, XML::Element const& type);

    SemanticGraph::Type*
    restriction (XML::Element const& r, XML::Element const& type);

    void
    enumeration (XML::Element const&);

    SemanticGraph::Type*
    complex_type (XML::Element const&);

    All*
    all (XML::Element const&);

    Choice*
    choice (XML::Element const&, bool in_compositor);

    Sequence*
    sequence (XML::Element const&, bool in_compositor);

    void
    simple_content (XML::Element const&);

    void
    complex_content (XML::Element const&, Complex&);

    void
    simple_content_extension (XML::Element const&);

    void
    simple_content_restriction (XML::Element const&);

    void
    complex_content_extension (XML::Element const&, Complex&);

    void
    complex_content_restriction (XML::Element const&, Complex&);

    void
    element (XML::Element const&, bool global);

    void
    attribute (XML::Element const&, bool global);

    void
    attribute_group (XML::Element const&);

    void
    any (XML::Element const&);

    void
    any_attribute (XML::Element const&);

  private:
    bool
    is_disabled (char const* warning)
    {
      return disabled_warnings_all_ ||
        (disabled_warnings_ &&
         disabled_warnings_->find (warning) != disabled_warnings_->end ());
    }

  private:
    bool
    more () const
    {
      iterator const& it (iteration_state_.top ());

      return it.l_->getLength () > it.i_;
    }

    XML::Element
    next ()
    {
      iterator& it (iteration_state_.top ());

      return XML::Element (
        dynamic_cast<Xerces::DOMElement*> (it.l_->item (it.i_++)));
    }

    void
    prev ()
    {
      iterator& it (iteration_state_.top ());

      if (it.i_)
        --it.i_;
    }

    void
    push (XML::Element const& e)
    {
      iteration_state_.push (e.dom_element ());
    }

    void
    pop ()
    {
      iteration_state_.pop ();
    }

  private:
    void
    push_scope (SemanticGraph::Scope& s)
    {
      scope_stack_.push (&s);
    }

    void
    pop_scope ()
    {
      scope_stack_.pop ();
    }

    SemanticGraph::Scope&
    scope () const
    {
      return *(scope_stack_.top ());
    }

  private:
    void
    push_compositor (SemanticGraph::Compositor& c)
    {
      compositor_stack_.push (&c);
    }

    void
    pop_compositor ()
    {
      assert (!compositor_stack_.empty ());
      compositor_stack_.pop ();
    }

    SemanticGraph::Compositor&
    compositor () const
    {
      assert (!compositor_stack_.empty ());
      return *(compositor_stack_.top ());
    }

  private:
    static unsigned long const unbounded = ~static_cast<unsigned long> (0);

    unsigned long
    parse_min (String const& m)
    {
      if (m.empty ())
        return 1;

      unsigned long v;
      std::basic_istringstream<wchar_t> is (m);

      is >> v;
      return v;
    }

    unsigned long
    parse_max (String const& m)
    {
      if (m.empty ())
        return 1;

      if (m == L"unbounded")
        return unbounded;

      unsigned long v;
      std::basic_istringstream<wchar_t> is (m);

      is >> v;
      return v;
    }

  private:
    SemanticGraph::Namespace&
    cur_ns () const
    {
      // Here I am using the fact that each Schema Names only one
      // Namespace.
      //
      return dynamic_cast<Namespace&> (cur_->names_begin ()->named ());
    }

  private:
    String
    unqualified_name (String const& n)
    {
      return XML::uq_name (n);
    }

    String
    namespace_name (XML::Element const& e, String const& n)
    {
      try
      {
        String p (XML::prefix (n));

        // If we are currently handling a chameleon-included schema then
        // the empty prefix is logically translated into acquired target
        // namespace.
        //
        if (cur_chameleon_ && p.empty ())
          return cur_ns ().name ();

        // We have to try to resolve even the empty prefix since it can
        // be assigned to a namespace (which takes precedence over names
        // without a namespace).
        //
        return XML::ns_name (e.dom_element (), p);
      }
      catch (XML::NoMapping const& ex)
      {
        if (ex.prefix ().empty ())
          return String ();
        else
          throw;
      }
    }

    SemanticGraph::Type&
    ultimate_base (SemanticGraph::Type& t)
    {
      using namespace SemanticGraph;

      Complex* c = dynamic_cast<Complex*> (&t);

      if (c != 0 && c->inherits_p ())
      {
        Type* b (&c->inherits ().base ());

        while (true)
        {
          Complex* cb (dynamic_cast<Complex*> (b));

          if (cb != 0 && cb->inherits_p ())
          {
            b = &cb->inherits ().base ();
            continue;
          }

          break;
        }

        return *b;
      }
      else
        return t;
    }

  private:
    template <typename Edge, typename Node>
    Edge*
    set_type (String const& type, XML::Element const& e, Node& node);

  private:
    XML::PtrVector<Xerces::DOMDocument>* dom_docs_;

    struct iterator
    {
      iterator (Xerces::DOMElement* e)
          : l_ (e->getChildNodes ()), i_ (0)
      {
      }

      Xerces::DOMNodeList* l_;
      size_t i_;
    };

    std::stack<iterator> iteration_state_;
    SemanticGraph::Schema* s_;   // root schema file
    SemanticGraph::Schema* cur_; // current schema file
    bool cur_chameleon_;      // whethere cur_ is chameleon

    SemanticGraph::Schema* xml_schema_; // XML Schema file
    SemanticGraph::Path xml_schema_path_;

    //
    //
    std::stack<SemanticGraph::Scope*> scope_stack_;

    //
    //
    std::stack<SemanticGraph::Compositor*> compositor_stack_;


    // Map of absolute file path and namespace pair to a Schema node.
    //
    struct SchemaId
    {
      SchemaId (SemanticGraph::Path const& path, String const& ns)
          : path_ (path), ns_ (ns)
      {
      }


      friend bool
      operator< (SchemaId const& x, SchemaId const& y)
      {
        return x.path_ < y.path_ || (x.path_ == y.path_ && x.ns_ < y.ns_);
      }

    private:
      SemanticGraph::Path path_;
      String ns_;
    };


    typedef std::map<SchemaId, SemanticGraph::Schema*> SchemaMap;
    SchemaMap schema_map_;

    // Path stack for diagnostic.
    //
    struct PathPair
    {
      PathPair (SemanticGraph::Path const& r, SemanticGraph::Path const& a)
          : rel (r), abs (a)
      {
      }

      SemanticGraph::Path rel, abs;
    };

    std::stack<PathPair> file_stack_;

    SemanticGraph::Path const&
    file ()
    {
      return file_stack_.top ().rel;
    }

    SemanticGraph::Path const&
    abs_file ()
    {
      return file_stack_.top ().abs;
    }

    // Members with default/fixed values (needed for QName handling).
    //
    DefaultValues default_values_;

  private:
    bool qualify_attribute_;
    bool qualify_element_;

    bool valid_;

    bool proper_restriction_;
    bool multiple_imports_;
    bool full_schema_check_;
    LocationTranslator* loc_translator_;
    const WarningSet* disabled_warnings_;
    bool disabled_warnings_all_;

    NamespaceMap* cache_;
  };


  Parser::Impl::
  Impl (bool proper_restriction,
        bool multiple_imports,
        bool full_schema_check,
        LocationTranslator* t,
        const WarningSet* dw)
      : s_ (0),
        cur_ (0),
        cur_chameleon_ (false),
        xml_schema_path_ ("XMLSchema.xsd"),
        qualify_attribute_ (false),
        qualify_element_ (false),
        proper_restriction_ (proper_restriction),
        multiple_imports_ (multiple_imports),
        full_schema_check_ (full_schema_check),
        loc_translator_ (t),
        disabled_warnings_ (dw),
        disabled_warnings_all_ (false)
  {
    if (dw && dw->find ("all") != dw->end ())
      disabled_warnings_all_ = true;

    // Initialize the Xerces-C++ runtime.
    //
    Xerces::XMLPlatformUtils::Initialize ();
  }

  Parser::Impl::
  ~Impl ()
  {
    // Terminate the Xerces-C++ runtime.
    //
    Xerces::XMLPlatformUtils::Terminate ();
  }

  template<typename T> T&
  add_type (Schema& s, Namespace& ns, String name)
  {
    Path path ("XMLSchema.xsd");
    T& node (s.new_node<T> (path, 0, 0));
    s.new_edge<Names> (ns, node, name);

    return node;
  }

  void Parser::Impl::
  fill_xml_schema (Schema& s, Path const& path)
  {
    Namespace& ns (s.new_node<Namespace> (path, 1, 1));
    s.new_edge<Names> (s, ns, xsd);

    // anyType and & anySimpleType
    //
    AnyType& any_type (
      add_type<AnyType>                       (s, ns, L"anyType"));
    add_type<AnySimpleType>                   (s, ns, L"anySimpleType");

    // Integers.
    //
    add_type<Fundamental::Byte>               (s, ns, L"byte");
    add_type<Fundamental::UnsignedByte>       (s, ns, L"unsignedByte");
    add_type<Fundamental::Short>              (s, ns, L"short");
    add_type<Fundamental::UnsignedShort>      (s, ns, L"unsignedShort");
    add_type<Fundamental::Int>                (s, ns, L"int");
    add_type<Fundamental::UnsignedInt>        (s, ns, L"unsignedInt");
    add_type<Fundamental::Long>               (s, ns, L"long");
    add_type<Fundamental::UnsignedLong>       (s, ns, L"unsignedLong");
    add_type<Fundamental::Integer>            (s, ns, L"integer");
    add_type<Fundamental::NonPositiveInteger> (s, ns, L"nonPositiveInteger");
    add_type<Fundamental::NonNegativeInteger> (s, ns, L"nonNegativeInteger");
    add_type<Fundamental::PositiveInteger>    (s, ns, L"positiveInteger");
    add_type<Fundamental::NegativeInteger>    (s, ns, L"negativeInteger");

    // Boolean.
    //
    add_type<Fundamental::Boolean>            (s, ns, L"boolean");

    // Floats.
    //
    add_type<Fundamental::Float>              (s, ns, L"float");
    add_type<Fundamental::Double>             (s, ns, L"double");
    add_type<Fundamental::Decimal>            (s, ns, L"decimal");

    // Strings
    //
    add_type<Fundamental::String>             (s, ns, L"string");
    add_type<Fundamental::NormalizedString>   (s, ns, L"normalizedString");
    add_type<Fundamental::Token>              (s, ns, L"token");
    add_type<Fundamental::Name>               (s, ns, L"Name");
    add_type<Fundamental::NameToken>          (s, ns, L"NMTOKEN");
    add_type<Fundamental::NameTokens>         (s, ns, L"NMTOKENS");
    add_type<Fundamental::NCName>             (s, ns, L"NCName");
    add_type<Fundamental::Language>           (s, ns, L"language");

    // ID/IDREF.
    //
    add_type<Fundamental::Id>                 (s, ns, L"ID");

    Fundamental::IdRef& id_ref (
      s.new_node<Fundamental::IdRef> (path, 0, 0));
    s.new_edge<Names> (ns, id_ref, L"IDREF");
    s.new_edge<Arguments> (any_type, id_ref);

    Fundamental::IdRefs& id_refs (
      s.new_node<Fundamental::IdRefs> (path, 0, 0));
    s.new_edge<Names> (ns, id_refs, L"IDREFS");
    s.new_edge<Arguments> (any_type, id_refs);

    // URI.
    //
    add_type<Fundamental::AnyURI>             (s, ns, L"anyURI");

    // Qualified name.
    //
    add_type<Fundamental::QName>              (s, ns, L"QName");

    // Binary.
    //
    add_type<Fundamental::Base64Binary>       (s, ns, L"base64Binary");
    add_type<Fundamental::HexBinary>          (s, ns, L"hexBinary");

    // Date/time.
    //
    add_type<Fundamental::Date>               (s, ns, L"date");
    add_type<Fundamental::DateTime>           (s, ns, L"dateTime");
    add_type<Fundamental::Duration>           (s, ns, L"duration");
    add_type<Fundamental::Day>                (s, ns, L"gDay");
    add_type<Fundamental::Month>              (s, ns, L"gMonth");
    add_type<Fundamental::MonthDay>           (s, ns, L"gMonthDay");
    add_type<Fundamental::Year>               (s, ns, L"gYear");
    add_type<Fundamental::YearMonth>          (s, ns, L"gYearMonth");
    add_type<Fundamental::Time>               (s, ns, L"time");

    // Entity.
    //
    add_type<Fundamental::Entity>             (s, ns, L"ENTITY");
    add_type<Fundamental::Entities>           (s, ns, L"ENTITIES");

    // Notation.
    //
    add_type<Fundamental::Notation>           (s, ns, L"NOTATION");
  }


  auto_ptr<Schema> Parser::Impl::
  xml_schema (Path const& tu)
  {
    valid_ = true;

    auto_ptr<Schema> rs (new Schema (tu, 1, 1));
    fill_xml_schema (*rs, tu);

    if (!valid_)
      throw InvalidSchema ();

    return rs;
  }
  auto_ptr<Schema> Parser::Impl::
  parse (Path const& tu)
  {
    valid_ = true;
    schema_map_.clear ();
    default_values_.clear ();

    XML::PtrVector<Xerces::DOMDocument> dom_docs;
    dom_docs_ = &dom_docs;

    NamespaceMap cache;
    cache_ = &cache;

    XML::AutoPtr<Xerces::DOMDocument> d (dom (tu, true));

    if (!d)
      throw InvalidSchema ();

    XML::Element root (d->getDocumentElement ());
    String ns (trim (root["targetNamespace"]));

    if (trace_)
      wcout << "target namespace: " << ns << endl;

    auto_ptr<Schema> rs (new Schema (tu, root.line (), root.column ()));

    // Implied schema with fundamental types.
    //
    xml_schema_ = &rs->new_node<Schema> (xml_schema_path_, 1, 1);
    rs->new_edge<Implies> (*rs, *xml_schema_, xml_schema_path_);

    fill_xml_schema (*xml_schema_, xml_schema_path_);

    // Parse.
    //
    {
      // Enter the file into schema_map_. Do normalize() before
      // complete() to avoid hitting system path limits with '..'
      // directories.
      //
      Path abs_path (tu);
      abs_path.normalize ().complete ();
      schema_map_[SchemaId (abs_path, ns)] = rs.get ();
      rs->context ().set ("absolute-path", abs_path);

      s_ = cur_ = rs.get ();
      {
        file_stack_.push (PathPair (tu, abs_path));

        {
          push_scope (
            s_->new_node<Namespace> (
              file (), root.line (), root.column ()));
          s_->new_edge<Names> (*cur_, scope (), ns);

          {
            schema (root);
          }

          pop_scope ();
        }

        file_stack_.pop ();
      }

      s_ = cur_ = 0;
    }

    dom_docs_->push_back (d);

    // Second pass to resolve forward references to types, elements,
    // attributes and groups.
    //
    if (valid_)
    {
      Traversal::Schema schema;

      struct Uses: Traversal::Uses
      {
        virtual void
        traverse (Type& u)
        {
          Schema& s (u.schema ());

          if (!s.context ().count ("schema-resolved"))
          {
            s.context ().set ("schema-resolved", true);
            Traversal::Uses::traverse (u);
          }
        }
      } uses;

      Traversal::Names schema_names;
      Traversal::Namespace ns;
      Traversal::Names ns_names;

      schema >> uses >> schema;
      schema >> schema_names >> ns >> ns_names;

      Resolver resolver (*rs, valid_, *cache_, default_values_);

      struct AnonymousMember: Traversal::Attribute,
                              Traversal::Element,
                              Traversal::Member
      {
        AnonymousMember (Traversal::NodeDispatcher& d)
        {
          belongs_.node_traverser (d);
        }

        virtual void
        traverse (SemanticGraph::Attribute& a)
        {
          traverse_member (a);
        }

        virtual void
        traverse (SemanticGraph::Element& e)
        {
          traverse_member (e);
        }

        void
        traverse_member (SemanticGraph::Member& m)
        {
          if (m.typed_p () &&
              !m.type ().named_p () &&
              !m.type ().context ().count ("seen"))
          {
            m.type().context ().set ("seen", true);

            Traversal::Member::belongs (m, belongs_);

            m.type ().context ().remove ("seen");
          }
        }

      private:
        Traversal::Belongs belongs_;
      } anonymous_member (resolver);

      struct AnonymousBase: Traversal::Type
      {
        AnonymousBase (Traversal::NodeDispatcher& d)
            : base_ (d)
        {
        }

        virtual void
        traverse (SemanticGraph::Type& t)
        {
          if (!t.named_p ())
            base_.dispatch (t);
        }

      private:
        Traversal::NodeDispatcher& base_;
      } anonymous_base (resolver);

      ns_names >> resolver;
      ns_names >> anonymous_member;

      Traversal::Names names;
      Traversal::Inherits inherits;
      Traversal::Argumented argumented;
      resolver >> names >> resolver;
      names >> anonymous_member;
      resolver >> inherits >> anonymous_base;
      resolver >> argumented >> anonymous_base;

      if (trace_)
        wcout << "starting resolution pass" << endl;

      schema.dispatch (*rs);
    }

    // Resolve default/fixed values of QName type.
    //
    if (valid_)
    {
      for (DefaultValues::const_iterator i (default_values_.begin ()),
             e (default_values_.end ()); i != e; ++i)
      {
        SemanticGraph::Member& m (**i);
        SemanticGraph::Type& t (m.type ());
        SemanticGraph::Context& c (m.context ());

        if (ultimate_base (t).is_a<SemanticGraph::Fundamental::QName> ())
        {
          String v (m.value ());
          Xerces::DOMElement* e (c.get<Xerces::DOMElement*> ("dom-node"));

          try
          {
            // We have to try to resolve even the empty prefix since it can
            // be assigned to a namespace (which takes precedence over names
            // without a namespace).
            //
            String ns (XML::ns_name (e, XML::prefix (v)));

            if (m.fixed_p ())
              m.fixed (ns + L'#' + v);
            else
              m.default_ (ns + L'#' + v);
          }
          catch (XML::NoMapping const& ex)
          {
            if (!ex.prefix ().empty ())
            {
              wcerr << m.file () << ":" << m.line () << ":" << m.column ()
                    << ": error: unable to resolve namespace for prefix '"
                    <<  ex.prefix () << "'" << endl;

              valid_ = false;
            }
          }
        }

        c.remove ("dom-node");
      }
    }

    if (!valid_)
      throw InvalidSchema ();

    return rs;
  }

  auto_ptr<Schema> Parser::Impl::
  parse (Paths const& paths)
  {
    valid_ = true;
    schema_map_.clear ();
    default_values_.clear ();

    XML::PtrVector<Xerces::DOMDocument> dom_docs;
    dom_docs_ = &dom_docs;

    NamespaceMap cache;
    cache_ = &cache;

    auto_ptr<Schema> rs (new Schema (Path (), 0, 0));

    // Implied schema with fundamental types.
    //
    xml_schema_ = &rs->new_node<Schema> (xml_schema_path_, 1, 1);
    rs->new_edge<Implies> (*rs, *xml_schema_, xml_schema_path_);

    fill_xml_schema (*xml_schema_, xml_schema_path_);

    // Parse individual schemas.
    //
    s_ = rs.get ();

    for (Paths::const_iterator i (paths.begin ()); i != paths.end (); ++i)
    {
      Path const& tu (*i);
      XML::AutoPtr<Xerces::DOMDocument> d (dom (tu, true));

      if (!d)
        throw InvalidSchema ();

      XML::Element root (d->getDocumentElement ());
      String ns (trim (root["targetNamespace"]));

      if (trace_)
        wcout << "target namespace: " << ns << endl;

      // Check if we already have this schema. Do normalize() before
      // complete() to avoid hitting system path limits with '..'
      // directories.
      //
      Path abs_path (tu);
      abs_path.normalize ().complete ();
      SchemaId schema_id (abs_path, ns);

      if (schema_map_.find (schema_id) != schema_map_.end ())
        continue;

      Schema& s (s_->new_node<Schema> (tu, root.line (), root.column ()));
      s_->new_edge<Implies> (s, *xml_schema_, xml_schema_path_);
      s_->new_edge<Imports> (*s_, s, tu);

      // Enter the file into schema_map_.
      //
      schema_map_[schema_id] = &s;
      s.context ().set ("absolute-path", abs_path);

      cur_ = &s;

      {
        file_stack_.push (PathPair (tu, abs_path));

        {
          push_scope (
            s_->new_node<Namespace> (
              file (), root.line (), root.column ()));
          s_->new_edge<Names> (*cur_, scope (), ns);

          {
            schema (root);
          }

          pop_scope ();
        }

        file_stack_.pop ();
      }

      cur_ = 0;

      dom_docs_->push_back (d);

      if (!valid_)
        break;
    }

    s_ = 0;

    // Second pass to resolve forward references to types, elements,
    // attributes and groups.
    //
    if (valid_)
    {
      Traversal::Schema schema;

      struct Uses: Traversal::Uses
      {
        virtual void
        traverse (Type& u)
        {
          Schema& s (u.schema ());

          if (!s.context ().count ("schema-resolved"))
          {
            s.context ().set ("schema-resolved", true);
            Traversal::Uses::traverse (u);
          }
        }
      } uses;

      Traversal::Names schema_names;
      Traversal::Namespace ns;
      Traversal::Names ns_names;

      schema >> uses >> schema;
      schema >> schema_names >> ns >> ns_names;

      Resolver resolver (*rs, valid_, *cache_, default_values_);

      struct AnonymousMember: Traversal::Attribute,
                              Traversal::Element,
                              Traversal::Member
      {
        AnonymousMember (Traversal::NodeDispatcher& d)
        {
          belongs_.node_traverser (d);
        }

        virtual void
        traverse (SemanticGraph::Attribute& a)
        {
          traverse_member (a);
        }

        virtual void
        traverse (SemanticGraph::Element& e)
        {
          traverse_member (e);
        }

        virtual void
        traverse_member (SemanticGraph::Member& m)
        {
          if (m.typed_p () &&
              !m.type ().named_p () &&
              !m.type ().context ().count ("seen"))
          {
            m.type().context ().set ("seen", true);

            Traversal::Member::belongs (m, belongs_);

            m.type ().context ().remove ("seen");
          }
        }

      private:
        Traversal::Belongs belongs_;
      } anonymous_member (resolver);

      struct AnonymousBase: Traversal::Type
      {
        AnonymousBase (Traversal::NodeDispatcher& d)
            : base_ (d)
        {
        }

        virtual void
        traverse (SemanticGraph::Type& t)
        {
          if (!t.named_p ())
            base_.dispatch (t);
        }

      private:
        Traversal::NodeDispatcher& base_;
      } anonymous_base (resolver);

      ns_names >> resolver;
      ns_names >> anonymous_member;

      Traversal::Names names;
      Traversal::Inherits inherits;
      Traversal::Argumented argumented;
      resolver >> names >> resolver;
      names >> anonymous_member;
      resolver >> inherits >> anonymous_base;
      resolver >> argumented >> anonymous_base;

      if (trace_)
        wcout << "starting resolution pass" << endl;

      schema.dispatch (*rs);
    }

    // Resolve default/fixed values of QName type.
    //
    if (valid_)
    {
      for (DefaultValues::const_iterator i (default_values_.begin ()),
             e (default_values_.end ()); i != e; ++i)
      {
        SemanticGraph::Member& m (**i);
        SemanticGraph::Type& t (m.type ());
        SemanticGraph::Context& c (m.context ());

        if (ultimate_base (t).is_a<SemanticGraph::Fundamental::QName> ())
        {
          String v (m.value ());
          Xerces::DOMElement* e (c.get<Xerces::DOMElement*> ("dom-node"));

          try
          {
            // We have to try to resolve even the empty prefix since it can
            // be assigned to a namespace (which takes precedence over names
            // without a namespace).
            //
            String ns (XML::ns_name (e, XML::prefix (v)));

            if (m.fixed_p ())
              m.fixed (ns + L'#' + v);
            else
              m.default_ (ns + L'#' + v);
          }
          catch (XML::NoMapping const& ex)
          {
            if (!ex.prefix ().empty ())
            {
              wcerr << m.file () << ":" << m.line () << ":" << m.column ()
                    << ": error: unable to resolve namespace for prefix '"
                    <<  ex.prefix () << "'" << endl;

              valid_ = false;
            }
          }
        }

        c.remove ("dom-node");
      }
    }

    if (!valid_)
      throw InvalidSchema ();

    return rs;
  }

  void Parser::Impl::
  schema (XML::Element const& s)
  {
    bool old_qa (qualify_attribute_);
    bool old_qe (qualify_element_);

    if (String af = trim (s["attributeFormDefault"]))
      qualify_attribute_ = af == L"qualified";
    else
      qualify_attribute_ = false;

    if (String ef = trim (s["elementFormDefault"]))
      qualify_element_ = ef == L"qualified";
    else
      qualify_element_ = false;

    push (s);

    // Parse leading annotation if any and add it as an annotation for
    // this schema.
    //
    if (Annotation* a = annotation (true))
      s_->new_edge<Annotates> (*a, *cur_);

    while (more ())
    {
      XML::Element e (next ());
      String name (e.name ());

      if (trace_)
        wcout << name << endl;

      if (name == L"import")         import (e);               else
      if (name == L"include")        include (e);              else
      if (name == L"element")        element (e, true);        else
      if (name == L"attribute")      attribute (e, true);      else
      if (name == L"simpleType")     simple_type (e);          else
      if (name == L"annotation");                              else
      if (name == L"complexType")    complex_type (e);         else
      if (name == L"group")          element_group (e, false); else
      if (name == L"attributeGroup") attribute_group (e);      else
      {
        wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
              << "error: unexpected top-level element: '" << name << "'"
              << endl;

        valid_ = false;
      }
    }

    pop ();

    qualify_attribute_ = old_qa;
    qualify_element_ = old_qe;
  }

  void Parser::Impl::
  import (XML::Element const& i)
  {
    NarrowString loc (
      trim (
        XML::transcode_to_narrow (
          i.dom_element ()->getAttribute (
            XML::XMLChString ("schemaLocation").c_str ()))));

    if (loc_translator_)
      loc = loc_translator_->translate (loc);

    String ins (trim (i["namespace"]));

    // Ignore empty <import>.
    //
    if (!loc && !ins)
      return;

    Path path, rel_path, abs_path;
    try
    {
      path = Path (loc);

      if (path.absolute ())
      {
        abs_path = rel_path = path;
        abs_path.normalize ();
      }
      else
      {
        // Do normalize() before complete() to avoid hitting system path
        // limits with '..' directories.
        //
        abs_path = rel_path = file ().directory () / path;
        abs_path.normalize ().complete ();
      }
    }
    catch (InvalidPath const&)
    {
      wcerr << file () << ":" << i.line () << ":" << i.column () << ": "
            << "error: '" << loc.c_str () << "' is not a valid "
            << "filesystem path" << endl;

      valid_ = false;
      return;
    }

    SchemaId schema_id (abs_path, ins);

    if (schema_map_.find (schema_id) != schema_map_.end ())
    {
      s_->new_edge<Imports> (*cur_, *schema_map_[schema_id], path);
      return;
    }

    if (trace_)
      wcout << "importing " << rel_path << endl;

    if (XML::AutoPtr<Xerces::DOMDocument> d  = dom (abs_path, false))
    {
      XML::Element r (d->getDocumentElement ());
      String ns (trim (r["targetNamespace"]));

      if (trace_)
        wcout << "target namespace: " << ns << endl;

      Schema& s (s_->new_node<Schema> (rel_path, r.line (), r.column ()));
      s_->new_edge<Implies> (s, *xml_schema_, xml_schema_path_);
      s_->new_edge<Imports> (*cur_, s, path);

      schema_map_[schema_id] = &s;
      s.context ().set ("absolute-path", abs_path);

      Schema* old_cur (cur_);
      bool old_cur_chameleon (cur_chameleon_);
      cur_ = &s;
      cur_chameleon_ = false;

      {
        file_stack_.push (PathPair (rel_path, abs_path));

        {
          push_scope (
            s_->new_node<Namespace> (file (), r.line (), r.column ()));
          s_->new_edge<Names> (*cur_, scope (), ns);

          {
            schema (r);
          }

          pop_scope ();
        }

        file_stack_.pop ();
      }

      cur_chameleon_ = old_cur_chameleon;
      cur_ = old_cur;

      dom_docs_->push_back (d);
    }
  }

  void Parser::Impl::
  include (XML::Element const& i)
  {
    NarrowString loc (
      trim (
        XML::transcode_to_narrow (
          i.dom_element ()->getAttribute (
            XML::XMLChString ("schemaLocation").c_str ()))));

    if (loc_translator_)
      loc = loc_translator_->translate (loc);

    Path path, rel_path, abs_path;
    try
    {
      path = Path (loc);

      if (path.absolute ())
      {
        abs_path = rel_path = path;
        abs_path.normalize ();
      }
      else
      {
        // Do normalize() before complete() to avoid hitting system path
        // limits with '..' directories.
        //
        abs_path = rel_path = file ().directory () / path;
        abs_path.normalize ().complete ();
      }
    }
    catch (InvalidPath const&)
    {
      wcerr << file () << ":" << i.line () << ":" << i.column () << ": "
            << "error: '" << loc.c_str () << "' is not a valid "
            << "filesystem path" << endl;

      valid_ = false;
      return;
    }

    // Included schema should have the same namespace as ours.
    //
    SchemaId schema_id (abs_path, cur_ns ().name ());

    if (schema_map_.find (schema_id) != schema_map_.end ())
    {
      Schema& s (*schema_map_[schema_id]);

      // Chemeleon inclusion results in a new Schema node for every
      // namespace. As a result, such a Schema node can only be
      // Source'ed. I use this property to decide which edge to use.
      //

      if (s.used_p () && s.used_begin ()->is_a<Sources> ())
        s_->new_edge<Sources> (*cur_, s, path);
      else
        s_->new_edge<Includes> (*cur_, s, path);

      return;
    }

    if (trace_)
      wcout << "including " << rel_path << endl;

    if (XML::AutoPtr<Xerces::DOMDocument> d  = dom (abs_path, false))
    {
      XML::Element r (d->getDocumentElement ());
      String ns (trim (r["targetNamespace"])), cur_ns;

      Schema& s (s_->new_node<Schema> (rel_path, r.line (), r.column ()));
      s_->new_edge<Implies> (s, *xml_schema_, xml_schema_path_);

      schema_map_[schema_id] = &s;
      s.context ().set ("absolute-path", abs_path);

      bool chameleon (false);

      if (ns.empty () && !(cur_ns = (cur_->names_begin ())->name ()).empty ())
      {
        // Chameleon.
        //
        ns = cur_ns;
        s_->new_edge<Sources> (*cur_, s, path);
        chameleon = true;

        if (trace_)
          wcout << "handling chameleon schema" << endl;
      }
      else
        s_->new_edge<Includes> (*cur_, s, path);

      if (trace_)
        wcout << "target namespace: " << ns << endl;

      Schema* old_cur (cur_);
      bool old_cur_chameleon (cur_chameleon_);
      cur_ = &s;
      cur_chameleon_ = chameleon;

      {
        file_stack_.push (PathPair (rel_path, abs_path));

        {
          push_scope (
            s_->new_node<Namespace> (file (), r.line (), r.column ()));
          s_->new_edge<Names> (*cur_, scope (), ns);

          {
            schema (r);
          }

          pop_scope ();
        }

        file_stack_.pop ();
      }

      cur_chameleon_ = old_cur_chameleon;
      cur_ = old_cur;

      dom_docs_->push_back (d);
    }
  }

  void Parser::Impl::
  element_group (XML::Element const& g, bool in_compositor)
  {
    if (String name = trim (g["name"]))
    {
      ElementGroup& group (
        s_->new_node<ElementGroup> (file (), g.line (), g.column ()));

      s_->new_edge<Names> (scope (), group, name);

      push_scope (group);
      push (g);

      annotation (false);

      XML::Element e (next ());

      name = e.name ();

      if (trace_)
        wcout << name << endl;

      Compositor* c (0);

      if (name == L"all")      c = all (e);             else
      if (name == L"choice")   c = choice (e, false);   else
      if (name == L"sequence") c = sequence (e, false); else
      {
        wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
              << "error: expected 'all', 'choice' or 'sequence' "
              << "instead of '" << name << "'" << endl;

        valid_ = false;
      }

      // Group's immediate compositor always has cardinality 1,1.
      //
      if (c)
        s_->new_edge<ContainsCompositor> (group, *c, 1, 1);

      pop ();
      pop_scope ();
    }
    else if (String ref = trim (g["ref"]))
    {
      if (trace_)
        wcout << "element-group-ref " << ref << endl;

      try
      {
        String uq_name (unqualified_name (ref));
        String ns_name (namespace_name (g, ref));

        // In order to avoid code duplication we are going to let the
        // resolver handle this case.
        //
        if (trace_)
          wcout << "deferring resolution of group name '" << uq_name
                << "' inside namespace '" << ns_name << "'"
                << " until later" << endl;

        if (in_compositor)
        {
          Compositor& c (compositor ());

          unsigned long min (parse_min (trim (g["minOccurs"])));
          unsigned long max (parse_max (trim (g["maxOccurs"])));

          ElementGroupRef ref (
            uq_name, ns_name,
            min,
            max == unbounded ? 0 : max,
            c,
            scope ());

          if (!c.context ().count ("element-group-refs"))
            c.context ().set ("element-group-refs", ElementGroupRefs ());

          c.context ().get<ElementGroupRefs> (
            "element-group-refs").push_back (ref);
        }
        else
        {
          // This is a group-ref directly in complexType.
          //

          Scope& s (scope ());

          unsigned long min (parse_min (trim (g["minOccurs"])));
          unsigned long max (parse_max (trim (g["maxOccurs"])));

          ElementGroupRef ref (
            uq_name, ns_name, min, max == unbounded ? 0 : max, s);

          s.context ().set ("element-group-ref", ref);
        }
      }
      catch (NotNamespace const& ex)
      {
        if (valid_)
        {
          wcerr << file () << ":" << g.line () << ":" << g.column () << ": "
                << "ice: unable to resolve namespace '" << ex.ns () << "'"
                << endl;

          abort ();
        }
      }
      catch (XML::NoMapping const& ex)
      {
        wcerr << file () << ":" << g.line () << ":" << g.column () << ": "
              << "error: unable to resolve namespace prefix '" << ex.prefix ()
              << "' in '" << ref << "'" << endl;

        valid_ = false;
      }
    }
    else
    {
      wcerr << file () << ":" << g.line () << ":" << g.column () << ": "
            << "error: 'name' or 'ref' attribute is missing in group "
            << "declaration" << endl;

      valid_ = false;

      return;
    }
  }

  //@@ Need RAII for push/pop.
  //

  Type* Parser::Impl::
  simple_type (XML::Element const& t)
  {
    Type* r (0);

    push (t);

    Annotation* a (annotation (true));

    XML::Element e (next ());

    String name (e.name ());

    if (name == L"list") r = list (e, t); else
    if (name == L"union") r = union_ (e, t); else
    if (name == L"restriction") r = restriction (e, t); else
    {
      wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
            << "error: expected 'list', 'union', or 'restriction' "
            << "instead of '" << name << "'" << endl;

      valid_ = false;
    }

    if (r != 0 && a != 0)
      s_->new_edge<Annotates> (*a, *r);

    pop ();

    return r;
  }

  SemanticGraph::Type* Parser::Impl::
  list (XML::Element const& l, XML::Element const& t)
  {
    if (trace_)
      wcout << "list" << endl;

    List& node (s_->new_node<List> (file (), t.line (), t.column ()));

    if (String item_type = trim (l["itemType"]))
    {
      if (trace_)
        wcout << "item type: " << fq_name (l, item_type) << endl;

      set_type<Arguments> (item_type, l, node);
    }
    else
    {
      // Anonymous list item type.
      //
      push (l);

      annotation (false);

      if (more ())
      {
        XML::Element e (next ());

        String name (e.name ());

        if (trace_)
          wcout << name << endl;

        Type* t (0);

        if (name == L"simpleType")  t = simple_type (e);  else
        {
          wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
                << "error: expected 'simpleType' instead of "
                << "'" << e.name () << "'" << endl;

          valid_ = false;
        }

        if (t)
          s_->new_edge<Arguments> (*t, node);
      }
      else
      {
        wcerr << file () << ":" << l.line () << ":" << l.column () << ": "
              << "error: expected 'itemType' attribute or 'simpleType' "
              << "nested element" << endl;

        valid_ = false;
      }

      pop ();
    }

    if (String name = trim (t["name"]))
      s_->new_edge<Names> (scope (), node, name);

    return &node;
  }

  namespace
  {
    //
    // List parsing utility functions.
    //

    // Find first non-space character.
    //
    size_t
    find_ns (const wchar_t* s, size_t size, size_t pos)
    {
      while (pos < size &&
             (s[pos] == 0x20 || // space
              s[pos] == 0x0D || // carriage return
              s[pos] == 0x09 || // tab
              s[pos] == 0x0A))
        ++pos;

      return pos < size ? pos : String::npos;
    }

    // Find first space character.
    //
    size_t
    find_s (const wchar_t* s, size_t size, size_t pos)
    {
      while (pos < size &&
             s[pos] != 0x20 && // space
             s[pos] != 0x0D && // carriage return
             s[pos] != 0x09 && // tab
             s[pos] != 0x0A)
        ++pos;

      return pos < size ? pos : String::npos;
    }
  }

  SemanticGraph::Type* Parser::Impl::
  union_ (XML::Element const& u, XML::Element const& t)
  {
    if (trace_)
      wcout << "union" << endl;

    Union& node (s_->new_node<Union> (file (), t.line (), t.column ()));

    bool has_members (false);

    if (String members = trim (u["memberTypes"]))
    {
      // Don't bother trying to resolve member types at this point
      // since the order is important so we would have to insert
      // the late resolutions into specific places. It is simpler
      // to just do the whole resolution later.
      //
      const wchar_t* data (members.c_str ());
      size_t size (members.size ());

      UnionMemberTypes* m (0);

      // Traverse the type list while logically collapsing spaces.
      //
      for (size_t i (find_ns (data, size, 0)); i != String::npos;)
      {
        String s;
        size_t j (find_s (data, size, i));

        if (j != String::npos)
        {
          s = String (data + i, j - i);
          i = find_ns (data, size, j);
        }
        else
        {
          // Last item.
          //
          s = String (data + i, size - i);
          i = String::npos;
        }

        if (trace_)
          wcout << "member type: " << fq_name (u, s) << endl;

        if (m == 0)
        {
          node.context ().set ("union-member-types", UnionMemberTypes ());
          m = &node.context ().get<UnionMemberTypes> ("union-member-types");
        }

        try
        {
          m->push_back (
            UnionMemberType (
              namespace_name (u, s), unqualified_name (s)));
        }
        catch (XML::NoMapping const& ex)
        {
          wcerr << file () << ":" << u.line () << ":" << u.column () << ": "
                << "error: unable to resolve namespace prefix "
                << "'" << ex.prefix () << "' in '" << s << "'" << endl;

          valid_ = false;
        }
      }

      has_members = (m != 0);
    }

    // Handle anonymous members.
    //
    push (u);

    annotation (false);

    while (more ())
    {
      XML::Element e (next ());
      String name (e.name ());

      if (trace_)
        wcout << name << endl;

      Type* t (0);

      if (name == L"simpleType")  t = simple_type (e);  else
      {
        wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
              << "error: expected 'simpleType' instead of "
              << "'" << e.name () << "'" << endl;

        valid_ = false;
      }

      if (t)
        s_->new_edge<Arguments> (*t, node);
    }

    pop ();

    if (node.argumented_begin () == node.argumented_end () && !has_members)
    {
      wcerr << file () << ":" << u.line () << ":" << u.column () << ": "
            << "error: expected 'memberTypes' attribute or 'simpleType' "
            << "nested element" << endl;

      valid_ = false;
    }

    if (String name = trim (t["name"]))
      s_->new_edge<Names> (scope (), node, name);

    return &node;
  }

  Type* Parser::Impl::
  restriction (XML::Element const& r, XML::Element const& t)
  {
    String base (trim (r["base"]));
    Type* base_type (0);

    if (base)
    {
      if (trace_)
        wcout << "restriction base: " << fq_name (r, base) << endl;
    }

    Type* rv (0);

    push (r);

    annotation (false);

    bool enum_ (false);

    if (!base)
    {
      // Anonymous base type.
      //
      if (more ())
      {
        XML::Element e (next ());

        String name (e.name ());

        if (trace_)
          wcout << name << endl;

        if (name == L"simpleType")  base_type = simple_type (e); else
        {
          wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
                << "error: expected 'simpleType' instead of "
                << "'" << e.name () << "'" << endl;

          valid_ = false;
        }
      }
      else
      {
        wcerr << file () << ":" << r.line () << ":" << r.column () << ": "
              << "error: expected 'base' attribute or 'simpleType' "
              << "nested element" << endl;

        valid_ = false;
      }

      if (!valid_)
      {
        pop ();
        return 0;
      }
    }

    Facets facets;
    Restricts* restricts (0);
    String pattern;

    while (more ())
    {
      XML::Element e (next ());
      String name (e.name ());

      if (name == L"enumeration")
      {
        // Enumeration
        //
        if (enum_)
          enumeration (e);
        else
        {
          // First
          //
          enum_ = true;

          Enumeration& node (
            s_->new_node<Enumeration> (file (), t.line (), t.column ()));

          if (base_type)
            restricts = &s_->new_edge<Restricts> (node, *base_type);
          else
            restricts = set_type<Restricts> (base, r, node);

          if (String name = trim (t["name"]))
            s_->new_edge<Names> (scope (), static_cast<Nameable&> (node), name);

          rv = &node;
          push_scope (node);
          enumeration (e);
        }
      }
      else if (name == L"minExclusive" ||
               name == L"minInclusive" ||
               name == L"maxExclusive" ||
               name == L"maxInclusive" ||
               name == L"totalDigits" ||
               name == L"fractionDigits" ||
               name == L"length" ||
               name == L"minLength" ||
               name == L"maxLength" ||
               name == L"whiteSpace")
      {
        facets[name] = trim (e["value"]);
      }
      else if (name == L"pattern")
      {
        if (pattern)
          pattern += L'|';

        pattern += e["value"];
      }
      else
      {
        wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
              << "error: unexpected element '" << name << "' in "
              << "simple type restriction" << endl;

        valid_ = false;
      }
    }

    if (pattern)
      facets[L"pattern"] = pattern;

    if (enum_)
      pop_scope ();
    else
    {
      // Really a simple type so not abstract/mixed checks.
      //
      Complex& node (s_->new_node<Complex> (
                       file (), t.line (), t.column (), false));

      if (base_type)
        restricts = &s_->new_edge<Restricts> (node, *base_type);
      else
        restricts = set_type<Restricts> (base, r, node);

      if (String name = trim (t["name"]))
        s_->new_edge<Names> (scope (), node, name);

      rv = &node;
    }

    if (!facets.empty ())
    {
      if (restricts)
        copy_facets (*restricts, facets);
      else
        rv->context ().set ("facets", facets);
    }

    pop ();

    return rv;
  }

  void Parser::Impl::
  enumeration (XML::Element const& e)
  {
    String value (e["value"]);

    if (trace_)
      wcout << "enumeration value: " << value << endl;

    push (e);
    Annotation* a (annotation (true));
    pop ();

    Enumerator& node (
      s_->new_node<Enumerator> (file (), e.line (), e.column ()));

    s_->new_edge<Names> (scope (), node, value);
    s_->new_edge<Belongs> (node, dynamic_cast<Type&>(scope ()));

    if (a != 0)
      s_->new_edge<Annotates> (*a, node);

  }

  Type* Parser::Impl::
  complex_type (XML::Element const& t)
  {
    Type* r (0);

    String abs_s (trim (t["abstract"]));
    bool abs (abs_s == L"true" || abs_s == L"1");

    Complex& node (s_->new_node<Complex> (
                     file (), t.line (), t.column (), abs));

    if (String m = trim (t["mixed"]))
      node.mixed_p (m == L"true" || m == L"1");

    if (String name = trim (t["name"]))
      s_->new_edge<Names> (scope (), node, name);

    r = &node;

    push_scope (node);
    push (t);

    if (Annotation* a = annotation (true))
      s_->new_edge<Annotates> (*a, node);

    if (more ())
    {
      XML::Element e (next ());

      String name (e.name ());

      if (trace_)
        wcout << name << endl;

      if (name == L"simpleContent")  simple_content (e);        else
      if (name == L"complexContent") complex_content (e, node); else
      {
        Compositor* c (0);

        if (name == L"all")            c = all (e);              else
        if (name == L"choice")         c = choice (e, false);    else
        if (name == L"sequence")       c = sequence (e, false);  else
        if (name == L"attribute")      attribute (e, false);     else
        if (name == L"anyAttribute")   any_attribute (e);        else
        if (name == L"group")          element_group (e, false); else
        if (name == L"attributeGroup") attribute_group (e);      else
        {
          wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
                << "error: unexpected element '" << name << "'" << endl;

          valid_ = false;
        }

        if (c)
        {
          unsigned long min (parse_min (trim (e["minOccurs"])));
          unsigned long max (parse_max (trim (e["maxOccurs"])));

          if (!(min == 0 && max == 0))
            s_->new_edge<ContainsCompositor> (
              node, *c, min, max == unbounded ? 0 : max);
        }

        while (more ())
        {
          XML::Element e (next ());
          String name (e.name ());

          if (name == L"attribute")      attribute (e, false); else
          if (name == L"anyAttribute")   any_attribute (e);    else
          if (name == L"attributeGroup") attribute_group (e);  else
          {
            wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
                  << "error: expected 'attribute', 'anyAttribute', or "
                  << "'attributeGroup' instead of '" << name << "'" << endl;

            valid_ = false;
          }
        }
      }
    }

    pop ();
    pop_scope ();

    return r;
  }

  All* Parser::Impl::
  all (XML::Element const& a)
  {
    // 'all' cannot be nested inside 'choice' or 'sequence', nor
    //  can it contain any of those. The only valid  cardinality
    //  values for 'all' are min=0,1 and max=1.
    //
    All& node (s_->new_node<All> (file (), a.line (), a.column ()));

    push_compositor (node);
    push (a);

    if (Annotation* a = annotation (true))
      s_->new_edge<Annotates> (*a, node);

    while (more ())
    {
      XML::Element e (next ());

      String name (e.name ());

      if (name == L"element") element (e, false); else
      {
        wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
              << "error: expected 'element' "
              << "instead of '" << name << "'" << endl;

        valid_ = false;
      }
    }

    pop ();
    pop_compositor ();

    return &node;
  }

  Choice* Parser::Impl::
  choice (XML::Element const& c, bool in_compositor)
  {
    Choice& node (s_->new_node<Choice> (file (), c.line (), c.column ()));

    if (in_compositor)
    {
      unsigned long min (parse_min (trim (c["minOccurs"])));
      unsigned long max (parse_max (trim (c["maxOccurs"])));

      if (!(min == 0 && max == 0))
        s_->new_edge<ContainsParticle> (
          compositor (), node, min, max == unbounded ? 0 : max);
    }

    push_compositor (node);
    push (c);

    if (Annotation* a = annotation (true))
      s_->new_edge<Annotates> (*a, node);

    while (more ())
    {
      XML::Element e (next ());

      String name (e.name ());

      if (name == L"any")      any (e);                 else
      if (name == L"choice")   choice (e, true);        else
      if (name == L"element")  element (e, false);      else
      if (name == L"sequence") sequence (e, true);      else
      if (name == L"group")    element_group (e, true); else
      {
        wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
              << "error: expected 'any', 'group', 'choice', 'sequence', "
              << "or 'element' instead of '" << name << "'" << endl;

        valid_ = false;
      }
    }

    pop ();
    pop_compositor ();

    return &node;
  }

  Sequence* Parser::Impl::
  sequence (XML::Element const& s, bool in_compositor)
  {
    Sequence& node (s_->new_node<Sequence> (file (), s.line (), s.column ()));

    if (in_compositor)
    {
      unsigned long min (parse_min (trim (s["minOccurs"])));
      unsigned long max (parse_max (trim (s["maxOccurs"])));

      if (!(min == 0 && max == 0))
        s_->new_edge<ContainsParticle> (
          compositor (), node, min, max == unbounded ? 0 : max);
    }

    push_compositor (node);
    push (s);

    if (Annotation* a = annotation (true))
      s_->new_edge<Annotates> (*a, node);

    while (more ())
    {
      XML::Element e (next ());

      String name (e.name ());

      if (name == L"any")      any (e);                 else
      if (name == L"choice")   choice (e, true);        else
      if (name == L"element")  element (e, false);      else
      if (name == L"sequence") sequence (e, true);      else
      if (name == L"group")    element_group (e, true); else
      {
        wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
              << "error: expected 'any', 'group', 'choice', 'sequence', "
              << "or 'element' instead of '" << name << "'" << endl;

        valid_ = false;
      }
    }

    pop ();
    pop_compositor ();

    return &node;
  }

  void Parser::Impl::
  simple_content (XML::Element const& c)
  {
    push (c);

    annotation (false);

    XML::Element e (next ());
    String name (e.name ());

    if (name == L"extension")   simple_content_extension (e);   else
    if (name == L"restriction") simple_content_restriction (e); else
    {
      wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
            << "error: expected 'extension' or 'restriction' instead of "
            << "'" << name << "'" << endl;

      valid_ = false;
    }

    pop ();
  }

  void Parser::Impl::
  complex_content (XML::Element const& c, Complex& type)
  {
    if (String m = trim (c["mixed"]))
      type.mixed_p (m == L"true" || m == L"1");

    push (c);

    annotation (false);

    XML::Element e (next ());
    String name (e.name ());

    if (name == L"extension")   complex_content_extension (e, type);   else
    if (name == L"restriction") complex_content_restriction (e, type); else
    {
      wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
            << "error: expected 'extension' or 'restriction' instead of "
            << "'" << name << "'" << endl;

      valid_ = false;
    }

    pop ();
  }

  void Parser::Impl::
  simple_content_extension (XML::Element const& e)
  {
    if (trace_)
      wcout << "extension base: " << fq_name (e, e["base"]) << endl;

    set_type<Extends> (trim (e["base"]), e, dynamic_cast<Complex&> (scope ()));

    push (e);

    annotation (false);

    while (more ())
    {
      XML::Element e (next ());
      String name (e.name ());

      if (name == L"attribute")      attribute (e, false); else
      if (name == L"anyAttribute")   any_attribute (e);    else
      if (name == L"attributeGroup") attribute_group (e);  else
      {
        wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
              << "error: expected 'attribute', 'anyAttribute', or "
              << "'attributeGroup' instead of '" << name << "'" << endl;

        valid_ = false;
      }
    }

    pop ();
  }

  void Parser::Impl::
  simple_content_restriction (XML::Element const& r)
  {
    String base (trim (r["base"]));

    if (trace_ && base)
      wcout << "restriction base: " << fq_name (r, base) << endl;

    push (r);
    annotation (false);

    if (!base)
    {
      // Anonymous base type.
      //
      if (more ())
      {
        XML::Element e (next ());
        String name (e.name ());

        if (trace_)
          wcout << name << endl;

        if (name == L"simpleType") simple_type (e); else
        {
          wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
                << "error: expected 'simpleType' instead of "
                << "'" << e.name () << "'" << endl;

          valid_ = false;
        }
      }
      else
      {
        wcerr << file () << ":" << r.line () << ":" << r.column () << ": "
              << "error: expected 'base' attribute or 'simpleType' "
              << "nested element" << endl;

        valid_ = false;
      }

      if (!valid_)
      {
        pop ();
        return;
      }
    }

    Facets facets;
    String pattern;

    while (more ())
    {
      XML::Element e (next ());
      String name (e.name ());

      if (name == L"simpleType")
      {
        // This is a "superimposed" restriction where the base
        // content is restricted by specifying another simple
        // type. The attributes are restricted in the ussual
        // way. So in effect we have kind of two base classes.
        // I guess the way to handle this one day would be to
        // copy all the facets from the base-to-this-type
        // part of the hierarchy (will need to "know" facets
        // for the built-in type restrictions as well). For
        // now just ignore it.
        //
      }
      else if (name == L"enumeration")
      {
        // Right now our sementic graph cannot represent enumerations
        // with attributes so we are going to ignore enumerators for
        // now.
        //
      }
      else if (name == L"minExclusive" ||
               name == L"minInclusive" ||
               name == L"maxExclusive" ||
               name == L"maxInclusive" ||
               name == L"totalDigits" ||
               name == L"fractionDigits" ||
               name == L"length" ||
               name == L"minLength" ||
               name == L"maxLength" ||
               name == L"whiteSpace")
      {
        facets[name] = trim (e["value"]);
      }
      else if (name == L"pattern")
      {
        if (pattern)
          pattern += L'|';

        pattern += e["value"];
      }
      else if (name == L"attribute")
      {
        if (proper_restriction_)
          attribute (e, false);
      }
      else if (name == L"anyAttribute")
      {
        if (proper_restriction_)
          any_attribute (e);
      }
      else if (name == L"attributeGroup")
      {
        if (proper_restriction_)
          attribute_group (e);
      }
      else
      {
        wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
              << "error: unexpected element '" << name << "' in "
              << "simple content restriction" << endl;

        valid_ = false;
      }
    }

    if (pattern)
      facets[L"pattern"] = pattern;

    Complex& type (dynamic_cast<Complex&> (scope ()));
    Restricts* restricts = set_type<Restricts> (base, r, type);

    if (!facets.empty ())
    {
      if (restricts)
        copy_facets (*restricts, facets);
      else
        type.context ().set ("facets", facets);
    }

    pop ();
  }

  void Parser::Impl::
  complex_content_extension (XML::Element const& e, Complex& type)
  {
    if (trace_)
      wcout << "extension base: " << fq_name (e, e["base"]) << endl;

    set_type<Extends> (trim (e["base"]), e, dynamic_cast<Complex&> (scope ()));

    push (e);

    annotation (false);

    if (more ())
    {
      XML::Element e (next ());
      String name (e.name ());
      Compositor* c (0);

      if (name == L"all")            c = all (e);              else
      if (name == L"choice")         c = choice (e, false);    else
      if (name == L"sequence")       c = sequence (e, false);  else
      if (name == L"attribute")      attribute (e, false);     else
      if (name == L"anyAttribute")   any_attribute (e);        else
      if (name == L"group")          element_group (e, false); else
      if (name == L"attributeGroup") attribute_group (e);      else
      {
        wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
              << "error: unexpected element '" << name << "'" << endl;

        valid_ = false;
      }

      if (c)
      {
        unsigned long min (parse_min (trim (e["minOccurs"])));
        unsigned long max (parse_max (trim (e["maxOccurs"])));

        if (!(min == 0 && max == 0))
          s_->new_edge<ContainsCompositor> (
            type, *c, min, max == unbounded ? 0 : max);
      }

      while (more ())
      {
        XML::Element e (next ());
        String name (e.name ());

        if (name == L"attribute")      attribute (e, false); else
        if (name == L"anyAttribute")   any_attribute (e);    else
        if (name == L"attributeGroup") attribute_group (e);  else
        {
          wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
                << "error: expected 'attribute', 'anyAttribute', or "
                << "'attributeGroup' instead of '" << name << "'" << endl;

          valid_ = false;
        }
      }
    }

    pop ();
  }

  void Parser::Impl::
  complex_content_restriction (XML::Element const& e, Complex& type)
  {
    if (trace_)
      wcout << "restriction base: " << fq_name (e, e["base"]) << endl;

    set_type<Restricts> (
      trim (e["base"]),
      e,
      dynamic_cast<Complex&> (scope ()));

    // @@
    // For now we simply skip the contents unless the base is anyType
    // (or a trivial alias thereof). Checking for the trivial alias
    // is further complicated by the fact that it might not be defined
    // at this stage (forward inheritnace) so we will ignore that case
    // as well for now.
    //
    if (!proper_restriction_)
    {
      String base (trim (e["base"]));
      String uq_name (unqualified_name (base));
      String ns_name (namespace_name (e, base));

      if (ns_name != xsd || uq_name != L"anyType")
        return;
    }

    push (e);

    annotation (false);

    if (more ())
    {
      XML::Element e (next ());
      String name (e.name ());
      Compositor* c (0);

      if (name == L"all")            c = all (e);              else
      if (name == L"choice")         c = choice (e, false);    else
      if (name == L"sequence")       c = sequence (e, false);  else
      if (name == L"attribute")      attribute (e, false);     else
      if (name == L"anyAttribute")   any_attribute (e);        else
      if (name == L"group")          element_group (e, false); else
      if (name == L"attributeGroup") attribute_group (e);      else
      {
        wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
              << "error: unexpected element '" << name << "'" << endl;

        valid_ = false;
      }

      if (c)
      {
        unsigned long min (parse_min (trim (e["minOccurs"])));
        unsigned long max (parse_max (trim (e["maxOccurs"])));

        if (!(min == 0 && max == 0))
          s_->new_edge<ContainsCompositor> (
            type, *c, min, max == unbounded ? 0 : max);
      }

      while (more ())
      {
        XML::Element e (next ());
        String name (e.name ());

        if (name == L"attribute")      attribute (e, false); else
        if (name == L"anyAttribute")   any_attribute (e);    else
        if (name == L"attributeGroup") attribute_group (e);  else
        {
          wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
                << "error: expected 'attribute', 'anyAttribute', or "
                << "'attributeGroup' instead of '" << name << "'" << endl;

          valid_ = false;
        }
      }
    }

    pop ();
  }

  void Parser::Impl::
  element (XML::Element const& e, bool global)
  {
    bool qualified (global ? true : qualify_element_);

    if (String form = trim (e["form"]))
      qualified = form == L"qualified";

    if (trace_)
      wcout << "element qualified: " << qualified << endl;

    if (String name = trim (e["name"]))
    {
      if (trace_)
        wcout << "element name '" << name << "'" << endl;

      Element& node (
        s_->new_node<Element> (
          file (), e.line (), e.column (), global, qualified));

      if (!global)
      {
        unsigned long min (parse_min (trim (e["minOccurs"])));
        unsigned long max (parse_max (trim (e["maxOccurs"])));

        if (!(min == 0 && max == 0))
        {
          s_->new_edge<Names> (scope (), node, name);

          s_->new_edge<ContainsParticle> (
            compositor (), node, min, max == unbounded ? 0 : max);
        }
      }
      else
        s_->new_edge<Names> (scope (), node, name);

      if (qualified)
        s_->new_edge<BelongsToNamespace> (node, cur_ns ());

      // Default and fixed values are mutually exclusive.
      //
      if (e.attribute_p ("fixed"))
        node.fixed (e.attribute ("fixed"));
      else if (e.attribute_p ("default"))
        node.default_ (e.attribute ("default"));

      if (node.default_p ())
      {
        node.context ().set ("dom-node", e.dom_element ());
        default_values_.push_back (&node);
      }

      bool subst (false);
      if (global)
      {
        if (String sg = trim (e["substitutionGroup"]))
        {
          if (trace_)
            wcout << "substitutes " << sg << endl;

          subst = true;

          try
          {
            String uq_name (unqualified_name (sg));
            String ns_name (namespace_name (e, sg));

            node.context ().set ("substitution-ns-name", ns_name);
            node.context ().set ("substitution-uq-name", uq_name);
          }
          catch (XML::NoMapping const& ex)
          {
            wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
                  << "error: unable to resolve namespace prefix '"
                  << ex.prefix () << "' in '" << sg << "'" << endl;

            valid_ = false;
          }
        }
      }

      if (String type = trim (e["type"]))
      {
        if (trace_)
          wcout << "element type " << fq_name (e, type) << endl;

        set_type<Belongs> (type, e, node);

        // Parse annotation.
        //
        push (e);

        if (Annotation* a = annotation (true))
          s_->new_edge<Annotates> (*a, node);

        pop ();
      }
      else
      {
        // Looks like an anonymous type.
        //
        push (e);

        if (Annotation* a = annotation (true))
          s_->new_edge<Annotates> (*a, node);

        if (more ())
        {
          XML::Element e (next ());

          String name (e.name ());

          if (trace_)
            wcout << name << endl;

          Type* t (0);

          if (name == L"simpleType")  t = simple_type (e);  else
          if (name == L"complexType") t = complex_type (e); else
          {
            wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
                  << "error: expected 'simpleType' or 'complexType' "
                  << "instead of '" << e.name () << "'" << endl;

            valid_ = false;
          }

          if (t)
            s_->new_edge<Belongs> (node, *t);
        }
        // By default the type is anyType unless this element is a
        // member of a substitution group, in which case it has the
        // same type as the element it substiutes.
        //
        else if (!subst)
        {
          if (!is_disabled ("F001"))
          {
            wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
                  << "warning F001: element '" << name << "' is implicitly "
                  << "of anyType" << endl;

            wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
                  << "info: did you forget to specify 'type' attribute?"
                  << endl;
          }

          String prefix (ns_prefix (e, xsd));
          type =  prefix + (prefix.empty () ? L"" : L":") + L"anyType";

          set_type<Belongs> (type, e, node);
        }

        pop ();
      }
    }
    else if (String ref = trim (e["ref"]))
    {
      Element& node (
        s_->new_node<Element> (
          file (), e.line (), e.column (), true, true));

      unsigned long min (parse_min (trim (e["minOccurs"])));
      unsigned long max (parse_max (trim (e["maxOccurs"])));

      // Default and fixed values are mutually exclusive.
      //
      if (e.attribute_p ("fixed"))
        node.fixed (e.attribute ("fixed"));
      else if (e.attribute_p ("default"))
        node.default_ (e.attribute ("default"));

      if (node.default_p ())
      {
        node.context ().set ("dom-node", e.dom_element ());
        default_values_.push_back (&node);
      }

      // Parse annotation.
      //
      push (e);

      if (Annotation* a = annotation (true))
        s_->new_edge<Annotates> (*a, node);

      pop ();

      if (!(min == 0 && max == 0))
      {
        // Ref can only be in compositor.
        //
        s_->new_edge<ContainsParticle> (
          compositor (), node, min, max == unbounded ? 0 : max);

        // Try to resolve the prototype.
        //
        try
        {
          String uq_name (unqualified_name (ref));
          String ns_name (namespace_name (e, ref));

          s_->new_edge<Names> (scope (), node, uq_name);

          Element& prot (resolve<Element> (ns_name, uq_name, *s_, *cache_));
          s_->new_edge<BelongsToNamespace> (node, prot.namespace_ ());

          // Copy substitution group information if any.
          //
          if (prot.context ().count ("substitution-ns-name"))
          {
            node.context ().set (
              "substitution-ns-name",
              prot.context ().get<String> ("substitution-ns-name"));

            node.context ().set (
              "substitution-uq-name",
              prot.context ().get<String> ("substitution-uq-name"));
          }

          // Transfer default and fixed values if the ref declaration hasn't
          // defined its own.
          //
          if (!node.default_p ())
          {
            if (prot.fixed_p ())
              node.fixed (prot.value ());
            else if (prot.default_p ())
              node.default_ (prot.value ());

            if (node.default_p ())
            {
              node.context ().set (
                "dom-node",
                prot.context ().get<Xerces::DOMElement*> ("dom-node"));
              default_values_.push_back (&node);
            }
          }

          // Transfer annotation if the ref declaration hasn't defined its own.
          //
          if (!node.annotated_p () && prot.annotated_p ())
            s_->new_edge<Annotates> (prot.annotation (), node);

          // Set type information.
          //
          if (prot.typed_p ())
          {
            s_->new_edge<Belongs> (node, prot.type ());
          }
          else if (prot.context ().count ("type-ns-name"))
          {
            String ns_name (prot.context ().get<String> ("type-ns-name"));
            String uq_name (prot.context ().get<String> ("type-uq-name"));

            node.context ().set ("type-ns-name", ns_name);
            node.context ().set ("type-uq-name", uq_name);
            node.context ().set ("edge-type-id", type_id (typeid (Belongs)));

            if (trace_)
              wcout << "element '" << ref << "' is not typed" << endl
                    << "deferring resolution until later" << endl;
          }
          else
          {
            // This could be a recursive reference to an element who's
            // (anonymous) type is being defined. We are going to let
            // resolver sort out this case.
            //
            node.context ().set ("instance-ns-name", ns_name);
            node.context ().set ("instance-uq-name", uq_name);

            if (trace_)
              wcout << "looks like a recursive reference to an element '"
                    << ns_name << "#" << uq_name << "' which is being "
                    << "defined" << endl
                    << "deferring resolution until later" << endl;
          }
        }
        catch (NotNamespace const& ex)
        {
          if (valid_)
          {
            wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
                  << "ice: unable to resolve namespace '" << ex.ns () << "'"
                  << endl;

            abort ();
          }
        }
        catch (NotName const& ex)
        {
          node.context ().set ("instance-ns-name", ex.ns ());
          node.context ().set ("instance-uq-name", ex.name ());

          if (trace_)
            wcout << "unable to resolve name '" << ex.name ()
                  << "' inside namespace '" << ex.ns () << "'" << endl
                  << "deferring resolution until later" << endl;
        }
        catch (XML::NoMapping const& ex)
        {
          wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
                << "error: unable to resolve namespace prefix '"
                << ex.prefix () << "' in '" << ref << "'" << endl;

          valid_ = false;
        }
      }
    }
    else
    {
      if (valid_)
      {
        wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
              << "error: 'name' or 'ref' attribute is missing in element "
              << "declaration" << endl;
      }
    }
  }

  SemanticGraph::Annotation* Parser::Impl::
  annotation (bool process)
  {
    Annotation* r (0);

    if (more ())
    {
      XML::Element e (next ());

      if (e.name () == L"annotation")
      {
        if (process)
        {
          push (e);

          while (more ())
          {
            XML::Element doc (next ());

            if (doc.name () == L"documentation")
            {
              using Xerces::DOMNode;
              using Xerces::DOMText;
              using Xerces::DOMElement;

              // Use first non-structured (text only) documentation element.
              //
              String text;
              bool struc (false);
              DOMElement* de (doc.dom_element());

              for (DOMNode* n (de->getFirstChild ());
                   n != 0 && !struc;
                   n = n->getNextSibling ())
              {
                switch (n->getNodeType ())
                {
                case DOMNode::TEXT_NODE:
                case DOMNode::CDATA_SECTION_NODE:
                  {
                    DOMText* t (static_cast<DOMText*> (n));
                    text += XML::transcode (t->getData ());
                    break;
                  }
                case DOMNode::ELEMENT_NODE:
                  {
                    struc = true;
                    break;
                  }
                default:
                  break; // ignore
                }
              }

              if (struc)
                continue;

              r = &s_->new_node<Annotation> (
                file (), e.line (), e.column (), text);
              break;
            }
          }

          pop ();
        }
      }
      else
        prev ();
    }

    return r;
  }


  void Parser::Impl::
  attribute (XML::Element const& a, bool global)
  {
    bool optional (true);

    String use (trim (a["use"]));

    if (use == L"prohibited")
      return;
    else if (use == L"required")
      optional = false;

    bool qualified (global ? true : qualify_attribute_);

    if (String form = trim (a["form"]))
      qualified = form == L"qualified";

    if (String name = trim (a["name"]))
    {
      if (trace_)
        wcout << "attribute '" << name << "'" << endl;

      Attribute& node (
        s_->new_node<Attribute> (
          file (), a.line (), a.column (), optional, global, qualified));

      s_->new_edge<Names> (scope (), node, name);

      if (qualified)
        s_->new_edge<BelongsToNamespace> (node, cur_ns ());


      // Default and fixed values are mutually exclusive.
      //
      if (a.attribute_p ("fixed"))
        node.fixed (a.attribute ("fixed"));
      else if (a.attribute_p ("default"))
        node.default_ (a.attribute ("default"));

      if (node.default_p ())
      {
        node.context ().set ("dom-node", a.dom_element ());
        default_values_.push_back (&node);
      }

      if (String type = trim (a["type"]))
      {
        if (trace_)
          wcout << "attribute type: '" << fq_name (a, type) << "'" << endl;

        set_type<Belongs> (type, a, node);

        // Parse annotation.
        //
        push (a);

        if (Annotation* ann = annotation (true))
          s_->new_edge<Annotates> (*ann, node);

        pop ();
      }
      else
      {
        // Looks like an anonymous type.
        //
        push (a);

        if (Annotation* ann = annotation (true))
          s_->new_edge<Annotates> (*ann, node);

        if (more ())
        {
          XML::Element e (next ());

          String name (e.name ());

          if (trace_)
            wcout << name << endl;

          Type* t (0);

          if (name == L"simpleType") t = simple_type (e); else
          {
            wcerr << file () << ":" << a.line () << ":" << a.column () << ": "
                  << "error: expected 'simpleType' instead of '" << e.name ()
                  << "'" << endl;

            valid_ = false;
          }

          if (t)
            s_->new_edge<Belongs> (node, *t);
        }
        else
        {
          if (!is_disabled ("F002"))
          {
            wcerr << file () << ":" << a.line () << ":" << a.column () << ": "
                  << "warning F002: attribute '" << name << "' is implicitly "
                  << "of anySimpleType" << endl;

            wcerr << file () << ":" << a.line () << ":" << a.column () << ": "
                  << "info: did you forget to specify 'type' attribute?"
                  << endl;
          }

          // anySimpleType
          //
          String prefix (ns_prefix (a, xsd));
          type =  prefix + (prefix.empty () ? L"" : L":") + L"anySimpleType";

          set_type<Belongs> (type, a, node);
        }

        pop ();
      }
    }
    else if (String ref = trim (a["ref"]))
    {
      Attribute& node (
        s_->new_node<Attribute> (
          file (), a.line (), a.column (), optional, true, true));


      // Default and fixed values are mutually exclusive.
      //
      if (a.attribute_p ("fixed"))
        node.fixed (a.attribute ("fixed"));
      else if (a.attribute_p ("default"))
        node.default_ (a.attribute ("default"));

      if (node.default_p ())
      {
        node.context ().set ("dom-node", a.dom_element ());
        default_values_.push_back (&node);
      }

      // Parse annotation.
      //
      push (a);

      if (Annotation* ann = annotation (true))
        s_->new_edge<Annotates> (*ann, node);

      pop ();

      try
      {
        String uq_name (unqualified_name (ref));
        String ns_name (namespace_name (a, ref));

        s_->new_edge<Names> (scope (), node, uq_name);

        Attribute& prot (resolve<Attribute> (ns_name, uq_name, *s_, *cache_));
        s_->new_edge<BelongsToNamespace> (node, prot.namespace_ ());

        // Transfer default and fixed values if the ref declaration hasn't
        // defined its own.
        //
        if (!node.default_p ())
        {
          // Default value applies only if this attribute is optional.
          //
          if (prot.fixed_p ())
            node.fixed (prot.value ());
          else if (optional && prot.default_p ())
            node.default_ (prot.value ());

          if (node.default_p ())
          {
            node.context ().set (
              "dom-node",
              prot.context ().get<Xerces::DOMElement*> ("dom-node"));
            default_values_.push_back (&node);
          }
        }

        // Transfer annotation if the ref declaration hasn't defined its own.
        //
        if (!node.annotated_p () && prot.annotated_p ())
          s_->new_edge<Annotates> (prot.annotation (), node);

        // Set type.
        //
        if (prot.typed_p ())
        {
          s_->new_edge<Belongs> (node, prot.type ());
        }
        else if (prot.context ().count ("type-ns-name"))
        {
          String ns_name (prot.context ().get<String> ("type-ns-name"));
          String uq_name (prot.context ().get<String> ("type-uq-name"));

          node.context ().set ("type-ns-name", ns_name);
          node.context ().set ("type-uq-name", uq_name);
          node.context ().set ("edge-type-id", type_id (typeid (Belongs)));

          if (trace_)
            wcout << "attribute '" << ref << "' is not typed" << endl
                  << "deferring resolution until later" << endl;
        }
        else
        {
          // This could be a recursive reference to an attribute who's
          // (anonymous) type is being defined. We are going to let
          // resolver sort out this case.
          //
          node.context ().set ("instance-ns-name", ns_name);
          node.context ().set ("instance-uq-name", uq_name);

          if (trace_)
            wcout << "looks like a recursive reference to an attribute '"
                  << ns_name << "#" << uq_name << "' which is being "
                  << "defined" << endl
                  << "deferring resolution until later" << endl;
        }
      }
      catch (NotNamespace const& ex)
      {
        if (valid_)
        {
          wcerr << file () << ":" << a.line () << ":" << a.column () << ": "
                << "ice: unable to resolve namespace '" << ex.ns () << "'"
                << endl;
          abort ();
        }
      }
      catch (NotName const& ex)
      {
        node.context ().set ("instance-ns-name", ex.ns ());
        node.context ().set ("instance-uq-name", ex.name ());

        if (trace_)
          wcout << "unable to resolve name '" << ex.name ()
                << "' inside namespace '" << ex.ns () << "'" << endl
                << "deferring resolution until later" << endl;
      }
      catch (XML::NoMapping const& ex)
      {
        wcerr << file () << ":" << a.line () << ":" << a.column () << ": "
              << "error: unable to resolve namespace prefix '"
              << ex.prefix () << "' in '" << ref << "'" << endl;

        valid_ = false;
      }
    }
    else
    {
      if (valid_)
      {
        wcerr << file () << ":" << a.line () << ":" << a.column () << ": "
              << "error: 'name' or 'ref' attribute is missing in attribute "
              << "declaration" << endl;
      }
    }
  }

  void Parser::Impl::
  attribute_group (XML::Element const& g)
  {
    if (String name = trim (g["name"]))
    {
      // Global definition.
      //
      if (trace_)
        wcout << "attributeGroup '" << name << "'" << endl;

      AttributeGroup& group (
        s_->new_node<AttributeGroup> (file (), g.line (), g.column ()));
      s_->new_edge<Names> (scope (), group, name);

      push_scope (group);
      push (g);

      annotation (false);

      while (more ())
      {
        XML::Element e (next ());
        String name (e.name ());

        if (trace_)
          wcout << name << endl;

        if (name == L"attribute")      attribute (e, false); else
        if (name == L"anyAttribute")   any_attribute (e);    else
        if (name == L"attributeGroup") attribute_group (e);  else
        {
          wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
                << "error: expected 'attribute', 'anyAttribute', or "
                << "'attributeGroup' instead of '" << name << "'" << endl;

          valid_ = false;
        }
      }

      pop ();
      pop_scope ();
    }
    else if (String ref = trim (g["ref"]))
    {
      if (trace_)
        wcout << "attribute-group-ref " << ref << endl;

      try
      {
        String uq_name (unqualified_name (ref));
        String ns_name (namespace_name (g, ref));

        // In order to avoid code duplication we are going to let the
        // resolver handle this case.
        //
        if (trace_)
          wcout << "deferring resolution of group name '" << uq_name
                << "' inside namespace '" << ns_name << "'"
                << " until later" << endl;

        Scope& s (scope ());
        AttributeGroupRef ref (uq_name, ns_name, s);

        if (!s.context ().count ("attribute-group-refs"))
          s.context ().set ("attribute-group-refs", AttributeGroupRefs ());

        s.context ().get<AttributeGroupRefs> (
          "attribute-group-refs").push_back (ref);
      }
      catch (NotNamespace const& ex)
      {
        if (valid_)
        {
          wcerr << file () << ":" << g.line () << ":" << g.column () << ": "
                << "ice: unable to resolve namespace '" << ex.ns () << "'"
                << endl;
          abort ();
        }
      }
      catch (XML::NoMapping const& ex)
      {
        wcerr << file () << ":" << g.line () << ":" << g.column () << ": "
              << "error: unable to resolve namespace prefix '"
              << ex.prefix () << "' in '" << ref << "'" << endl;

        valid_ = false;
      }
    }
    else
    {
      wcerr << file () << ":" << g.line () << ":" << g.column () << ": "
            << "error: 'name' or 'ref' attribute is missing in "
            << "attributeGroup declaration" << endl;

      valid_ = false;
      return;
    }
  }

  void Parser::Impl::
  any (XML::Element const& a)
  {
    if (trace_)
      wcout << "any" << endl;

    String namespaces (trim (a["namespace"]));

    if (!namespaces)
      namespaces = L"##any";

    Any& any (
      s_->new_node<Any> (file (), a.line (), a.column (), namespaces));

    unsigned long min (parse_min (trim (a["minOccurs"])));
    unsigned long max (parse_max (trim (a["maxOccurs"])));

    // Parse annotation.
    //
    push (a);

    if (Annotation* ann = annotation (true))
      s_->new_edge<Annotates> (*ann, any);

    pop ();

    if (!(min == 0 && max == 0))
    {
      s_->new_edge<ContainsParticle> (
        compositor (), any, min, max == unbounded ? 0 : max);

      // Any has no name so we have to come up with a fake one in order to
      // put it into the scope.
      //
      unsigned long count;
      SemanticGraph::Context& ctx (scope ().context ());

      if (!ctx.count ("any-name-count"))
      {
        count = 0;
        ctx.set ("any-name-count", count);
      }
      else
        count = ++(ctx.get<unsigned long> ("any-name-count"));

      std::basic_ostringstream<wchar_t> os;
      os << "any #" << count;

      s_->new_edge<Names> (scope (), any, os.str ());
    }
  }

  void Parser::Impl::
  any_attribute (XML::Element const& a)
  {
    if (trace_)
      wcout << "anyAttribute" << endl;

    String namespaces (trim (a["namespace"]));

    if (!namespaces)
      namespaces = L"##any";

    AnyAttribute& any (
      s_->new_node<AnyAttribute> (
        file (), a.line (), a.column (), namespaces));

    // Parse annotation.
    //
    push (a);

    if (Annotation* ann = annotation (true))
      s_->new_edge<Annotates> (*ann, any);

    pop ();

    // AnyAttribute has no name so we have to come up with a fake one
    // in order to put it into the scope.
    //

    unsigned long count;
    SemanticGraph::Context& ctx (scope ().context ());

    if (!ctx.count ("any-attribute-name-count"))
    {
      count = 0;
      ctx.set ("any-attribute-name-count", count);
    }
    else
      count = ++(ctx.get<unsigned long> ("any-attribute-name-count"));

    std::basic_ostringstream<wchar_t> os;
    os << "any-attribute #" << count;

    s_->new_edge<Names> (scope (), any, os.str ());
  }

  // Some specializations to get edge orientations right.
  //

  template <typename Edge, typename Node>
  struct Orientation
  {
    static Edge&
    set_edge (Schema& s, Node& node, Type& type)
    {
      // By default it is node->edge
      //
      return s.template new_edge<Edge> (node, type);
    }
  };

  template <typename Node>
  struct Orientation<Arguments, Node>
  {
    static Arguments&
    set_edge (Schema& s, Node& node, Type& type)
    {
      // For Arguments it is type->node.
      //
      return s.template new_edge<Arguments> (type, node);
    }
  };

  template <typename Edge, typename Node>
  Edge* Parser::Impl::
  set_type (String const& type, XML::Element const& e, Node& node)
  {
    Edge* r (0);

    try
    {
      String uq_name (unqualified_name (type));
      String ns_name (namespace_name (e, type));

      Type& t (resolve<Type> (ns_name, uq_name, *s_, *cache_));

      // See if it is an IDREF specialization.
      //
      if (ns_name == xsd && (uq_name == L"IDREF" || uq_name == L"IDREFS"))
      {
        // See if we've got 'xse:refType' attribute.
        //
        if (String ref_type = trim (e.attribute (xse, "refType")))
        {
          if (trace_)
            wcout << "found refType attribute '" << ref_type << "'" << endl;

          //@@ It is a bit wasteful to create a new spcialization for
          //   each refType. Instead we could lookup the target type
          //   and then navigate through Arguments edges to see if this
          //   type already arguments specialization that we are intersted
          //   in. But for now I will simplify the logic by creating a new
          //   specialization every time.
          //

          Specialization* spec (0);

          if (uq_name == L"IDREF")
            spec = &s_->new_node<Fundamental::IdRef> (
              file (), e.line (), e.column ());
          else
            spec = &s_->new_node<Fundamental::IdRefs> (
              file (), e.line (), e.column ());

          r = &Orientation<Edge, Node>::set_edge (*s_, node, *spec);

          set_type<Arguments> (ref_type, e, *spec);
        }
        else
          r = &Orientation<Edge, Node>::set_edge (*s_, node, t);
      }
      else
        r = &Orientation<Edge, Node>::set_edge (*s_, node, t);
    }
    catch (NotNamespace const& ex)
    {
      wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
            << "error: unable to resolve namespace '" << ex.ns () << "'"
            << endl;

      valid_ = false;

    }
    catch (NotName const& ex)
    {
      node.context ().set ("type-ns-name", ex.ns ());
      node.context ().set ("type-uq-name", ex.name ());
      node.context ().set ("edge-type-id", type_id (typeid (Edge)));

      if (trace_)
        wcout << "unable to resolve name '" << ex.name ()
              << "' inside namespace '" << ex.ns () << "'" << endl
              << "deferring resolution until later" << endl;
    }
    catch (XML::NoMapping const& ex)
    {
      wcerr << file () << ":" << e.line () << ":" << e.column () << ": "
            << "error: unable to resolve namespace prefix "
            << "'" << ex.prefix () << "' in '" << type << "'" << endl;

      valid_ = false;
    }

    return r;
  }

  // Xerces has a provision to associate a public id with input streams
  // that can later be used in diagnostics. Unfortunately, it doesn't
  // work. So we will have to keep our own track.
  //
  struct Context
  {
    Context () {}

    // File map for diagnostic.
    //
    Path const&
    file (Path const& abs) const
    {
      FileMap::const_iterator i (file_map_.find (abs));

      if (i != file_map_.end ())
      {
        return i->second;
      }
      else
      {
        return abs;
      }
    }

    void
    map_file (Path const& abs, Path const& rel)
    {
      file_map_[abs] = rel;
    }

  private:
    Context (Context const&);
    Context& operator= (Context const&);

  private:
    typedef std::map<Path, Path> FileMap;
    FileMap file_map_;
  };

  //
  //
  class ErrorHandler : public  Xerces::DOMErrorHandler
  {
  public:
    ErrorHandler (bool& valid, XSDFrontend::Context const& ctx)
        : valid_ (valid),
          ctx_ (ctx)
    {
    }

    virtual bool
    handleError (Xerces::DOMError const& e)
    {
      // Xerces likes to say "Fatal error encountered during schema scan".
      // We don't need this junk.
      //
      if (!valid_
          && e.getLocation ()->getLineNumber () == 0
          && e.getLocation ()->getColumnNumber () == 0)
        return true;


      XSDFrontend::SemanticGraph::Path abs_path (
        XML::transcode_to_narrow (e.getLocation ()->getURI ()));

      XSDFrontend::SemanticGraph::Path rel_path (ctx_.file (abs_path));

      wcerr << rel_path << ':'
            << e.getLocation ()->getLineNumber () << ':'
            << e.getLocation ()->getColumnNumber () << ": ";

      switch (e.getSeverity ())
      {
      case Xerces::DOMError::DOM_SEVERITY_WARNING:
        {
          wcerr << "warning: ";
          break;
        }
      default:
        {
          wcerr << "error: ";
          valid_ = false;
          break;
        }
      }

      wcerr << e.getMessage () << endl;

      return true;
    }

  private:
    bool& valid_;
    XSDFrontend::Context const& ctx_;
  };


  // Failed to open resource.
  //
  struct Open {};

  class InputSource: public Xerces::InputSource
  {
  public:
    InputSource (
      Path const& abs,
      Path const& rel,
      Path const& base,
      XSDFrontend::Context const& ctx,
      Xerces::MemoryManager* mm = Xerces::XMLPlatformUtils::fgMemoryManager)
        : Xerces::InputSource (mm),
          abs_ (abs),
          rel_ (rel),
          base_ (base),
          ctx_ (ctx)
    {
      setSystemId (XML::XMLChString (String (abs_.string ())).c_str ());
    }

    virtual Xerces::BinInputStream*
    makeStream () const
    {
      using namespace Xerces;

      BinFileInputStream* is (
        new (getMemoryManager ())
        BinFileInputStream (getSystemId (), getMemoryManager ()));

      if (!is->getIsOpen ())
      {
        delete is;

        wcerr << ctx_.file (base_) << ": error: "
              << "'" << rel_ << "': unable to open in read mode"
              << endl;

        throw Open ();
      }

      return is;
    }

  private:
    Path abs_;
    Path rel_;
    Path base_;
    XSDFrontend::Context const& ctx_;
  };


  class EntityResolver: public Xerces::XMemory,
                        public Xerces::DOMLSResourceResolver
  {
  public:
    EntityResolver (XSDFrontend::Context& ctx, LocationTranslator* t)
        : ctx_ (ctx), loc_translator_ (t)
    {
    }

    virtual Xerces::DOMLSInput*
    resolveResource(XMLCh const* const,
                    XMLCh const* const,
                    XMLCh const* const /*pub_id*/,
                    XMLCh const* const prv_id,
                    XMLCh const* const base_uri)
    {
      /*
      XMLCh empty[1];
      empty[0] = 0;

      wcerr << "resolve entity:" << endl
            << "  pub_id " << (pub_id ? pub_id : empty) << endl
            << "  prv_id " << (prv_id ? prv_id : empty) << endl
            << "     uri " << (base_uri ? base_uri : empty) << endl;
      */

      // base_uri should be a valid path by now.
      //
      Path base (XML::transcode_to_narrow (base_uri));

      if (prv_id == 0)
      {
        //@@ How can I get the line/column numbers for this?
        //
        wcerr << ctx_.file (base) << ": error: "
              << "unable to guess which schema to open"
              << endl;

        wcerr << ctx_.file (base) << ": info: "
              << "did you forget to specify schemaLocation for import/include?"
              << endl;

        throw Open ();
      }

      NarrowString path_str (XML::transcode_to_narrow (prv_id));

      if (loc_translator_)
        path_str = loc_translator_->translate (path_str);

      try
      {
        Path path (path_str);
        Path base_dir (base.directory ());

        Path abs_path, rel_path;

        if (path.absolute ())
        {
          abs_path = rel_path = path;
        }
        else
        {
          abs_path = base_dir / path;
          rel_path = ctx_.file (base).directory () / path;
        }

        abs_path.normalize ();

        ctx_.map_file (abs_path, rel_path);

        using namespace Xerces;

        InputSource* is (
          new (XMLPlatformUtils::fgMemoryManager)
          InputSource (abs_path, rel_path, base, ctx_));

        // Note that I can't use XMLPlatformUtils::fgMemoryManager here
        // since Wrapper4InputSource is-not-an XMemory.
        //
        return new Wrapper4InputSource (is);
      }
      catch (InvalidPath const&)
      {
        wcerr << ctx_.file (base) << ": error: "
              << "'" << path_str.c_str () << "' is not a valid filesystem path"
              << endl;
        throw;
      }

      // Will never reach.
      //
      return 0;
    }

  private:
    XSDFrontend::Context& ctx_;
    LocationTranslator* loc_translator_;
  };


  XML::AutoPtr<Xerces::DOMDocument> Parser::Impl::
  dom (Path const& tu, bool validate)
  {
    using namespace Xerces;

    try
    {
      XSDFrontend::Context ctx;

      // Do normalize() before complete() to avoid hitting system path
      // limits with '..' directories.
      //
      Path abs_path (tu);
      abs_path.normalize ().complete ();
      ctx.map_file (abs_path, tu);

      InputSource input_source (abs_path, tu, abs_path, ctx);

      // First validate the schema with Xerces.
      //
      if (validate)
      {
        // Instantiate the DOM parser.
        //
        XMLCh const gLS[] = {chLatin_L, chLatin_S, chNull };

        // Get an implementation of the Load-Store (LS) interface.
        //
        DOMImplementationLS* impl (
          static_cast<DOMImplementationLS*> (
            DOMImplementationRegistry::getDOMImplementation (gLS)));

        // Create a DOMBuilder.
        //
        XML::AutoPtr<DOMLSParser> parser (
          impl->createLSParser (DOMImplementationLS::MODE_SYNCHRONOUS, 0));

        DOMConfiguration* conf (parser->getDomConfig ());

        conf->setParameter (XMLUni::fgDOMComments, false);
        conf->setParameter (XMLUni::fgDOMDatatypeNormalization, true);
        conf->setParameter (XMLUni::fgDOMEntities, false);
        conf->setParameter (XMLUni::fgDOMNamespaces, true);
        conf->setParameter (XMLUni::fgDOMValidate, true);
        conf->setParameter (XMLUni::fgDOMElementContentWhitespace, false);
        conf->setParameter (XMLUni::fgXercesSchema, true);

        // Xerces-C++ 3.1.0 is the first version with working multi import
        // support.
        //
#if _XERCES_VERSION >= 30100
        conf->setParameter (XMLUni::fgXercesHandleMultipleImports, multiple_imports_);
#endif

        conf->setParameter (XMLUni::fgXercesSchemaFullChecking, full_schema_check_);
        conf->setParameter (XMLUni::fgXercesValidationErrorAsFatal, true);

        ErrorHandler eh (valid_, ctx);
        conf->setParameter (XMLUni::fgDOMErrorHandler, &eh);

        EntityResolver er (ctx, loc_translator_);
        conf->setParameter (XMLUni::fgDOMResourceResolver, &er);

        Wrapper4InputSource wrap (&input_source, false);
        parser->loadGrammar (&wrap, Grammar::SchemaGrammarType);
      }

      if (!valid_)
        return XML::AutoPtr<DOMDocument> (0);

      // Now do our own parsing.
      //
      std::auto_ptr<XML::SchemaDOMParser> xsd_parser (
        new (XMLPlatformUtils::fgMemoryManager) XML::SchemaDOMParser ());

      xsd_parser->parse (input_source);

      XML::AutoPtr<DOMDocument> doc (xsd_parser->adoptDocument());

      return doc;
    }
    catch (Xerces::XMLException const& e)
    {
      wcerr << tu << ": ice: Xerces::XMLException: " << e.getMessage ()
            << endl;

      abort ();
    }
    catch (Xerces::DOMException const& e)
    {
      size_t const size = 2047;
      XMLCh text[size + 1];

      wcerr << tu << ": ice: Xerces::DOMException: ";

      if (DOMImplementation::loadDOMExceptionMsg (e.code, text, size))
        wcerr << text << endl;
      else
        wcerr << "no message available, error code: " << e.code << endl;

      abort ();
    }
    catch (InvalidPath const&)
    {
      // Diagnostics has already been issued.
      //
      valid_ = false;
    }
    catch (Open const&)
    {
      // Diagnostics has already been issued.
      //
      valid_ = false;
    }

    return XML::AutoPtr<DOMDocument> (0);
  }

  // LocationTranslator
  //
  LocationTranslator::
  ~LocationTranslator ()
  {
  }

  // Parser
  //
  Parser::
  ~Parser ()
  {
  }

  Parser::
  Parser (bool proper_restriction,
          bool multiple_imports,
          bool full_schema_check)
      : impl_ (new Impl (proper_restriction,
                         multiple_imports,
                         full_schema_check,
                         0,
                         0))
  {
  }

  Parser::
  Parser (bool proper_restriction,
          bool multiple_imports,
          bool full_schema_check,
          LocationTranslator& t,
          const WarningSet& d)
      : impl_ (new Impl (proper_restriction,
                         multiple_imports,
                         full_schema_check,
                         &t,
                         &d))
  {
  }

  auto_ptr<SemanticGraph::Schema> Parser::
  parse (SemanticGraph::Path const& path)
  {
    return impl_->parse (path);
  }

  auto_ptr<SemanticGraph::Schema> Parser::
  parse (SemanticGraph::Paths const& paths)
  {
    return impl_->parse (paths);
  }

  auto_ptr<SemanticGraph::Schema> Parser::
  xml_schema (SemanticGraph::Path const& path)
  {
    return impl_->xml_schema (path);
  }
}
