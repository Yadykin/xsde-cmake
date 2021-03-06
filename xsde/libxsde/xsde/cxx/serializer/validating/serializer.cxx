// file      : xsde/cxx/serializer/validating/serializer.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsde/cxx/serializer/validating/serializer.hxx>

namespace xsde
{
  namespace cxx
  {
    namespace serializer
    {
      namespace validating
      {
        // simple_content
        //
#ifdef XSDE_REUSE_STYLE_TIEIN
        void simple_content::
        _serialize_content ()
        {
          if (impl_)
            impl_->_serialize_content ();
        }
#endif
      }
    }
  }
}
