// file      : cutl/meta/remove-p.hxx
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#ifndef CUTL_META_REMOVE_P_HXX
#define CUTL_META_REMOVE_P_HXX

namespace cutl
{
  namespace meta
  {
    template <typename X>
    struct remove_p
    {
      typedef X r;
    };

    template <typename X>
    struct remove_p<X*>
    {
      typedef X r;
    };
  }
}

#endif // CUTL_META_REMOVE_P_HXX
