// file      : xsde/cxx/version.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSDE_CXX_VERSION_HXX
#define XSDE_CXX_VERSION_HXX

// Version format is AABBCCDD where
//
// AA - major version number
// BB - minor version number
// CC - bugfix version number
// DD - alpha / beta (DD + 50) version number
//
// When DD is not 00, 1 is subtracted from AABBCC. For example:
//
// Version     AABBCCDD
// 2.0.0       02000000
// 2.1.0       02010000
// 2.1.1       02010100
// 2.2.0.a1    02019901
// 3.0.0.b2    02999952
//

#define XSDE_STR_VERSION "3.3.0.a10"
#define XSDE_INT_VERSION 3029910L

#endif  // XSDE_CXX_VERSION_HXX
