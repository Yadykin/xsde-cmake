// file      : tests/re/driver.cxx
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <string>
#include <cassert>
#include <iostream>

#include <cutl/re.hxx>

using namespace cutl::re;

int
main ()
{
  // empty() and str().
  //
  {
    regex r;
    assert (r.empty ());
    r = "['`]foo([^ ]*)bar['`]";
    assert (!r.empty ());
    assert (r.str () == "['`]foo([^ ]*)bar['`]");
  }

  // Error handling.
  //
  try
  {
    regex r ("['`]foo([^ ]*bar['`]");
    assert (false);
  }
  catch (format const& e)
  {
    assert (e.regex () == "['`]foo([^ ]*bar['`]");
    assert (!e.description ().empty ());
    //std::cerr << e.description () << std::endl;
  }

  // match(), search(), and replace().
  //
  {
    regex r ("['`]foo([^ ]*)bar['`]");

    assert (r.match ("'foofoxbar'"));
    assert (!r.match ("'foof xbar'"));

    assert (r.search ("prefix 'foofoxbar' suffix"));
    assert (!r.search ("prefix 'foof xbar' suffix"));

    assert (r.replace ("'foofoxbar'", "\\u$1") == "Fox");
  }

  // replace() using escape sequences.
  //
  {
    regex r ("([aA][bB][cC])");

    // $-based escape sequences.
    //
    assert (r.replace ("xabcyz", "v$") == "xv$yz");
    assert (r.replace ("xabcyz", "v$d") == "xv$dyz");
    assert (r.replace ("xabcyz", "v$1d") == "xvabcdyz");
    assert (r.replace ("xabcyabcz", "v$2d") == "xvdyvdz");
    assert (r.replace ("xabcyz", "v$&d") == "xvabcdyz");
    assert (r.replace ("xabcyz", "$`$$$\'") == "xx$yzyz");

    {
      regex r ("(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)");

      assert (r.replace ("xabcdefghijy", "$10$9$8$7$6$5$4$3$2$1") ==
              "xjihgfedcbay");
    }

    // \-based escape sequences.
    //
    assert (r.replace ("xabcyz", "v\\d") == "xvdyz");
    assert (r.replace ("xabcyz", "v\\1d") == "xvabcdyz");
    assert (r.replace ("xabcyabcz", "v\\2d") == "xvdyvdz");
    assert (r.replace ("xabcyz", "v\\\\d") == "xv\\dyz");
    assert (r.replace ("xabcyz", "v\\$d") == "xv$dyz");

    {
      regex r ("(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)");

      assert (r.replace ("xabcdefghijy", "\\10\\9\\8\\7\\6\\5\\4\\3\\2\\1") ==
              "xa0ihgfedcbay");
    }

    assert (r.replace ("xabcyz", "\\u") == "xyz");
    assert (r.replace ("xabcyz", "\\uv") == "xVyz");
    assert (r.replace ("xabcyz", "\\u\\1") == "xAbcyz");
    assert (r.replace ("xabcyz", "\\lV") == "xvyz");
    assert (r.replace ("xAbcyz", "\\l\\1") == "xabcyz");

    assert (r.replace ("xabcyz", "\\U") == "xyz");
    assert (r.replace ("xabcyz", "\\Uv") == "xVyz");
    assert (r.replace ("xabcyz", "\\U\\1v") == "xABCVyz");
    assert (r.replace ("xabcyz", "\\U\\1\\Ev") == "xABCvyz");

    assert (r.replace ("xabcyz", "\\L") == "xyz");
    assert (r.replace ("xabcyz", "\\LV") == "xvyz");
    assert (r.replace ("xABCyz", "\\L\\1V") == "xabcvyz");
    assert (r.replace ("xabcyz", "\\L\\1\\EV") == "xabcVyz");

    assert (r.replace ("xabcyz", "\\Uv\\LV") == "xVvyz");
    assert (r.replace ("xabcyz", "\\U\\1\\LV") == "xABCvyz");

    {
      regex r ("(b?)-");
      assert (r.replace ("a-b-", "\\u\\1x") == "aXBx");
    }
  }

  // wregex::replace().
  //
  {
    {
      wregex r (L"['`]foo([^ ]*)bar['`]");
      assert (r.replace (L"'foofoxbar'", L"\\u$1") == L"Fox");
    }
/*
    {
      std::locale::global (std::locale ("en_US.utf8"));

      wregex r (L"(a)");
      assert (r.replace (L"a", L"\\l\u0190") == L"\u025b");
    }
*/
  }

  // regexsub
  //
  {
    regexsub r ("/['`]foo([^ ]*)bar['`]/\\u$1/");

    assert (r.replace ("'foofoxbar'") == "Fox");
  }

  // regexsub escaping
  //
  {
    regexsub r ("#a\\#\\\\#a?\\\\#");

    assert (r.replace ("a#\\") == "a?\\");
  }

  // regexsub error handling.
  //
  try
  {
    regexsub r ("/['`]foo([^ ]*)bar['`]#$1/");
    assert (false);
  }
  catch (format const& e)
  {
    assert (e.regex () == "/['`]foo([^ ]*)bar['`]#$1/");
    assert (!e.description ().empty ());
    //std::cerr << e.description () << std::endl;
  }
}
