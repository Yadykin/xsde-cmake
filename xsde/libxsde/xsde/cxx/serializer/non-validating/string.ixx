// file      : xsde/cxx/serializer/non-validating/string.ixx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

namespace xsde
{
  namespace cxx
  {
    namespace serializer
    {
      namespace non_validating
      {
        inline string_simpl::
        string_simpl (bool free)
            : free_ (free), value_ (0)
        {
        }
      }
    }
  }
}
