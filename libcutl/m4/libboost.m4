dnl file      : m4/libboost.m4
dnl copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
dnl license   : MIT; see accompanying LICENSE file
dnl
dnl LIBBOOST([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
dnl
AC_DEFUN([LIBBOOST], [
libboost_found=no

AC_MSG_CHECKING([for boost base headers])

AC_ARG_WITH(
  [boost],
  [AC_HELP_STRING([--with-boost=DIR],[location of boost build directory])],
  [libboost_dir=${withval}],
  [libboost_dir=])

# If libboost_dir was given, add the necessary preprocessor and linker flags.
#
if test x"$libboost_dir" != x; then
  save_CPPFLAGS="$CPPFLAGS"
  save_LDFLAGS="$LDFLAGS"

  AS_SET_CATFILE([abs_libboost_dir], [$ac_pwd], [$libboost_dir])

  CPPFLAGS="$CPPFLAGS -I$abs_libboost_dir"
  LDFLAGS="$LDFLAGS -L$abs_libboost_dir/stage/lib"
fi

CXX_LIBTOOL_LINK_IFELSE([
AC_LANG_SOURCE([
#include <boost/version.hpp>

#ifndef BOOST_VERSION
#  error BOOST_VERSION not defined
#endif

int
main ()
{
}
])],
[
libboost_found=yes
])

if test x"$libboost_found" = xno; then
  if test x"$libboost_dir" != x; then
    CPPFLAGS="$save_CPPFLAGS"
    LDFLAGS="$save_LDFLAGS"
  fi
fi

if test x"$libboost_found" = xyes; then
  AC_MSG_RESULT([yes])
  $1
else
  AC_MSG_RESULT([no])
  $2
fi
])dnl
dnl
dnl LIBBOOST_HEADER_LIB(NAME, SOURCE, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)
dnl
AC_DEFUN([LIBBOOST_HEADER_LIB], [
libboost_$1_found=no

AC_MSG_CHECKING([for boost $1 library])
CXX_LIBTOOL_LINK_IFELSE([$2],[libboost_$1_found=yes])

if test x"$libboost_$1_found" = xyes; then
  AC_MSG_RESULT([yes])
  [$3]
else
  AC_MSG_RESULT([no])
  [$4]
fi
])dnl
dnl
dnl LIBBOOST_LIB(NAME, SOURCE, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)
dnl
AC_DEFUN([LIBBOOST_LIB], [
libboost_$1_found=no

AC_MSG_CHECKING([for boost $1 library])

save_LIBS="$LIBS"
LIBS="-lboost_$1 $LIBS"

CXX_LIBTOOL_LINK_IFELSE([$2],[libboost_$1_found=yes])

# Try to fall back on the -mt version for backwards-compatibility.
#
if test x"$libboost_$1_found" = xno; then
   LIBS="-lboost_$1-mt $save_LIBS"
   CXX_LIBTOOL_LINK_IFELSE([$2],[libboost_$1_found=yes])
fi

if test x"$libboost_$1_found" = xno; then
  LIBS="$save_LIBS"
fi

if test x"$libboost_$1_found" = xyes; then
  AC_MSG_RESULT([yes])
  [$3]
else
  AC_MSG_RESULT([no])
  [$4]
fi
])dnl
dnl
dnl LIBBOOST_SYSTEM([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
AC_DEFUN([LIBBOOST_SYSTEM], [
LIBBOOST_LIB([system],[
AC_LANG_SOURCE([
int
main ()
{
}
])],
[$1],
[$2])
])dnl
dnl
dnl LIBBOOST_REGEX([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl
AC_DEFUN([LIBBOOST_REGEX], [
LIBBOOST_LIB([regex],[
AC_LANG_SOURCE([
#include <boost/tr1/regex.hpp>

int
main ()
{
  std::tr1::regex r ("te.t", std::tr1::regex_constants::ECMAScript);
  return std::tr1::regex_match ("test", r) ? 0 : 1;
}
])],
[$1],
[$2])
])dnl
