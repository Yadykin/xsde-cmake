
// (C) Copyright Dave Abrahams, Steve Cleary, Beman Dawes, 
// Aleksey Gurtovoy, Howard Hinnant & John Maddock 2000.  
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_PP_IS_ITERATING)

///// header body

#ifndef BOOST_DETAIL_IS_FUNCTION_REF_TESTER_HPP_INCLUDED
#define BOOST_DETAIL_IS_FUNCTION_REF_TESTER_HPP_INCLUDED

#include "cutl/details/boost/type_traits/detail/yes_no_type.hpp"
#include "cutl/details/boost/type_traits/config.hpp"

#if defined(BOOST_TT_PREPROCESSING_MODE)
#   include "cutl/details/boost/preprocessor/iterate.hpp"
#   include "cutl/details/boost/preprocessor/enum_params.hpp"
#   include "cutl/details/boost/preprocessor/comma_if.hpp"
#endif

namespace cutl_details_boost {
namespace detail {
namespace is_function_ref_tester_ {

template <class T>
cutl_details_boost::type_traits::no_type BOOST_TT_DECL is_function_ref_tester(T& ...);

#if !defined(BOOST_TT_PREPROCESSING_MODE)
// preprocessor-generated part, don't edit by hand!

template <class R>
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(), int);

template <class R,class T0 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0), int);

template <class R,class T0,class T1 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1), int);

template <class R,class T0,class T1,class T2 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2), int);

template <class R,class T0,class T1,class T2,class T3 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3), int);

template <class R,class T0,class T1,class T2,class T3,class T4 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19,class T20 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19,class T20,class T21 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20,T21), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19,class T20,class T21,class T22 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20,T21,T22), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19,class T20,class T21,class T22,class T23 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20,T21,T22,T23), int);

template <class R,class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19,class T20,class T21,class T22,class T23,class T24 >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(T0,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20,T21,T22,T23,T24), int);

#else

#define BOOST_PP_ITERATION_PARAMS_1 \
    (3, (0, 25, "cutl/details/boost/detail/is_function_ref_tester.hpp"))
#include BOOST_PP_ITERATE()

#endif // BOOST_TT_PREPROCESSING_MODE

} // namespace detail
} // namespace python
} // namespace cutl_details_boost

#endif // BOOST_DETAIL_IS_FUNCTION_REF_TESTER_HPP_INCLUDED

///// iteration

#else
#define i BOOST_PP_FRAME_ITERATION(1)

template <class R BOOST_PP_COMMA_IF(i) BOOST_PP_ENUM_PARAMS(i,class T) >
cutl_details_boost::type_traits::yes_type is_function_ref_tester(R (&)(BOOST_PP_ENUM_PARAMS(i,T)), int);

#undef i
#endif // BOOST_PP_IS_ITERATING

