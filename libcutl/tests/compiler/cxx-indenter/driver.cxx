// file      : tests/compiler/cxx-indenter/driver.cxx
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <fstream>
#include <iostream>

#include <cutl/compiler/code-stream.hxx>
#include <cutl/compiler/cxx-indenter.hxx>

using namespace std;
using namespace cutl::compiler;

int
main ()
{
  //std::wostream& os (wcout);
  //ostream_filter<cxx_indenter, wchar_t> filt (os);

  std::ostream& os (cout);
  ostream_filter<cxx_indenter, char> filt (os);

  os << "if (true)"
     << "{"
     << "// Hello there" << endl
     << "//" << endl
     << "a ();"
     << "}"
     << "else"
     << "{"
     << "b ();"
     << "}";

  os << "if (true)" << endl
     << "// Hello there" << endl
     << "//" << endl
     << "a ();"
     << "else" << endl
     << "b ();"
     << endl;

  os << "if (false)"
     << "{"
     << "if (true)"
     << "{"
     << "// test" << endl
     << "}"
     << "else"
     << "{"
     << "// test" << endl
     << "b ();"
     << "}"
     << "}";

  os << "namespace a"
     << "{"
     << "void f ();"
     << "}"
     << "#if defined(__HP_aCC) && __HP_aCC <= 39999" << endl
     << "#include <foo.h>" << endl
     << "#endif" << endl
     << endl
     << "namespace b"
     << "{"
     << "void f ();"
     << "}";

  // Test do-while handling.
  //
  os << "do" << endl
     << "f ();"
     << "while (false);"
     << endl;

  os << "do"
     << "{"
     << "f ();"
     << "}"
     << "while (false);"
     << endl;

  os << "do"
     << "{"
     << "if (f ())"
     << "{"
     << "g ();"
     << "}"
     << "}"
     << "while (false);"
     << endl;

  os << "do"
     << "{"
     << "do" << endl
     << "f ();"
     << "while (false);"
     << "}"
     << "while (false);"
     << endl;

  os << "do"
     << "{"
     << "do"
     << "{"
     << "f ();"
     << "}"
     << "while (false);"
     << "}"
     << "while (false);"
     << endl;

  os << "{"
     << "f (\"CREATE TABLE \\\"test\\\" (\"" << endl
     << "\"'id',\"" << endl
     << "\"'name')\");"
     << "}";

  os << "namespace N"
     << "{"
     << "static int i[] = {{0,\n0},{1,\n1}};"
     << "}";

/*
  @@ TODO: still misindents (if-else association problem)

  os << "{"
     << "if (foo != bar)" << endl
     << "if (foo (bar))" << endl
     << "baz = true;"
     << "else" << endl
     << "baz = false;"
     << "else" << endl
     << "biz = true;"
     << endl
     << "biz = false;"
     << "}";

  os << "{"
     << "if (foo != bar)" << endl
     << "if (foo (bar))"
     << "{"
     << "baz = true;"

     << "if (x)" << endl
     << "test ();"
     << "else" << endl
     << "test ();"
     << endl

     << "if (x)" << endl
     << "if (y)"
     << "{"
     << "test ();"
     << "}"
     << "else"
     << "{"
     << "test ();"
     << "}"

     << "}"
     << "else"
     << "{"
     << "test ();"
     << "}"
     << "biz = false;"
     << "}";
*/
}
