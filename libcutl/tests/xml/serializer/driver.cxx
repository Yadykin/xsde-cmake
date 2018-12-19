// file      : tests/xml/serializer/driver.cxx
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <string>
#include <cassert>
#include <iostream>
#include <sstream>

#include <cutl/xml/serializer.hxx>

using namespace std;
namespace xml = cutl::xml;
using namespace xml;

int
main ()
{
  // Test error handling.
  //
  try
  {
    ostringstream os;
    serializer s (os, "test");

    s.attribute ("foo", "bar");
    assert (false);
  }
  catch (const xml::exception& e)
  {
    // cerr << e.what () << endl;
  }

  try
  {
    ostringstream os;
    os.exceptions (ios_base::badbit | ios_base::failbit);
    serializer s (os, "test");

    s.start_element ("root");
    s.characters ("one");
    os.setstate (ios_base::badbit);
    s.characters ("two");
    assert (false);
  }
  catch (const ios_base::failure& e)
  {
  }

  // Test value serialization.
  //
  {
    ostringstream os;
    serializer s (os, "test", 0);

    s.start_element ("root");
    s.attribute ("version", 123);
    s.characters (true);
    s.end_element ();

    assert (os.str () == "<root version=\"123\">true</root>\n");
  }
}
