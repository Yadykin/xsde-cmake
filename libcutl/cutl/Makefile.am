# file      : cutl/Makefile.am
# copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
# license   : MIT; see accompanying LICENSE file

EXTRA_DIST = __file__(extra_dist)

cutlincludedir = $(includedir)/cutl

lib_LTLIBRARIES = libcutl.la
libcutl_la_SOURCES = __path__(sources)

nobase_cutlinclude_HEADERS = __path__(headers)
nobase_nodist_cutlinclude_HEADERS = details/config.h

if !LIBCUTL_EXTERNAL_BOOST
libcutl_la_SOURCES += __path__(boost_sources)
endif

if !LIBCUTL_EXTERNAL_EXPAT
libcutl_la_SOURCES += __path__(expat_sources)
nobase_cutlinclude_HEADERS += __path__(expat_headers)
endif

libcutl_la_SOURCES += __path__(genx_sources)
nobase_cutlinclude_HEADERS += __path__(genx_headers)

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir) -DLIBCUTL_DYNAMIC_LIB
AM_LDFLAGS = -release __value__(interface_version) -no-undefined
