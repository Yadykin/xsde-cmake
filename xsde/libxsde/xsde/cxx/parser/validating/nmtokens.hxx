// file      : xsde/cxx/parser/validating/nmtokens.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSDE_CXX_PARSER_VALIDATING_NMTOKENS_HXX
#define XSDE_CXX_PARSER_VALIDATING_NMTOKENS_HXX

#include <xsde/cxx/parser/validating/xml-schema-pskel.hxx>
#include <xsde/cxx/parser/validating/nmtoken.hxx>

namespace xsde
{
  namespace cxx
  {
    namespace parser
    {
      namespace validating
      {
#ifdef XSDE_REUSE_STYLE_MIXIN
        struct nmtokens_pimpl: virtual nmtokens_pskel
#else
        struct nmtokens_pimpl: nmtokens_pskel
#endif
        {
          ~nmtokens_pimpl ();
          nmtokens_pimpl (bool base = false);

          void
          pre_impl (string_sequence*);

          virtual void
          _pre ();

          virtual void
          _xsde_parse_item (const ro_string&);

          virtual void
          _post ();

          virtual string_sequence*
          post_nmtokens ();

          virtual void
          _reset ();

        protected:
          bool base_;
          nmtoken_pimpl parser_;
          string_sequence* seq_;
        };
      }
    }
  }
}

#endif  // XSDE_CXX_PARSER_VALIDATING_NMTOKENS_HXX