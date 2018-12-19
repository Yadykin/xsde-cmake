// file      : xsd-frontend/xml.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_XML_HXX
#define XSD_FRONTEND_XML_HXX

#include <vector>
#include <ostream>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>

#include <xsd-frontend/version.hxx>            // Check Xerces-C++ version.
#include <xsd-frontend/types.hxx>
#include <xsd-frontend/schema-dom-parser.hxx>

namespace XSDFrontend
{
  namespace XML
  {
    namespace Xerces = xercesc;

    inline
    String
    transcode (XMLCh const* s, size_t length)
    {
      if (sizeof (wchar_t) == 4)
      {
        // UTF-32
        //
        XMLCh const* end (s + length);

        // Find what the resulting buffer size will be.
        //
        size_t rl (0);
        bool valid (true);

        for (XMLCh const* p (s); p < end; ++p)
        {
          rl++;

          if ((*p >= 0xD800) && (*p <= 0xDBFF))
          {
            // Make sure we have one more char and it has a valid
            // value for the second char in a surrogate pair.
            //
            if (++p == end || !((*p >= 0xDC00) && (*p <= 0xDFFF)))
            {
              valid = false;
              break;
            }
          }
        }

        if (!valid)
          return String ();

        String r;
        r.reserve (rl + 1);
        r.resize (rl);
        wchar_t* rs (const_cast<wchar_t*> (r.c_str ()));

        size_t i (0);

        for (XMLCh const* p (s); p < end; ++p)
        {
          XMLCh x (*p);

          if (x < 0xD800 || x > 0xDBFF)
            rs[i++] = wchar_t (x);
          else
            rs[i++] = ((x - 0xD800) << 10) + (*++p - 0xDC00) + 0x10000;
        }

        return r;
      }
      else if (sizeof (wchar_t) == 2)
      {
        // UTF-16
        //
        return String (reinterpret_cast<const wchar_t*> (s), length);
      }
      else
        return String ();
    }

    inline
    String
    transcode (XMLCh const* s)
    {
      return transcode (s,  Xerces::XMLString::stringLen (s));
    }

    inline
    NarrowString
    transcode_to_narrow (XMLCh const* xs)
    {
      char* s (Xerces::XMLString::transcode (xs));
      NarrowString r (s);
      Xerces::XMLString::release (&s);
      return r;
    }

    inline
    XMLCh*
    transcode (String const& str)
    {
      size_t l (str.size ());
      wchar_t const* s (str.c_str ());

      if (sizeof (wchar_t) == 4)
      {
        // Find what the resulting buffer size will be.
        //
        size_t rl (0);

        for (wchar_t const* p (s); p < s + l; ++p)
        {
          rl += (*p & 0xFFFF0000) ? 2 : 1;
        }

        XMLCh* r (new XMLCh[rl + 1]);
        XMLCh* ir (r);

        for (wchar_t const* p (s); p < s + l; ++p)
        {
          wchar_t w (*p);

          if (w & 0xFFFF0000)
          {
            // Surrogate pair.
            //
            *ir++ = static_cast<XMLCh> (((w - 0x10000) >> 10) + 0xD800);
            *ir++ = static_cast<XMLCh> ((w & 0x3FF) + 0xDC00);
          }
          else
            *ir++ = static_cast<XMLCh> (w);
        }

        *ir = XMLCh (0);

        return r;
      }
      else if (sizeof (wchar_t) == 2)
      {
        XMLCh* r (new XMLCh[l + 1]);
        XMLCh* ir (r);

        for (size_t i (0); i < l; ++ir, ++i)
          *ir = static_cast<XMLCh> (s[i]);

        *ir = XMLCh (0);

        return r;
      }
      else
        return 0;
    }

    class XMLChString
    {
    public :
      XMLChString (String const& s)
          : s_ (transcode (s))
      {
      }

      XMLChString (wchar_t const* s)
          : s_ (transcode (String (s)))
      {
      }

      ~XMLChString ()
      {
        delete[] s_;
      }

      XMLCh const*
      c_str () const
      {
        return s_;
      }

    private:
      XMLChString (XMLChString const&);

      XMLChString&
      operator= (XMLChString const&);

    private:
      XMLCh* s_;
    };


    class Element
    {
    public:
      Element (Xerces::DOMElement* e)
          : e_ (e),
            name_ (transcode (e->getLocalName ())),
            namespace__ (transcode (e->getNamespaceURI ()))
      {
      }

      String
      name () const
      {
        return name_;
      }

      String
      namespace_ () const
      {
        return namespace__;
      }

    public:
      unsigned long
      line () const
      {
        //@@ cache
        //
        return reinterpret_cast<unsigned long> (e_->getUserData (line_key));
      }

      unsigned long
      column () const
      {
        //@@ cache
        //
        return reinterpret_cast<unsigned long> (e_->getUserData (column_key));
      }

    public:
      Element
      parent () const
      {
        return dynamic_cast<Xerces::DOMElement*>(e_->getParentNode ());
      }

    public:
      // Attribute identified by a name.
      //
      bool
      attribute_p (String const& name) const
      {
        return attribute_p ("", name);
      }

      String
      attribute (String const& name) const
      {
        return attribute ("", name);
      }

      String
      operator[] (String const& name) const
      {
        return attribute (name);
      }

      // Attribute identified by namespace and name.
      //

      bool
      attribute_p (String const& namespace_, String const& name) const
      {
        Xerces::DOMAttr* a (
          e_->getAttributeNodeNS (
            XMLChString (namespace_).c_str (),
            XMLChString (name).c_str ()));

        return a != 0;
      }

      String
      attribute (String const& namespace_, String const& name) const
      {
        XMLCh const* value (
          e_->getAttributeNS (
            XMLChString (namespace_).c_str (),
            XMLChString (name).c_str ()));

        return transcode (value);
      }

    public:
      Xerces::DOMElement*
      dom_element () const
      {
        return e_;
      }

    private:
      Xerces::DOMElement* e_;

      String name_;
      String namespace__;
    };

    inline String
    prefix (String const& n)
    {
      size_t i (0);
      while (i < n.length () && n[i] != L':') ++i;

      //std::wcerr << "prefix " << n << " "
      //           << String (n, i == n.length () ? i : 0, i) << std::endl;

      return String (n, i == n.length () ? i : 0, i);
    }

    inline String
    uq_name (String const& n)
    {
      size_t i (0);
      while (i < n.length () && n[i] != L':') ++i;

      return String (n.c_str () + (i == n.length () ? 0 : i + 1));
    }

    struct NoMapping
    {
      NoMapping (String const& prefix)
          : prefix_ (prefix)
      {
      }

      String const&
      prefix () const
      {
        return prefix_;
      }

    private:
      String prefix_;
    };

    // Throws NoMapping if there is no prefix-namespace association.
    //
    inline String
    ns_name (Xerces::DOMElement const* e, String const& prefix)
    {
      // 'xml' prefix requires special handling and Xerces folks refuse
      // to handle this in DOM so I have to do it myself.
      //
      if (prefix == L"xml")
        return L"http://www.w3.org/XML/1998/namespace";

      // 0 means "no prefix" to Xerces.
      //
      XMLCh const* xns (
        e->lookupNamespaceURI (
          prefix.empty () ? 0 : XMLChString (prefix).c_str ()));

      if (xns == 0)
        throw NoMapping (prefix);

      return transcode (xns);
    }

    class NoPrefix {};

    inline String
    ns_prefix (Element const& e, String const& wns)
    {
      XMLChString ns (wns);
      XMLCh const* p (e.dom_element ()->lookupPrefix (ns.c_str ()));

      if (p == 0)
      {
        bool r (e.dom_element ()->isDefaultNamespace (ns.c_str ()));

        if (r)
          return L"";
        else
        {
          // 'xml' prefix requires special handling and Xerces folks refuse
          // to handle this in DOM so I have to do it myself.
          //
          if (wns == L"http://www.w3.org/XML/1998/namespace")
            return L"xml";

          throw NoPrefix ();
        }
      }

      return transcode (p);
    }

    inline String
    fq_name (Element const& e, String const& n)
    {
      String un (uq_name (n));

      try
      {
        String ns (ns_name (e.dom_element (), prefix (n)));
        return ns + L'#' + un;
      }
      catch (XML::NoMapping const&)
      {
        return un;
      }
    }


    // Simple auto_ptr version that calls release() instead of delete.
    //

    template <typename X>
    struct AutoPtrRef
    {
      X* x_;

      explicit
      AutoPtrRef (X* x)
          : x_ (x)
      {
      }
    };

    template <typename X>
    struct AutoPtr
    {
      ~AutoPtr ()
      {
        reset ();
      }

      explicit
      AutoPtr (X* x = 0)
          : x_ (x)
      {
      }

      AutoPtr (AutoPtr& y)
          : x_ (y.release ())
      {
      }

      AutoPtr (AutoPtrRef<X> r)
          : x_ (r.x_)
      {
      }

      AutoPtr&
      operator= (AutoPtr& y)
      {
        if (this != &y)
        {
          reset (y.release ());
        }

        return *this;
      }

      AutoPtr&
      operator= (AutoPtrRef<X> r)
      {
        if (r.x_ != x_)
        {
          reset (r.x_);
        }

        return *this;
      }

      operator AutoPtrRef<X> ()
      {
        return AutoPtrRef<X> (release ());
      }

    public:
      X&
      operator* () const
      {
        return *x_;
      }

      X*
      operator-> () const
      {
        return x_;
      }

      X*
      get () const
      {
        return x_;
      }

      X*
      release ()
      {
        X* x (x_);
        x_ = 0;
        return x;
      }

      void
      reset (X* x = 0)
      {
        if (x_)
          x_->release ();

        x_ = x;
      }

      // Conversion to bool.
      //
      typedef X* (AutoPtr::*boolConvertible)() const;

      operator boolConvertible () const throw ()
      {
        return x_ ? &AutoPtr<X>::operator-> : 0;
      }

    private:
      X* x_;
    };

    template <typename X>
    struct PtrVector: std::vector<X*>
    {
      typedef std::vector<X*> Base;

      ~PtrVector ()
      {
        for (typename Base::iterator i (this->begin ()), e (this->end ());
             i != e; ++i)
        {
          if (*i)
            (*i)->release ();
        }
      }

      void
      push_back (AutoPtr<X>& x)
      {
        Base::push_back (0);
        this->back () = x.release ();
      }
    };
  }
}

// Xerces DOoM.
//
//
inline
std::wostream&
operator<< (std::wostream& o, XMLCh const* s)
{
  return o << XSDFrontend::XML::transcode (s);
}

#endif  // XSD_FRONTEND_XML_HXX
