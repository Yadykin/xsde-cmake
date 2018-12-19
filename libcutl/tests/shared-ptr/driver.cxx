// file      : tests/shared-ptr/driver.cxx
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <string>
#include <cassert>

#include <cutl/shared-ptr.hxx>

using namespace cutl;

struct type
{
  type (int x, char const* y) : x_ (x), y_ (y) {}

  int x_;
  std::string y_;
};

struct base1
{
  virtual
  ~base1 () {}
  base1 (int x) : x_ (x) {}

  int x_;
};

struct base2
{
  virtual
  ~base2 () {}
  base2 (char const* y) : y_ (y) {}

  std::string y_;
};

struct derived: base1, base2
{
  derived (int x, char const* y) : base1 (x), base2 (y) {}
};

struct shared_type: shared_base
{
  shared_type (int x, char const* y)
      : x_ (x), y_ (y)
  {
    assert (ref_count (this) == 1);
  }

  int x_;
  std::string y_;
};

int
main ()
{
  //
  // inc_ref, dec_ref, ref_count
  //

  // Non-polymorphic type.
  //
  {
    type* x (new (shared) type (5, "foo"));
    assert (ref_count (x) == 1);
    inc_ref (x);
    assert (ref_count (x) == 2);
    dec_ref (x);
    assert (ref_count (x) == 1);
    dec_ref (x);
  }

  // Polymorphic type.
  //
  {
    base2* x (new (shared) derived (5, "foo"));
    assert (ref_count (x) == 1);
    inc_ref (x);
    assert (ref_count (x) == 2);
    dec_ref (x);
    assert (ref_count (x) == 1);
    dec_ref (x);
  }

  // Shared type.
  //
  {
    shared_type* x (new (shared) shared_type (5, "foo"));
    assert (ref_count (x) == 1);
    inc_ref (x);
    assert (ref_count (x) == 2);
    dec_ref (x);
    assert (ref_count (x) == 1);
    dec_ref (x);
  }

  // Error handling. This can theoretically can segfault and it trips up
  // the address sanitizer.
  //
#ifndef __SANITIZE_ADDRESS__
  {
    type* x (new type (5, "foo"));

    try
    {
      inc_ref (x);
      assert (false);
    }
    catch (not_shared const&)
    {
    }

    delete x;
  }
#endif

  //
  // shared_ptr
  //

  // Non-polymorphic type.
  //
  {
    shared_ptr<type> x (new (shared) type (5, "foo"));
    assert (x.count () == 1);
    assert (x);
    assert (x->x_ == 5);
    assert ((*x).y_ == "foo");
    {
      shared_ptr<type> y (x);
      assert (y.count () == 2);
    }
    {
      shared_ptr<type> y;
      y = x;
      assert (y.count () == 2);
    }
    assert (x.count () == 1);
    shared_ptr<type> y (x.release ());
    assert (y.count () == 1);
  }

  // Polymorphic type.
  //
  {
    shared_ptr<derived> x (new (shared) derived (5, "foo"));
    assert (x.count () == 1);
    {
      shared_ptr<base2> y (x);
      assert (y.count () == 2);
      assert (y->y_ == "foo");
    }
    {
      shared_ptr<base2> y;
      y = x;
      assert (y.count () == 2);
    }
    assert (x.count () == 1);
  }

  // Non-polymorphic type.
  //
  {
    shared_ptr<shared_type> x (new (shared) shared_type (5, "foo"));
    assert (x.count () == 1);
    assert (x);
    assert (x->x_ == 5);
    assert ((*x).y_ == "foo");
    assert (x->_ref_count () == 1);
    x->_inc_ref ();
    assert (x.count () == 2);
    x->_dec_ref ();
    assert (x.count () == 1);
    {
      shared_ptr<shared_type> y (x);
      assert (y.count () == 2);
    }
    {
      shared_ptr<shared_type> y;
      y = x;
      assert (y.count () == 2);
    }
    assert (x.count () == 1);
    shared_ptr<shared_type> y (x.release ());
    assert (y.count () == 1);
  }
}
