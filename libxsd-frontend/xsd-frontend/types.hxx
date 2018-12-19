// file      : xsd-frontend/types.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_TYPES_HXX
#define XSD_FRONTEND_TYPES_HXX

#include <string>
#include <cstddef> // std::size_t

namespace XSDFrontend
{
  using std::size_t;

  namespace Bits
  {
    struct None {};

    template <typename C>
    struct NarrowerChar
    {
      typedef None Type;
    };

    template <>
    struct NarrowerChar<wchar_t>
    {
      typedef char Type;
    };
  }

  struct NonRepresentable: std::exception
  {
    virtual char const*
    what () const throw ();
  };

  template <typename C, typename NC = typename Bits::NarrowerChar<C>::Type>
  class StringTemplate;

  template <>
  class StringTemplate<Bits::None, Bits::None>
  {
  };

  template <typename C, typename NC>
  class StringTemplate : public std::basic_string<C>
  {
    typedef std::basic_string<C> Base;
    typedef std::basic_string<NC> NarrowerBase;

    Base&
    base ()
    {
      return *this;
    }

    Base const&
    base () const
    {
      return *this;
    }

  public:
    typedef typename Base::size_type size_type;

    using Base::npos;

  public:
    StringTemplate ()
    {
    }

    StringTemplate (StringTemplate const& str,
                    size_type pos,
                    size_type n = npos)
        : Base (str, pos, n)
    {
    }

    StringTemplate (C const* s, size_type n)
        : Base (s, n)
    {
    }

    StringTemplate (C const* s)
        : Base (s)
    {
    }

    StringTemplate (size_type n, C c)
        : Base (n, c)
    {
    }

    template <typename I>
    StringTemplate(I begin, I end)
        : Base (begin, end)
    {
    }

    StringTemplate (StringTemplate const& other)
        : Base (other)
    {
    }

    // Conversion from Base.
    //
    StringTemplate (Base const& str)
        : Base (str)
    {
    }

    // Conversion from the Narrower type. Experimental.
    //
    StringTemplate (NC const* s)
    {
      from_narrow (s);
    }

    StringTemplate (StringTemplate<NC> const& other)
    {
      from_narrow (other.c_str ());
    }

    StringTemplate (NarrowerBase const& other)
    {
      from_narrow (other.c_str ());
    }

    // Assignment.
    //
    StringTemplate&
    operator= (StringTemplate const& str)
    {
      base () = str;
      return *this;
    }

    StringTemplate&
    operator= (C const* s)
    {
      base () = s;
      return *this;
    }

    StringTemplate&
    operator= (C c)
    {
      base () = c;
      return *this;
    }

    // Assignment from Base.
    //
    StringTemplate&
    operator= (Base const& str)
    {
      base () = str;
      return *this;
    }

  public:
    StringTemplate&
    operator+= (StringTemplate const& str)
    {
      base () += str;
      return *this;
    }

    StringTemplate&
    operator+= (C const* s)
    {
      base () += s;
      return *this;
    }

    StringTemplate&
    operator+= (C c)
    {
      base () += c;
      return *this;
    }

    // Conversion to the Narrower type.
    //
  public:
    StringTemplate<NC>
    to_narrow () const;

    // Conversion to bool.
    //
  private:
    typedef void (StringTemplate::*BooleanConvertible)();
    void true_ () {}

  public:
    operator BooleanConvertible () const
    {
      return this->empty () ? 0 : &StringTemplate::true_;
    }

  private:
    void
    from_narrow (NC const* s);
  };


  template<typename C>
  StringTemplate<C>
  operator+ (StringTemplate<C> const& lhs, StringTemplate<C> const& rhs)
  {
    return StringTemplate<C> (lhs) += rhs;
  }

  template<typename C>
  StringTemplate<C>
  operator+ (C const* lhs, StringTemplate<C> const& rhs)
  {
    return StringTemplate<C> (lhs) += rhs;
  }

  template<typename C>
  StringTemplate<C>
  operator+ (StringTemplate<C> const& lhs, C const* rhs)
  {
    return StringTemplate<C> (lhs) += rhs;
  }

  template<typename C>
  StringTemplate<C>
  operator+ (C lhs, StringTemplate<C> const& rhs)
  {
    return StringTemplate<C> (1, lhs) += rhs;
  }

  template<typename C>
  StringTemplate<C>
  operator+ (StringTemplate<C> const& lhs, C rhs)
  {
    return StringTemplate<C> (lhs) += rhs;
  }

  typedef StringTemplate<char> NarrowString;
  typedef StringTemplate<wchar_t> WideString;

  typedef WideString String;
}

#endif  // XSD_FRONTEND_TYPES_HXX
