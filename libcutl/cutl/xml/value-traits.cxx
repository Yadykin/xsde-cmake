// file      : cutl/xml/value-traits.cxx
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <cutl/xml/parser.hxx>
#include <cutl/xml/serializer.hxx>

using namespace std;

namespace cutl
{
  namespace xml
  {
    bool default_value_traits<bool>::
    parse (string s, const parser& p)
    {
      if (s == "true" || s == "1" || s == "True" || s == "TRUE")
        return true;
      else if (s == "false" || s == "0" || s == "False" || s == "FALSE")
        return false;
      else
        throw parsing (p, "invalid bool value '" + s + "'");
    }
  }
}
