// file      : cutl/version.hxx.in -*- C++ -*-
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#ifndef LIBCUTL_VERSION // Note: using the version macro itself.

// Note: using build2 standard versioning scheme. The numeric version format
// is AAABBBCCCDDDE where:
//
// AAA - major version number
// BBB - minor version number
// CCC - bugfix version number
// DDD - alpha / beta (DDD + 500) version number
// E   - final (0) / snapshot (1)
//
// When DDDE is not 0, 1 is subtracted from AAABBBCCC. For example:
//
// Version      AAABBBCCCDDDE
//
// 0.1.0        0000010000000
// 0.1.2        0000010010000
// 1.2.3        0010020030000
// 2.2.0-a.1    0020019990010
// 3.0.0-b.2    0029999995020
// 2.2.0-a.1.z  0020019990011
//
#define LIBCUTL_VERSION       10109995031ULL
//#define LIBCUTL_VERSION_STR   "1.11.0-b.3.20181203094219"
//#define LIBCUTL_VERSION_ID    "1.11.0-b.3.20181203094219"
#define LIBCUTL_VERSION_STR   "1.11.0"
#define LIBCUTL_VERSION_ID    "1.11.0"

#define LIBCUTL_VERSION_MAJOR 1
#define LIBCUTL_VERSION_MINOR 11
#define LIBCUTL_VERSION_PATCH 0

#define LIBCUTL_PRE_RELEASE   true

#define LIBCUTL_SNAPSHOT      20181203094219ULL
#define LIBCUTL_SNAPSHOT_ID   ""

#endif // LIBCUTL_VERSION
