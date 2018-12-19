// file      : tests/container/multi-index/driver.cxx
// copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <map>
#include <list>
#include <string>
#include <cassert>
#include <iostream>

#include <cutl/container/multi-index.hxx>

using namespace std;
using namespace cutl::container;

struct person
{
  person (const string& e, const string& f, const string& l, unsigned short a)
      : email (e), first (f), last (l), age (a) {}

  const string email;
  const string first;
  const string last;
  unsigned short age;
};

struct person_email_set
{
  typedef map<key<string>, person> email_map;
  typedef map_const_iterator<email_map> const_iterator;

  pair<const_iterator, bool>
  insert (const person& v)
  {
    pair<email_map::iterator, bool> r (
      email_map_.insert (email_map::value_type (v.email, v)));

    const_iterator i (r.first);

    if (r.second)
      r.first->first.assign (i->email);

    return make_pair (i, r.second);
  }

  const_iterator
  find (const string& email) const
  {
    return email_map_.find (email);
  }

  const_iterator begin () const {return email_map_.begin ();}
  const_iterator end () const {return email_map_.end ();}

private:
  email_map email_map_;
};

struct person_name_set
{
  typedef key<string, string> name_key;
  typedef map<name_key, person> name_map;
  typedef map_const_iterator<name_map> const_iterator;

  pair<const_iterator, bool>
  insert (const person& v)
  {
    pair<name_map::iterator, bool> r (
      name_map_.insert (
        name_map::value_type (name_key (v.first, v.last), v)));

    const_iterator i (r.first);

    if (r.second)
      r.first->first.assign (i->first, i->last);

    return make_pair (i, r.second);
  }

  const_iterator
  find (const string& first, const string& last) const
  {
    return name_map_.find (name_key (first, last));
  }

  const_iterator begin () const {return name_map_.begin ();}
  const_iterator end () const {return name_map_.end ();}

private:
  name_map name_map_;
};

struct person_email_name_set
{
  typedef key<string, string> name_key;
  typedef map<name_key, person> name_map;
  typedef map_iterator<name_map> iterator;
  typedef map_const_iterator<name_map> const_iterator;

  typedef map<key<string>, iterator> email_map;

  pair<iterator, bool>
  insert (const person& v)
  {
    // First check that we don't have any collisions in the secondary
    // indexes.
    //
    {
      email_map::iterator i (email_map_.find (v.email));

      if (i != email_map_.end ())
        return make_pair (i->second, false);
    }

    pair<name_map::iterator, bool> r (
      name_map_.insert (
        name_map::value_type (name_key (v.first, v.last), v)));

    iterator i (r.first);

    if (r.second)
    {
      r.first->first.assign (i->first, i->last);
      email_map_.insert (email_map::value_type (i->email, i));
    }

    return make_pair (i, r.second);
  }

  iterator
  find (const string& first, const string& last)
  {
    return name_map_.find (name_key (first, last));
  }

  const_iterator
  find (const string& first, const string& last) const
  {
    return name_map_.find (name_key (first, last));
  }

  iterator
  find (const string& email)
  {
    email_map::iterator i (email_map_.find (email));
    return i != email_map_.end () ? i->second : end ();
  }

  const_iterator
  find (const string& email) const
  {
    email_map::const_iterator i (email_map_.find (email));
    return i != email_map_.end () ? i->second : end ();
  }

  void
  erase (iterator i )
  {
    email_map_.erase (i->email);
    name_map_.erase (i);
  }

  iterator begin () {return name_map_.begin ();}
  const_iterator begin () const {return name_map_.begin ();}

  iterator end () {return name_map_.end ();}
  const_iterator end () const {return name_map_.end ();}

private:
  name_map name_map_;
  email_map email_map_;
};

struct person_list_email_set
{
  typedef list<person> person_list;
  typedef person_list::iterator iterator;
  typedef person_list::const_iterator const_iterator;

  typedef map<key<string>, iterator> email_map;

  pair<iterator, bool>
  insert (const person& v)
  {
    // First check that we don't have any collisions in the secondary
    // indexes.
    //
    {
      email_map::iterator i (email_map_.find (v.email));

      if (i != email_map_.end ())
        return make_pair (i->second, false);
    }

    iterator i (person_list_.insert (end (), v));
    email_map_.insert (email_map::value_type (i->email, i));
    return make_pair (i, true);
  }

  iterator
  find (const string& email)
  {
    email_map::iterator i (email_map_.find (email));
    return i != email_map_.end () ? i->second : end ();
  }

  const_iterator
  find (const string& email) const
  {
    email_map::const_iterator i (email_map_.find (email));
    return i != email_map_.end () ? i->second : end ();
  }

  iterator begin () {return person_list_.begin ();}
  const_iterator begin () const {return person_list_.begin ();}

  iterator end () {return person_list_.end ();}
  const_iterator end () const {return person_list_.end ();}

private:
  person_list person_list_;
  email_map email_map_;
};

int
main ()
{
  {
    person_email_set s;

    assert (s.insert (person ("john@doe.com", "John", "Doe", 20)).second);
    assert (s.insert (person ("jane@doe.com", "Jane", "Doe", 21)).second);
    assert (!s.insert (person ("john@doe.com", "Johnny", "Doe", 22)).second);

    assert (s.find ("john@doe.com") != s.end ());
    assert (s.find ("jane@doe.com") != s.end ());
    assert (s.find ("john@doe.org") == s.end ());
  }

  {
    person_name_set s;

    assert (s.insert (person ("john@doe.com", "John", "Doe", 20)).second);
    assert (s.insert (person ("jane@doe.com", "Jane", "Doe", 21)).second);
    assert (!s.insert (person ("john@doe.org", "John", "Doe", 22)).second);

    assert (s.find ("John", "Doe") != s.end ());
    assert (s.find ("Jane", "Doe") != s.end ());
    assert (s.find ("Johnny", "Doe") == s.end ());
  }

  {
    person_email_name_set s;
    person_email_name_set const& cs (s);

    assert (s.insert (person ("john@doe.com", "John", "Doe", 20)).second);
    assert (s.insert (person ("jane@doe.com", "Jane", "Doe", 21)).second);
    assert (!s.insert (person ("john@doe.org", "John", "Doe", 22)).second);
    assert (!s.insert (person ("john@doe.com", "Johnny", "Doe", 23)).second);

    assert (s.find ("John", "Doe") != s.end ());
    assert (cs.find ("Jane", "Doe") != cs.end ());
    assert (s.find ("john@doe.com") != s.end ());
    assert (cs.find ("jane@doe.com") != s.end ());
    assert (s.find ("Johnny", "Doe") == s.end ());
    assert (cs.find ("john@doe.org") == s.end ());

    person_email_name_set::iterator i (s.find ("John", "Doe"));
    i->age++;

    s.erase (i);
    assert (s.find ("John", "Doe") == s.end ());
    assert (s.find ("john@doe.com") == s.end ());
  }

  {
    person_list_email_set s;

    assert (s.insert (person ("john@doe.com", "John", "Doe", 20)).second);
    assert (s.insert (person ("jane@doe.com", "Jane", "Doe", 21)).second);
    assert (!s.insert (person ("john@doe.com", "Johnny", "Doe", 22)).second);

    assert (s.find ("john@doe.com") != s.end ());
    assert (s.find ("jane@doe.com") != s.end ());
    assert (s.find ("jane@doe.org") == s.end ());

    person_list_email_set::iterator i (s.begin ());
    assert (i != s.end () && i->email == "john@doe.com");
    assert (++i != s.end () && i->email == "jane@doe.com");
    assert (++i == s.end ());
  }
}
