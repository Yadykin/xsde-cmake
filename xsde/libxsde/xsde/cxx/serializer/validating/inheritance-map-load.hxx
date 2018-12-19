// file      : xsde/cxx/serializer/validating/inheritance-map-load.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSDE_CXX_SERIALIZER_VALIDATING_INHERITANCE_MAP_LOAD_HXX
#define XSDE_CXX_SERIALIZER_VALIDATING_INHERITANCE_MAP_LOAD_HXX

#include <stddef.h> // size_t

#include <xsde/cxx/config.hxx>

namespace xsde
{
  namespace cxx
  {
    namespace serializer
    {
      namespace validating
      {
        size_t
        serializer_imap_elements ();

        inline size_t
        serializer_imap_buckets ()
        {
          return XSDE_SERIALIZER_IMAP_BUCKETS;
        }
      }
    }
  }
}

#endif // XSDE_CXX_SERIALIZER_VALIDATING_INHERITANCE_MAP_LOAD_HXX

