// file      : xsde/cxx/serializer/validating/gyear-month.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSDE_CXX_SERIALIZER_VALIDATING_GYEAR_MONTH_HXX
#define XSDE_CXX_SERIALIZER_VALIDATING_GYEAR_MONTH_HXX

#include <xsde/cxx/serializer/validating/xml-schema-sskel.hxx>

namespace xsde
{
  namespace cxx
  {
    namespace serializer
    {
      namespace validating
      {
#ifdef XSDE_REUSE_STYLE_MIXIN
        struct gyear_month_simpl: virtual gyear_month_sskel
#else
        struct gyear_month_simpl: gyear_month_sskel
#endif
        {
          virtual void
          pre (const gyear_month&);

          virtual void
          _serialize_content ();

          gyear_month_simpl (); // Keep it last.

        protected:
          gyear_month value_;
        };
      }
    }
  }
}

#include <xsde/cxx/serializer/validating/gyear-month.ixx>

#endif // XSDE_CXX_SERIALIZER_VALIDATING_GYEAR_MONTH_HXX
