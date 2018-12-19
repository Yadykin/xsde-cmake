// file      : xsd-frontend/version.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_VERSION_HXX
#define XSD_FRONTEND_VERSION_HXX

#include <xercesc/util/XercesVersion.hpp>

#if _XERCES_VERSION < 30000
#  error Xerces-C++ 2-series is not supported
#endif

#endif  // XSD_FRONTEND_VERSION_HXX
