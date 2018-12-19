// file      : xsde/cxx/serializer/validating/idrefs.ixx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

namespace xsde
{
  namespace cxx
  {
    namespace serializer
    {
      namespace validating
      {
        inline idrefs_simpl::
        idrefs_simpl (bool free)
            : free_ (free), value_ (0), serializer_ (false)
        {
        }
      }
    }
  }
}
