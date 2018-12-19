// file      : tests/compiler/traversal/driver.cxx
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <vector>
#include <iostream>

#include <cutl/shared-ptr.hxx>

#include <cutl/compiler/type-info.hxx>
#include <cutl/compiler/traversal.hxx>

using namespace std;
using namespace cutl;

// Data types.
//
struct base
{
  virtual ~base () {}
};

struct derived1: base {};
struct derived2: base {};

typedef vector<cutl::shared_ptr<base> > objects;

struct init
{
  init ()
  {
    using compiler::type_info;

    {
      type_info ti (typeid (base));
      insert (ti);
    }

    {
      type_info ti (typeid (derived1));
      ti.add_base (typeid (base));
      insert (ti);
    }

    {
      type_info ti (typeid (derived2));
      ti.add_base (typeid (base));
      insert (ti);
    }
  }
} init_;

// Traversers.
//
template <typename X>
struct traverser: compiler::traverser_impl<X, base>,
                  virtual compiler::dispatcher<base>
{
  void
  add_traverser (compiler::traverser_map<base>& m)
  {
    compiler::dispatcher<base>::traverser (m);
  }
};

typedef traverser<base> base_trav;
typedef traverser<derived1> derived1_trav;
typedef traverser<derived2> derived2_trav;

struct base_impl: base_trav
{
  virtual void
  traverse (type&)
  {
    cout << "base_impl: base" << endl;
  }
};

struct derived1_impl: derived1_trav
{
  virtual void
  traverse (type&)
  {
    cout << "derived1_impl: derived1" << endl;
  }
};

struct combined_impl: derived1_trav, derived2_trav
{
  virtual void
  traverse (derived1&)
  {
    cout << "combined_impl: derived1" << endl;
  }

  virtual void
  traverse (derived2&)
  {
    cout << "combined_impl: derived2" << endl;
  }
};

int
main ()
{
  using cutl::shared_ptr;

  objects o;
  o.push_back (shared_ptr<base> (new (shared) base));
  o.push_back (shared_ptr<base> (new (shared) derived1));
  o.push_back (shared_ptr<base> (new (shared) derived2));

  base_impl base;
  derived1_impl derived1;
  combined_impl combined;

  for (objects::iterator i (o.begin ()); i != o.end (); ++i)
    base.dispatch (**i);

  cout << endl;

  for (objects::iterator i (o.begin ()); i != o.end (); ++i)
    derived1.dispatch (**i);

  cout << endl;

  for (objects::iterator i (o.begin ()); i != o.end (); ++i)
    combined.dispatch (**i);

  cout << endl;

  base.add_traverser (derived1);
  for (objects::iterator i (o.begin ()); i != o.end (); ++i)
    base.dispatch (**i);

  cout << endl;

  derived1.add_traverser (combined);
  for (objects::iterator i (o.begin ()); i != o.end (); ++i)
    derived1.dispatch (**i);
}
